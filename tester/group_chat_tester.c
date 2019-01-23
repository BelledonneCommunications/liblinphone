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

static const char sFactoryUri[] = "sip:conference-factory@conf.example.org";

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

static void chat_room_state_changed (LinphoneChatRoom *cr, LinphoneChatRoomState newState) {
	LinphoneCore *core = linphone_chat_room_get_core(cr);
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);
	ms_message("ChatRoom [%p] state changed: %d", cr, newState);
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

static void core_chat_room_state_changed (LinphoneCore *core, LinphoneChatRoom *cr, LinphoneChatRoomState state) {
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
		linphone_chat_room_cbs_set_undecryptable_message_received(cbs, undecryptable_message_received);
		linphone_chat_room_cbs_set_conference_joined(cbs, chat_room_conference_joined);
		linphone_chat_room_add_callbacks(cr, cbs);
		linphone_chat_room_cbs_unref(cbs);
	}
}

static void configure_core_for_conference (LinphoneCore *core, const char* username, const LinphoneAddress *factoryAddr, bool_t server) {
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
	linphone_core_set_linphone_specs(core, "groupchat");
}

static void _configure_core_for_conference (LinphoneCoreManager *lcm, LinphoneAddress *factoryAddr) {
	configure_core_for_conference(lcm->lc, NULL, factoryAddr, FALSE);
}

static void _configure_core_for_callbacks(LinphoneCoreManager *lcm, LinphoneCoreCbs *cbs) {
	// Remove is-composing callback from the core, we use our own on the chat room
	linphone_core_cbs_set_is_composing_received(lcm->cbs, NULL);
	linphone_core_add_callbacks(lcm->lc, cbs);
	linphone_core_set_user_data(lcm->lc, lcm);
}

static void _start_core(LinphoneCoreManager *lcm) {
	linphone_core_manager_start(lcm, TRUE);
}

static LinphoneChatMessage *_send_message(LinphoneChatRoom *chatRoom, const char *message) {
	LinphoneChatMessage *msg = linphone_chat_room_create_message(chatRoom, message);
	LinphoneChatMessageCbs *msgCbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_msg_state_changed(msgCbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg);
	return msg;
}

static void _send_file_plus_text(LinphoneChatRoom* cr, const char *sendFilepath, const char *text) {
	LinphoneChatMessage *msg;
	LinphoneChatMessageCbs *cbs;
	LinphoneContent *content = linphone_core_create_content(linphone_chat_room_get_core(cr));
	belle_sip_object_set_name(BELLE_SIP_OBJECT(content), "sintel trailer content");
	linphone_content_set_type(content,"video");
	linphone_content_set_subtype(content,"mkv");
	linphone_content_set_name(content,"sintel_trailer_opus_h264.mkv");
	linphone_content_set_file_path(content, sendFilepath);

	msg = linphone_chat_room_create_empty_message(cr);
	linphone_chat_message_add_file_content(msg, content);
	linphone_content_unref(content);

	if (text)
		linphone_chat_message_add_text_content(msg, text);

	cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_file_transfer_send(cbs, tester_file_transfer_send);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);
	linphone_chat_message_send(msg);
	linphone_chat_message_unref(msg);
}

static void _send_file(LinphoneChatRoom* cr, const char *sendFilepath) {
	_send_file_plus_text(cr, sendFilepath, NULL);
}

static void _receive_file(bctbx_list_t *coresList, LinphoneCoreManager *lcm, stats *receiverStats, const char *receive_filepath, const char *sendFilepath) {
	if (BC_ASSERT_TRUE(wait_for_list(coresList, &lcm->stat.number_of_LinphoneMessageReceivedWithFile, receiverStats->number_of_LinphoneMessageReceivedWithFile + 1, 10000))) {
		LinphoneChatMessageCbs *cbs;
		LinphoneChatMessage *msg = lcm->stat.last_received_chat_message;

		cbs = linphone_chat_message_get_callbacks(msg);
		linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
		linphone_chat_message_cbs_set_file_transfer_recv(cbs, file_transfer_received);
		linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);

		LinphoneContent *fileTransferContent = linphone_chat_message_get_file_transfer_information(msg);
		BC_ASSERT_TRUE(linphone_content_is_file_transfer(fileTransferContent));
		linphone_content_set_file_path(fileTransferContent, receive_filepath);
		linphone_chat_message_download_content(msg, fileTransferContent);

		if (BC_ASSERT_TRUE(wait_for_list(coresList, &lcm->stat.number_of_LinphoneFileTransferDownloadSuccessful,receiverStats->number_of_LinphoneFileTransferDownloadSuccessful + 1, 20000))) {
			compare_files(sendFilepath, receive_filepath);
		}
	}
}

static void _receive_file_plus_text(bctbx_list_t *coresList, LinphoneCoreManager *lcm, stats *receiverStats, const char *receive_filepath, const char *sendFilepath, const char *text) {
	if (BC_ASSERT_TRUE(wait_for_list(coresList, &lcm->stat.number_of_LinphoneMessageReceivedWithFile, receiverStats->number_of_LinphoneMessageReceivedWithFile + 1, 10000))) {
		LinphoneChatMessageCbs *cbs;
		LinphoneChatMessage *msg = lcm->stat.last_received_chat_message;

		BC_ASSERT_TRUE(linphone_chat_message_has_text_content(msg));
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text_content(msg), text);

		cbs = linphone_chat_message_get_callbacks(msg);
		linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
		linphone_chat_message_cbs_set_file_transfer_recv(cbs, file_transfer_received);
		linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);

		LinphoneContent *fileTransferContent = linphone_chat_message_get_file_transfer_information(msg);
		BC_ASSERT_TRUE(linphone_content_is_file_transfer(fileTransferContent));
		linphone_content_set_file_path(fileTransferContent, receive_filepath);
		linphone_chat_message_download_content(msg, fileTransferContent);

		if (BC_ASSERT_TRUE(wait_for_list(coresList, &lcm->stat.number_of_LinphoneFileTransferDownloadSuccessful,receiverStats->number_of_LinphoneFileTransferDownloadSuccessful + 1, 20000))) {
			compare_files(sendFilepath, receive_filepath);
		}
	}
}

// Configure list of core manager for conference and add the listener
static bctbx_list_t * init_core_for_conference(bctbx_list_t *coreManagerList) {
	LinphoneAddress *factoryAddr = linphone_address_new(sFactoryUri);
	bctbx_list_for_each2(coreManagerList, (void (*)(void *, void *))_configure_core_for_conference, (void *) factoryAddr);
	linphone_address_unref(factoryAddr);

	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_chat_room_state_changed(cbs, core_chat_room_state_changed);
	bctbx_list_for_each2(coreManagerList, (void (*)(void *, void *))_configure_core_for_callbacks, (void *) cbs);
	linphone_core_cbs_unref(cbs);

	bctbx_list_t *coresList = NULL;
	bctbx_list_t *item = coreManagerList;
	for (item = coreManagerList; item; item = bctbx_list_next(item))
		coresList = bctbx_list_append(coresList, ((LinphoneCoreManager *)(bctbx_list_get_data(item)))->lc);
	return coresList;
}

static void start_core_for_conference(bctbx_list_t *coreManagerList) {
	bctbx_list_for_each(coreManagerList, (void (*)(void *))_start_core);
}

static LinphoneChatRoom * check_creation_chat_room_client_side(bctbx_list_t *lcs, LinphoneCoreManager *lcm, stats *initialStats, const LinphoneAddress *confAddr, const char* subject, int participantNumber, bool_t isAdmin) {
	BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomStateCreationPending, initialStats->number_of_LinphoneChatRoomStateCreationPending + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomStateCreated, initialStats->number_of_LinphoneChatRoomStateCreated + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomConferenceJoined, initialStats->number_of_LinphoneChatRoomConferenceJoined + 1, 3000));
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

static LinphoneChatRoom * create_chat_room_client_side_with_expected_number_of_participants(bctbx_list_t *lcs, LinphoneCoreManager *lcm, stats *initialStats, bctbx_list_t *participantsAddresses, const char* initialSubject, int expectedParticipantSize, bool_t encrypted) {
	LinphoneChatRoom *chatRoom = linphone_core_create_client_group_chat_room_2(lcm->lc, initialSubject, FALSE, encrypted);
	if (!chatRoom) return NULL;

	BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomStateInstantiated, initialStats->number_of_LinphoneChatRoomStateInstantiated + 1, 100));
	if (encrypted)
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(chatRoom) & LinphoneChatRoomCapabilitiesEncrypted);

	// Add participants
	linphone_chat_room_add_participants(chatRoom, participantsAddresses);

	// Check that the chat room is correctly created on Marie's side and that the participants are added
	BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomStateCreationPending, initialStats->number_of_LinphoneChatRoomStateCreationPending + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomStateCreated, initialStats->number_of_LinphoneChatRoomStateCreated + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &lcm->stat.number_of_LinphoneChatRoomConferenceJoined, initialStats->number_of_LinphoneChatRoomConferenceJoined + 1, 3000));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(chatRoom),
		(expectedParticipantSize >= 0) ? expectedParticipantSize : (int)bctbx_list_size(participantsAddresses), int, "%d");
	LinphoneParticipant *participant = linphone_chat_room_get_me(chatRoom);
	BC_ASSERT_PTR_NOT_NULL(participant);
	if (participant)
		BC_ASSERT_TRUE(linphone_participant_is_admin(participant));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(chatRoom), initialSubject);

	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;

	return chatRoom;
}

static LinphoneChatRoom * create_chat_room_client_side(bctbx_list_t *lcs, LinphoneCoreManager *lcm, stats *initialStats, bctbx_list_t *participantsAddresses, const char* initialSubject, bool_t encrypted) {
	return create_chat_room_client_side_with_expected_number_of_participants(lcs, lcm, initialStats, participantsAddresses, initialSubject, -1, encrypted);
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_subject_changed, initialMarieStats.number_of_subject_changed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_subject_changed, initialLaureStats.number_of_subject_changed + 1, 3000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), newSubject);

	// Marie designates Pauline as admin
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneParticipant *paulineParticipant = linphone_chat_room_find_participant(marieCr, paulineAddr);
	linphone_address_unref(paulineAddr);
	BC_ASSERT_PTR_NOT_NULL(paulineParticipant);
	linphone_chat_room_set_participant_admin_status(marieCr, paulineParticipant, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_admin_statuses_changed, initialMarieStats.number_of_participant_admin_statuses_changed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_admin_statuses_changed, initialLaureStats.number_of_participant_admin_statuses_changed + 1, 3000));
	BC_ASSERT_TRUE(linphone_participant_is_admin(paulineParticipant));

	// Pauline adds Chloe to the chat room
	participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	linphone_chat_room_add_participants(paulineCr, participantsAddresses);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;

	// Check that the chat room is correctly created on Chloe's side and that she was added everywhere
	LinphoneChatRoom *chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, newSubject, 3, FALSE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participants_added, initialLaureStats.number_of_participants_added + 1, 3000));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 3, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 3, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(laureCr), 3, int, "%d");

	// Pauline revokes the admin status of Marie
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneParticipant *marieParticipant = linphone_chat_room_find_participant(paulineCr, marieAddr);
	linphone_address_unref(marieAddr);
	BC_ASSERT_PTR_NOT_NULL(marieParticipant);
	linphone_chat_room_set_participant_admin_status(paulineCr, marieParticipant, FALSE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 2, 3000));
	BC_ASSERT_FALSE(linphone_participant_is_admin(marieParticipant));

	// Marie tries to change the subject again but is not admin, so it is not changed
	linphone_chat_room_set_subject(marieCr, initialSubject);
	wait_for_list(coresList, &dummy, 1, 1000);

	// Chloe begins composing a message
	linphone_chat_room_compose(chloeCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived, initialMarieStats.number_of_LinphoneIsComposingActiveReceived + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneIsComposingActiveReceived, initialLaureStats.number_of_LinphoneIsComposingActiveReceived + 1, 3000));
	const char *chloeTextMessage = "Hello";
	LinphoneChatMessage *chloeMessage = _send_message(chloeCr, chloeTextMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDelivered, initialChloeStats.number_of_LinphoneMessageDelivered + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingIdleReceived, initialMarieStats.number_of_LinphoneIsComposingIdleReceived + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingIdleReceived, initialPaulineStats.number_of_LinphoneIsComposingIdleReceived + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneIsComposingIdleReceived, initialLaureStats.number_of_LinphoneIsComposingIdleReceived + 1, 3000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message), chloeTextMessage);
	linphone_chat_message_unref(chloeMessage);
	LinphoneAddress *chloeAddr = linphone_address_new(linphone_core_get_identity(chloe->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(chloeAddr, linphone_chat_message_get_from_address(marie->stat.last_received_chat_message)));
	linphone_address_unref(chloeAddr);

	// Pauline removes Laure from the chat room
	LinphoneAddress *laureAddr = linphone_address_new(linphone_core_get_identity(laure->lc));
	LinphoneParticipant *laureParticipant = linphone_chat_room_find_participant(paulineCr, laureAddr);
	linphone_address_unref(laureAddr);
	BC_ASSERT_PTR_NOT_NULL(laureParticipant);
	linphone_chat_room_remove_participant(paulineCr, laureParticipant);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated, initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_participants_removed, initialChloeStats.number_of_participants_removed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 3000));

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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateTerminated, initialMarieStats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneChatRoomStateTerminated, initialChloeStats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 2, 1000));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 0, int, "%d");

	// Pauline leaves the chat room
	wait_for_list(coresList, &dummy, 1, 1000);
	linphone_chat_room_leave(paulineCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateTerminationPending, initialPaulineStats.number_of_LinphoneChatRoomStateTerminationPending + 1, 100));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateTerminated, initialPaulineStats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));

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
	linphone_core_set_linphone_specs(chloe->lc, ""); // Disable group chat for Chloe

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
	paulineCr = linphone_core_get_chat_room(pauline->lc, paulineAddr);
	linphone_address_unref(paulineAddr);

	// Pauline adds Chloe to the chat room
	participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	linphone_chat_room_add_participants(paulineCr, participantsAddresses);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;

	// Refused by server because group chat disabled for Chloe
	BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &laure->stat.number_of_participants_added, initialLaureStats.number_of_participants_added + 1, 1000));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 2, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 2, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(laureCr), 2, int, "%d");

	// Pauline begins composing a message
	linphone_chat_room_compose(paulineCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived, initialMarieStats.number_of_LinphoneIsComposingActiveReceived + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneIsComposingActiveReceived, initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1, 3000));

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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participants_added, initialLaureStats.number_of_participants_added + 1, 3000));
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

static void group_chat_room_message (bool_t encrypt) {
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived, initialMarieStats.number_of_LinphoneIsComposingActiveReceived + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1, 3000));
	const char *chloeTextMessage = "Hello";
	LinphoneChatMessage *chloeMessage = _send_message(chloeCr, chloeTextMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingIdleReceived, initialMarieStats.number_of_LinphoneIsComposingIdleReceived + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingIdleReceived, initialPaulineStats.number_of_LinphoneIsComposingIdleReceived + 1, 3000));
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived, initialMarieStats.number_of_LinphoneIsComposingActiveReceived + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneIsComposingActiveReceived, initialChloeStats.number_of_LinphoneIsComposingActiveReceived + 1, 3000));
	const char *paulineTextMessage = "Héllö Dàrling";
	LinphoneChatMessage *paulineMessage = _send_message(paulineCr, paulineTextMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageReceived, initialChloeStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingIdleReceived, initialMarieStats.number_of_LinphoneIsComposingIdleReceived + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneIsComposingIdleReceived, initialChloeStats.number_of_LinphoneIsComposingIdleReceived + 1, 3000));
	marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg))
		goto end;

	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marieLastMsg), paulineTextMessage);
	linphone_chat_message_unref(paulineMessage);
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));

	BC_ASSERT_TRUE(linphone_address_weak_equal(paulineAddr, linphone_chat_message_get_from_address(marieLastMsg)));
	linphone_address_unref(paulineAddr);

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
	group_chat_room_message(FALSE);
}

static void group_chat_room_send_message_encrypted(void) {
	group_chat_room_message(TRUE);
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_admin_statuses_changed, initialMarieStats.number_of_participant_admin_statuses_changed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_admin_statuses_changed, initialLaureStats.number_of_participant_admin_statuses_changed + 1, 1000));
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_admin_statuses_changed, initialMarieStats.number_of_participant_admin_statuses_changed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_admin_statuses_changed, initialLaureStats.number_of_participant_admin_statuses_changed + 1, 1000));

	// Make sure pauline is not notified
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 1, 1000));
	linphone_core_set_network_reachable(pauline->lc, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 1, 3000));
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_admin_statuses_changed, initialMarieStats.number_of_participant_admin_statuses_changed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_admin_statuses_changed, initialLaureStats.number_of_participant_admin_statuses_changed + 1, 1000));
	BC_ASSERT_TRUE(linphone_participant_is_admin(paulineParticipant));

	// Pauline revokes the admin status of Marie
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneParticipant *marieParticipant = linphone_chat_room_find_participant(paulineCr, marieAddr);
	linphone_address_unref(marieAddr);
	BC_ASSERT_PTR_NOT_NULL(marieParticipant);
	linphone_chat_room_set_participant_admin_status(paulineCr, marieParticipant, FALSE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_admin_statuses_changed, initialMarieStats.number_of_participant_admin_statuses_changed + 2, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 2, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_admin_statuses_changed, initialLaureStats.number_of_participant_admin_statuses_changed + 2, 1000));
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateTerminated, initialMarieStats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participants_removed, initialLaureStats.number_of_participants_removed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_admin_statuses_changed, initialLaureStats.number_of_participant_admin_statuses_changed + 1, 1000));
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_subject_changed, initialMarieStats.number_of_subject_changed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_subject_changed, initialLaureStats.number_of_subject_changed + 1, 3000));
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_subject_changed, initialMarieStats.number_of_subject_changed, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_subject_changed, initialLaureStats.number_of_subject_changed, 3000));
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated, initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 1000));

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
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated, initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 1000));

	// Laure try to send a message with the chat room where she was removed
	const char *laureTextMessage = "Hello";
	LinphoneChatMessage *laureMessage = _send_message(laureCr, laureTextMessage);
	linphone_chat_message_unref(laureMessage);

	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageDelivered, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingIdleReceived, initialMarieStats.number_of_LinphoneIsComposingIdleReceived, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingIdleReceived, initialPaulineStats.number_of_LinphoneIsComposingIdleReceived, 3000));

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
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateTerminated, initialPaulineStats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participants_removed, initialLaureStats.number_of_participants_removed + 1, 1000));

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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_subject_changed, initialMarieStats.number_of_subject_changed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_subject_changed, initialLaureStats.number_of_subject_changed + 1, 3000));
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
		LinphoneChatMessage *msg = linphone_chat_room_create_message(marieCr, "Salut");
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
		if (BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, 1, 3000))) {
			LinphoneChatMessage *msg = linphone_chat_room_get_last_message_in_history(paulineCr);
			if (BC_ASSERT_PTR_NOT_NULL(msg)) {
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), "Salut");
				linphone_chat_message_unref(msg);
			}
		}
		if (BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, 1, 3000))) {
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
		savedLaureUuid = bctbx_strdup(lp_config_get_string(linphone_core_get_config(laure->lc), "misc", "uuid", NULL));
		coresList = bctbx_list_remove(coresList, laure->lc);
		coresManagerList = bctbx_list_remove(coresManagerList, laure);
		linphone_core_set_network_reachable(laure->lc, FALSE);
		linphone_core_manager_stop(laure);
	}

	// Marie removes Laure from the chat room
	LinphoneParticipant *laureParticipant = linphone_chat_room_find_participant(marieCr, laureAddr);
	BC_ASSERT_PTR_NOT_NULL(laureParticipant);
	linphone_chat_room_remove_participant(marieCr, laureParticipant);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 1000));

	if (offline_when_removed && !offline_when_reinvited) {
		linphone_core_manager_configure(laure);
		lp_config_set_string(linphone_core_get_config(laure->lc), "misc", "uuid", savedLaureUuid);
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, 1000));
	if (offline_when_reinvited) {
		linphone_core_manager_configure(laure);
		lp_config_set_string(linphone_core_get_config(laure->lc), "misc", "uuid", savedLaureUuid);
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_participants_removed, initialMarie1Stats.number_of_participants_removed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participants_removed, initialLaureStats.number_of_participants_removed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneChatRoomStateTerminated, initialPauline1Stats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));

	// Marie2 comes online, check that pauline is not notified as still being in the chat room
	linphone_core_set_network_reachable(marie2->lc, TRUE);
	LinphoneChatRoom *marie2Cr = check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr, initialSubject, 1, TRUE);

	// Marie2 adds Pauline back to the chat room
	initialPauline1Stats = pauline1->stat;
	participantsAddresses = bctbx_list_append(participantsAddresses, paulineAddr);
	linphone_chat_room_add_participants(marie2Cr, participantsAddresses);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_participants_added, initialMarie1Stats.number_of_participants_added + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_participants_added, initialMarie2Stats.number_of_participants_added + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participants_added, initialLaureStats.number_of_participants_added + 1, 1000));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marie1Cr), 2, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marie2Cr), 2, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(laureCr), 2, int, "%d");
	LinphoneChatRoom *newPauline1Cr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr, initialSubject, 2, FALSE);
	BC_ASSERT_PTR_EQUAL(newPauline1Cr, pauline1Cr);
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(newPauline1Cr), 2, int, "%d");
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(newPauline1Cr), initialSubject);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie1, marie1Cr, coresList);
	linphone_core_delete_chat_room(marie2->lc, marie2Cr);
	linphone_core_manager_delete_chat_room(pauline1, newPauline1Cr, coresList);
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
	participantsAddresses = NULL;
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Marie now changes the subject
	const char *newSubject = "New subject";
	linphone_chat_room_set_subject(marieCr, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_subject_changed, initialMarieStats.number_of_subject_changed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_subject_changed, initialLaureStats.number_of_subject_changed + 1, 3000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), newSubject);

	linphone_core_set_network_reachable(pauline->lc, FALSE);
	wait_for_list(coresList, &dummy, 1, 1000);

	// Marie now changes the subject
	newSubject = "Let's go drink a beer";
	linphone_chat_room_set_subject(marieCr, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_subject_changed, initialMarieStats.number_of_subject_changed + 2, 3000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 2, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_subject_changed, initialLaureStats.number_of_subject_changed + 2, 3000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(marieCr), newSubject);
	BC_ASSERT_STRING_NOT_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(laureCr), newSubject);

	linphone_core_set_network_reachable(pauline->lc, TRUE);
	wait_for_list(coresList, &dummy, 1, 1000);

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 2, 3000));

	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);

	// Test with more than one missed notify
	linphone_core_set_network_reachable(pauline->lc, FALSE);
	wait_for_list(coresList, &dummy, 1, 1000);

	// Marie now changes the subject
	newSubject = "Let's go drink a mineral water !";
	linphone_chat_room_set_subject(marieCr, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_subject_changed, initialMarieStats.number_of_subject_changed + 3, 3000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 3, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_subject_changed, initialLaureStats.number_of_subject_changed + 3, 3000));
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_admin_statuses_changed, initialMarieStats.number_of_participant_admin_statuses_changed + 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_admin_statuses_changed, initialLaureStats.number_of_participant_admin_statuses_changed + 1, 1000));
	BC_ASSERT_TRUE(linphone_participant_is_admin(laureParticipant));
	BC_ASSERT_FALSE(linphone_participant_is_admin(laureParticipantFromPauline));

	linphone_core_set_network_reachable(pauline->lc, TRUE);
	wait_for_list(coresList, &dummy, 1, 1000);

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 3, 3000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_room_get_subject(paulineCr), newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 1, 1000));
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated, initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_participants_removed, initialMarie1Stats.number_of_participants_removed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_participants_removed, initialMarie2Stats.number_of_participants_removed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_participants_removed, initialPauline1Stats.number_of_participants_removed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_participants_removed, initialPauline2Stats.number_of_participants_removed + 1, 1000));

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
	_configure_core_for_callbacks(marie2, cbs);
	linphone_core_cbs_unref(cbs);
	coresList = bctbx_list_append(coresList, marie2->lc);
	_start_core(marie2);
	stats initialMarie2Stats = marie2->stat;
	// Check that the chat room is correctly created on second Marie's device
	LinphoneChatRoom *marieCr2 = check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr, initialSubject, 2, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_participant_devices_added, initialMarie1Stats.number_of_participant_devices_added + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_participant_devices_added, initialMarie1Stats.number_of_participant_devices_added + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_participant_devices_added, initialMarie1Stats.number_of_participant_devices_added + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_devices_added, initialMarie1Stats.number_of_participant_devices_added + 1, 1000));

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
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated, initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_participants_removed, initialMarie1Stats.number_of_participants_removed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_participants_removed, initialMarie2Stats.number_of_participants_removed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_participants_removed, initialPauline1Stats.number_of_participants_removed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_participants_removed, initialPauline2Stats.number_of_participants_removed + 1, 1000));

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

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneIsComposingActiveReceived, 1, 1000));

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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingIdleReceived, 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneIsComposingIdleReceived, 1, 1000));

	composing_addresses = linphone_chat_room_get_composing_addresses(marieCr);
	BC_ASSERT_EQUAL(bctbx_list_size(composing_addresses), 0, int, "%i");
	composing_addresses = linphone_chat_room_get_composing_addresses(laureCr);
	BC_ASSERT_EQUAL(bctbx_list_size(composing_addresses), 0, int, "%i");

	// multiple is composing
	linphone_chat_room_compose(paulineCr);
	linphone_chat_room_compose(marieCr);

	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneIsComposingActiveReceived, 3, 1000)); // + 2
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived, 2, 1000)); // + 1
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingIdleReceived, 2, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneIsComposingIdleReceived, 3, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingIdleReceived, 1, 1000));

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
	linphone_core_set_linphone_specs(pauline->lc, NULL);
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreationPending, initialMarieStats.number_of_LinphoneChatRoomStateCreationPending + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated, initialMarieStats.number_of_LinphoneChatRoomStateCreated + 1, 3000));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneChatRoomStateCreationFailed, initialMarieStats.number_of_LinphoneChatRoomStateCreationFailed, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 1, int, "%d");
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesBasic);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;

	// Send a message and check that a basic chat room is created on Pauline's side
	LinphoneChatMessage *msg = linphone_chat_room_create_message(marieCr, "Hey Pauline!");
	linphone_chat_message_send(msg);
	linphone_chat_message_unref(msg);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 1000));
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
	linphone_core_set_linphone_specs(pauline->lc, NULL);
	linphone_core_set_linphone_specs(laure->lc, NULL);
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreationPending, initialMarieStats.number_of_LinphoneChatRoomStateCreationPending + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreationFailed, initialMarieStats.number_of_LinphoneChatRoomStateCreationFailed + 1, 3000));
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
	linphone_core_set_linphone_specs(laure->lc, NULL);
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

	// Create a basic chat room
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneChatRoom *marieCr = linphone_core_get_chat_room(marie->lc, paulineAddr);

	// Send a message and check that a basic chat room is created on Pauline's side
	LinphoneChatMessage *msg = linphone_chat_room_create_message(marieCr, "Hey Pauline!");
	linphone_chat_message_send(msg);
	linphone_chat_message_unref(msg);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 1000));
	BC_ASSERT_PTR_NOT_NULL(pauline->stat.last_received_chat_message);
	if (pauline->stat.last_received_chat_message)
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(pauline->stat.last_received_chat_message), "text/plain");
	LinphoneChatRoom *paulineCr = linphone_core_get_chat_room(pauline->lc, linphone_chat_room_get_local_address(marieCr));
	BC_ASSERT_PTR_NOT_NULL(paulineCr);
	if (paulineCr)
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesBasic);

	// Enable chat room migration and restart core for Marie
	_linphone_chat_room_enable_migration(marieCr, TRUE);
	coresList = bctbx_list_remove(coresList, marie->lc);
	linphone_core_manager_reinit(marie);
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, marie);
	bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_core_manager_start(marie, TRUE);

	marieCr = linphone_core_get_chat_room(marie->lc, paulineAddr);

	// Enable chat room migration and restart core for Pauline
	_linphone_chat_room_enable_migration(paulineCr, TRUE);
	coresList = bctbx_list_remove(coresList, pauline->lc);
	linphone_core_manager_reinit(pauline);
	tmpCoresManagerList = bctbx_list_append(NULL, pauline);
	tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	lp_config_set_int(linphone_core_get_config(pauline->lc), "misc", "enable_basic_to_client_group_chat_room_migration", 1);
	bctbx_list_free(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	linphone_core_manager_start(pauline, TRUE);
	paulineCr = linphone_core_get_chat_room(pauline->lc, linphone_chat_room_get_local_address(marieCr));

	// Send a new message to initiate chat room migration
	BC_ASSERT_PTR_NOT_NULL(marieCr);
	BC_ASSERT_PTR_NOT_NULL(paulineCr);
	if (marieCr && paulineCr) {
		initialMarieStats = marie->stat;
		initialPaulineStats = pauline->stat;
		BC_ASSERT_EQUAL(linphone_chat_room_get_capabilities(marieCr), LinphoneChatRoomCapabilitiesBasic | LinphoneChatRoomCapabilitiesProxy | LinphoneChatRoomCapabilitiesMigratable | LinphoneChatRoomCapabilitiesOneToOne, int, "%d");
		msg = linphone_chat_room_create_message(marieCr, "Did you migrate?");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreationPending, initialMarieStats.number_of_LinphoneChatRoomStateCreationPending + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated, initialMarieStats.number_of_LinphoneChatRoomStateCreated + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomConferenceJoined, initialMarieStats.number_of_LinphoneChatRoomConferenceJoined + 1, 3000));
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesConference);
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 1, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 2, int, "%d");

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreationPending, initialPaulineStats.number_of_LinphoneChatRoomStateCreationPending + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreated, initialPaulineStats.number_of_LinphoneChatRoomStateCreated + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomConferenceJoined, initialPaulineStats.number_of_LinphoneChatRoomConferenceJoined + 1, 3000));
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesConference);
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 1, int, "%d");
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 1000));
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 2, int, "%d");

		msg = linphone_chat_room_create_message(marieCr, "Let's go drink a beer");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 2, 1000));
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 3, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 3, int, "%d");

		msg = linphone_chat_room_create_message(paulineCr, "Let's go drink mineral water instead");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 1000));
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
	linphone_core_set_linphone_specs(pauline->lc, NULL);
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreationPending, initialMarieStats.number_of_LinphoneChatRoomStateCreationPending + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated, initialMarieStats.number_of_LinphoneChatRoomStateCreated + 1, 3000));
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneChatRoomStateCreationFailed, initialMarieStats.number_of_LinphoneChatRoomStateCreationFailed, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 1, int, "%d");
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesBasic);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;

	// Send a message and check that a basic chat room is created on Pauline's side
	LinphoneChatMessage *msg = linphone_chat_room_create_message(marieCr, "Hey Pauline!");
	linphone_chat_message_send(msg);
	linphone_chat_message_unref(msg);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 1000));
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
	linphone_core_manager_start(marie, TRUE);

	// Send a new message to initiate chat room migration
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
		msg = linphone_chat_room_create_message(marieCr, "Did you migrate?");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreationPending, initialMarieStats.number_of_LinphoneChatRoomStateCreationPending + 1, 3000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated, initialMarieStats.number_of_LinphoneChatRoomStateCreated + 1, 3000));
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesBasic);
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 1, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 2, int, "%d");

		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreationPending, initialPaulineStats.number_of_LinphoneChatRoomStateCreationPending + 1, 3000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreated, initialPaulineStats.number_of_LinphoneChatRoomStateCreated + 1, 3000));
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesBasic);
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 1, int, "%d");
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 1000));
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 2, int, "%d");

		msg = linphone_chat_room_create_message(marieCr, "Let's go drink a beer");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 2, 1000));
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 3, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 3, int, "%d");

		msg = linphone_chat_room_create_message(paulineCr, "Let's go drink mineral water instead");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 1000));
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 4, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 4, int, "%d");

		// Activate groupchat on Pauline's side and wait for 5 seconds, the migration should now be done on next message sending
		lp_config_set_int(linphone_core_get_config(marie->lc), "misc", "basic_to_client_group_chat_room_migration_timer", 5);
		_linphone_chat_room_enable_migration(paulineCr, TRUE);
		linphone_chat_room_unref(paulineCr);
		coresList = bctbx_list_remove(coresList, pauline->lc);
		linphone_core_manager_reinit(pauline);
		tmpCoresManagerList = bctbx_list_append(NULL, pauline);
		tmpCoresList = init_core_for_conference(tmpCoresManagerList);
		lp_config_set_int(linphone_core_get_config(pauline->lc), "misc", "enable_basic_to_client_group_chat_room_migration", 1);
		linphone_core_set_linphone_specs(pauline->lc, "groupchat");
		bctbx_list_free(tmpCoresManagerList);
		coresList = bctbx_list_concat(coresList, tmpCoresList);
		linphone_core_manager_start(pauline, TRUE);
		paulineCr = linphone_core_get_chat_room(pauline->lc, linphone_chat_room_get_local_address(marieCr));

		linphone_core_set_network_reachable(pauline->lc, FALSE);
		wait_for_list(coresList, &dummy, 1, 1000);
		linphone_core_set_network_reachable(pauline->lc, TRUE);
		wait_for_list(coresList, &dummy, 1, 5000);
		msg = linphone_chat_room_create_message(marieCr, "And now, did you migrate?");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreationPending, initialMarieStats.number_of_LinphoneChatRoomStateCreationPending + 2, 3000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated, initialMarieStats.number_of_LinphoneChatRoomStateCreated + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomConferenceJoined, initialMarieStats.number_of_LinphoneChatRoomConferenceJoined + 1, 3000));
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesConference);
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 1, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 5, int, "%d");

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreationPending, initialPaulineStats.number_of_LinphoneChatRoomStateCreationPending + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreated, initialPaulineStats.number_of_LinphoneChatRoomStateCreated + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomConferenceJoined, initialPaulineStats.number_of_LinphoneChatRoomConferenceJoined + 1, 3000));
		if (paulineCr) {
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
	LinphoneChatMessage *msg = linphone_chat_room_create_message(marieCr, "Hey Pauline!");
	linphone_chat_message_send(msg);
	linphone_chat_message_unref(msg);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 1000));
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
		msg = linphone_chat_room_create_message(marieCr, "Did you migrate?");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreationPending, initialMarieStats.number_of_LinphoneChatRoomStateCreationPending + 1, 3000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateCreated, initialMarieStats.number_of_LinphoneChatRoomStateCreated + 1, 3000));
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieCr) & LinphoneChatRoomCapabilitiesBasic);
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 1, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 2, int, "%d");

		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreationPending, initialPaulineStats.number_of_LinphoneChatRoomStateCreationPending + 1, 3000));
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateCreated, initialPaulineStats.number_of_LinphoneChatRoomStateCreated + 1, 3000));
		BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesBasic);
		BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 1, int, "%d");
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 1000));
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 2, int, "%d");

		msg = linphone_chat_room_create_message(marieCr, "Let's go drink a beer");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 2, 1000));
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 3, int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 3, int, "%d");

		msg = linphone_chat_room_create_message(paulineCr, "Let's go drink mineral water instead");
		linphone_chat_message_send(msg);
		linphone_chat_message_unref(msg);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 1000));
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

static void group_chat_room_send_file_with_or_without_text (bool_t with_text) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	int dummy = 0;
	char *sendFilepath = bc_tester_res("sounds/sintel_trailer_opus_h264.mkv");
	char *receivePaulineFilepath = bc_tester_file("receive_file_pauline.dump");
	char *receiveChloeFilepath = bc_tester_file("receive_file_chloe.dump");
	const char *text = "Hello Group !";

	/* Globally configure an http file transfer server. */
	linphone_core_set_file_transfer_server(marie->lc, "https://www.linphone.org:444/lft.php");
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
		_send_file_plus_text(marieCr, sendFilepath, text);
	} else {
		_send_file(marieCr, sendFilepath);
	}

	wait_for_list(coresList, &dummy, 1, 10000);

	// Check that chat rooms have received the file
	if (with_text) {
		_receive_file_plus_text(coresList, pauline, &initialPaulineStats, receivePaulineFilepath, sendFilepath, text);
		_receive_file_plus_text(coresList, chloe, &initialChloeStats, receiveChloeFilepath, sendFilepath, text);
	} else {
		_receive_file(coresList, pauline, &initialPaulineStats, receivePaulineFilepath, sendFilepath);
		_receive_file(coresList, chloe, &initialChloeStats, receiveChloeFilepath, sendFilepath);
	}

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	remove(receivePaulineFilepath);
	remove(receiveChloeFilepath);
	bc_free(sendFilepath);
	bc_free(receivePaulineFilepath);
	bc_free(receiveChloeFilepath);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(chloe);
}

static void group_chat_room_send_file (void) {
	group_chat_room_send_file_with_or_without_text(FALSE);
}

static void group_chat_room_send_file_plus_text (void) {
	group_chat_room_send_file_with_or_without_text(TRUE);
}

static void group_chat_room_unique_one_to_one_chat_room (void) {
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, initialMarieStats.number_of_LinphoneMessageDelivered + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, initialMarieStats.number_of_LinphoneMessageDelivered + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	// Check that the created address is the same as before
	const LinphoneAddress *newConfAddr = linphone_chat_room_get_conference_address(marieCr);
	BC_ASSERT_TRUE(linphone_address_weak_equal(confAddr, newConfAddr));

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	linphone_address_unref(confAddr);
	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_room_unique_one_to_one_chat_room_recreated_from_message_base (bool_t with_app_restart) {
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, initialMarieStats.number_of_LinphoneMessageDelivered + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

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
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDelivered, initialPaulineStats.number_of_LinphoneMessageDelivered + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	// Check that the chat room has been correctly recreated on Marie's side
	marieCr = check_creation_chat_room_client_side(coresList, marie, &initialMarieStats, confAddr, initialSubject, 1, FALSE);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineCr) & LinphoneChatRoomCapabilitiesOneToOne);

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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, initialMarieStats.number_of_LinphoneMessageDelivered + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
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
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, initialMarieStats.number_of_LinphoneMessageDelivered + 1, 3000));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
		linphone_chat_message_unref(message);
	}

	// Clean db from chat room
	linphone_core_set_network_reachable(marie2->lc, TRUE);
	linphone_core_set_network_reachable(pauline2->lc, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneChatRoomStateCreationPending, initialMarie2Stats.number_of_LinphoneChatRoomStateCreationPending + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneChatRoomStateCreated, initialMarie2Stats.number_of_LinphoneChatRoomStateCreated + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneChatRoomConferenceJoined, initialMarie2Stats.number_of_LinphoneChatRoomConferenceJoined + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_LinphoneChatRoomStateCreationPending, initialPauline2Stats.number_of_LinphoneChatRoomStateCreationPending + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_LinphoneChatRoomStateCreated, initialPauline2Stats.number_of_LinphoneChatRoomStateCreated + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_LinphoneChatRoomConferenceJoined, initialPauline2Stats.number_of_LinphoneChatRoomConferenceJoined + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_LinphoneMessageReceived, initialPauline2Stats.number_of_LinphoneMessageReceived + 1, 3000));
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateTerminated, initialMarieStats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateTerminated, initialPaulineStats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneChatRoomStateTerminated, initialMarie2Stats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_LinphoneChatRoomStateTerminated, initialPauline2Stats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));

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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_LinphoneMessageDelivered, initialMarie1Stats.number_of_LinphoneMessageDelivered + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	// Pauline answers to the previous message
	textMessage = "Hey. How are you?";
	message = _send_message(paulineCr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDelivered, initialPaulineStats.number_of_LinphoneMessageDelivered + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie1->stat.number_of_LinphoneMessageReceived, initialMarie1Stats.number_of_LinphoneMessageReceived + 1, 3000));
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageDelivered, initialMarie2Stats.number_of_LinphoneMessageDelivered + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 2, 3000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), textMessage);
	linphone_chat_message_unref(message);

	// Pauline answers to the previous message
	textMessage = "Perfect!";
	message = _send_message(paulineCr, textMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDelivered, initialPaulineStats.number_of_LinphoneMessageDelivered + 2, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageReceived, initialMarie2Stats.number_of_LinphoneMessageReceived + 1, 3000));
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
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	LinphoneChatRoom *chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);

	// Chloe begins composing a message
	const char *chloeTextMessage = "Hello";
	LinphoneChatMessage *chloeMessage = _send_message(chloeCr, chloeTextMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
	LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marieLastMsg), chloeTextMessage);
	LinphoneAddress *chloeAddr = linphone_address_new(linphone_core_get_identity(chloe->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(chloeAddr, linphone_chat_message_get_from_address(marieLastMsg)));
	linphone_address_unref(chloeAddr);

	// Check that the message has been delivered to Marie and Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDeliveredToUser, initialChloeStats.number_of_LinphoneMessageDeliveredToUser + 1, 3000));
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDisplayed, initialChloeStats.number_of_LinphoneMessageDisplayed + 1, 3000));
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
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

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
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	LinphoneChatRoom *chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);

	// Chloe begins composing a message
	const char *chloeTextMessage = "Hello";
	const char *chloeTextMessage2 = "Long time no talk";
	const char *chloeTextMessage3 = "How are you?";
	LinphoneChatMessage *chloeMessage = _send_message(chloeCr, chloeTextMessage);
	LinphoneChatMessage *chloeMessage2 = _send_message(chloeCr, chloeTextMessage2);
	LinphoneChatMessage *chloeMessage3 = _send_message(chloeCr, chloeTextMessage3);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 3, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 3, 3000));
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDisplayed, initialChloeStats.number_of_LinphoneMessageDisplayed + 1, 3000));
	BC_ASSERT_EQUAL(chloe->stat.number_of_LinphoneMessageDeliveredToUser, 0, int, "%d");
	if (read_while_offline) {
		wait_for_list(coresList, 0, 1, 2000); // To prevent memory leak
	}

	linphone_chat_message_unref(chloeMessage3);
	linphone_chat_message_unref(chloeMessage2);
	linphone_chat_message_unref(chloeMessage);

end:
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

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
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	LinphoneAddress *confAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieCr));

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	LinphoneChatRoom *chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);

	// Chloe begins composing a message
	const char *chloeTextMessage = "Hello";
	LinphoneChatMessage *chloeMessage = _send_message(chloeCr, chloeTextMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
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
	linphone_address_unref(confAddr);

	// Check that the message has been delivered to Marie and Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageDeliveredToUser, initialChloeStats.number_of_LinphoneMessageDeliveredToUser + 1, 3000));
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
	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

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
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	LinphoneChatRoom *chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);

	// Chloe leave the chat room
	linphone_chat_room_leave(chloeCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneChatRoomStateTerminationPending, initialChloeStats.number_of_LinphoneChatRoomStateTerminationPending + 1, 100));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneChatRoomStateTerminated, initialChloeStats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, 1000));

	LinphoneChatRoom *oneToOneChatRoom = linphone_core_find_one_to_one_chat_room(marie->lc, marieAddr, paulineAddr);
	BC_ASSERT_PTR_NULL(oneToOneChatRoom);

	// Marie create a one to one chat room with Pauline
	participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	LinphoneChatRoom *marieOneToOneCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, "one to one", FALSE);
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

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieOneToOneCr, coresList);
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

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
	LinphoneChatRoom *marie1Cr = create_chat_room_client_side(coresList, marie1, &initialMarie1Stats, participantsAddresses, initialSubject, FALSE);
	participantsAddresses = NULL;
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marie1Cr);

	// Check that the chat room is correctly created on Pauline1 and Pauline2's sides and that the participants are added
	LinphoneChatRoom *pauline1Cr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr, initialSubject, 2, FALSE);
	LinphoneChatRoom *pauline2Cr = check_creation_chat_room_client_side(coresList, pauline2, &initialPauline2Stats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Marie adds a new device
	LinphoneCoreManager *marie2 = linphone_core_manager_create("marie_rc");
	stats initialMarie2Stats = marie2->stat;
	bctbx_list_t *tmpCoresManagerList = bctbx_list_append(NULL, marie2);
	bctbx_list_t *tmpCoresList = init_core_for_conference(tmpCoresManagerList);
	coresList = bctbx_list_concat(coresList, tmpCoresList);
	start_core_for_conference(tmpCoresManagerList);
	bctbx_list_free(tmpCoresManagerList);
	LinphoneChatRoom *marie2Cr = check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr, initialSubject, 2, TRUE);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie1, marie1Cr, coresList);
	linphone_core_delete_chat_room(marie2->lc, marie2Cr);
	linphone_core_manager_delete_chat_room(pauline1, pauline1Cr, coresList);
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

static void group_chat_room_list_subscription (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
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
	LinphoneChatRoom *marieCr1 = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	const LinphoneAddress *confAddr1 = linphone_chat_room_get_conference_address(marieCr1);
	// Check that the chat room is correctly created on Pauline1 and Pauline2's sides and that the participants are added
	LinphoneChatRoom *paulineCr1 = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr1, initialSubject, 2, FALSE);
	LinphoneChatRoom *laureCr1 = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr1, initialSubject, 2, FALSE);
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	initialSubject = "Friends";
	participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	LinphoneChatRoom *marieCr2 = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	const LinphoneAddress *confAddr2 = linphone_chat_room_get_conference_address(marieCr2);
	// Check that the chat room is correctly created on Pauline1 and Pauline2's sides and that the participants are added
	LinphoneChatRoom *paulineCr2 = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr2, initialSubject, 2, FALSE);
	LinphoneChatRoom *laureCr2 = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr2, initialSubject, 2, FALSE);
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	initialLaureStats = laure->stat;

	// Marie creates a new group chat room
	initialSubject = "Bros";
	participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	LinphoneChatRoom *marieCr3 = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	const LinphoneAddress *confAddr3 = linphone_chat_room_get_conference_address(marieCr3);
	// Check that the chat room is correctly created on Pauline1 and Pauline2's sides and that the participants are added
	LinphoneChatRoom *paulineCr3 = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr3, initialSubject, 2, FALSE);
	LinphoneChatRoom *laureCr3 = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr3, initialSubject, 2, FALSE);

	participantsAddresses = NULL;

	// Marie designates Pauline as admin in chat room 1
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
	LinphoneParticipant *paulineParticipant = linphone_chat_room_find_participant(marieCr1, paulineAddr);
	linphone_address_unref(paulineAddr);
	BC_ASSERT_PTR_NOT_NULL(paulineParticipant);
	linphone_chat_room_set_participant_admin_status(marieCr1, paulineParticipant, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_admin_statuses_changed, initialMarieStats.number_of_participant_admin_statuses_changed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_admin_statuses_changed, initialLaureStats.number_of_participant_admin_statuses_changed + 1, 1000));
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_admin_statuses_changed, initialMarieStats.number_of_participant_admin_statuses_changed + 2, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_admin_statuses_changed, initialLaureStats.number_of_participant_admin_statuses_changed + 2, 1000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 2, 1000));
	BC_ASSERT_TRUE(linphone_participant_is_admin(laureParticipant1));
	linphone_chat_room_set_participant_admin_status(marieCr3, laureParticipant3, TRUE);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_admin_statuses_changed, initialMarieStats.number_of_participant_admin_statuses_changed + 3, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_admin_statuses_changed, initialLaureStats.number_of_participant_admin_statuses_changed + 3, 1000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 3, 1000));
	BC_ASSERT_TRUE(linphone_participant_is_admin(laureParticipant3));

	// Marie now changes the subject or chat room 1
	const char *newSubject = "New subject";
	linphone_chat_room_set_subject(marieCr1, newSubject);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_subject_changed, initialMarieStats.number_of_subject_changed + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_subject_changed, initialLaureStats.number_of_subject_changed + 1, 10000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_subject_changed, initialPaulineStats.number_of_subject_changed + 1, 10000));
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_admin_statuses_changed, initialPaulineStats.number_of_participant_admin_statuses_changed + 2, 1000));
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

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr1, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr1, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr1, coresList);
	linphone_core_manager_delete_chat_room(marie, marieCr2, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr2, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr2, coresList);
	linphone_core_manager_delete_chat_room(marie, marieCr3, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr3, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr3, coresList);

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
	check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Restart Laure core
	linphone_core_set_network_reachable(laure->lc, FALSE);
	char *laureIdentity = linphone_core_get_device_identity(laure->lc);
	LinphoneAddress *laureAddr = linphone_address_new(laureIdentity);
	bctbx_free(laureIdentity);
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated, initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));

	wait_for_list(coresList,0, 1, 2000);
	initialLaureStats = laure->stat;

	linphone_proxy_config_refresh_register(linphone_core_get_default_proxy_config(laure->lc));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneRegistrationOk, initialLaureStats.number_of_LinphoneRegistrationOk + 1, 1000));
	BC_ASSERT_FALSE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateTerminated, initialLaureStats.number_of_LinphoneChatRoomStateTerminated + 1, 3000));

	// Marie adds Laure to the chat room
	participantsAddresses = bctbx_list_append(participantsAddresses, laureAddr);
	linphone_chat_room_add_participants(marieCr, participantsAddresses);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateCreationPending, initialLaureStats.number_of_LinphoneChatRoomStateCreationPending + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomStateCreated, initialLaureStats.number_of_LinphoneChatRoomStateCreated + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneChatRoomConferenceJoined, initialLaureStats.number_of_LinphoneChatRoomConferenceJoined + 1, 3000));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 2, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 2, int, "%d");
	laureIdentity = linphone_core_get_device_identity(laure->lc);
	laureAddr = linphone_address_new(laureIdentity);
	bctbx_free(laureIdentity);
	LinphoneChatRoom *newLaureCr = linphone_core_find_chat_room(laure->lc, confAddr, laureAddr);
	linphone_address_unref(laureAddr);
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
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneRegistrationOk, initialLaureStats.number_of_LinphoneRegistrationOk + 1, 1000));
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

static const int x3dhServerDelay = 3000; // TODO replace me with X3DH server callbacks

static void group_chat_lime_x3dh_create_lime_user (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));

	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
}

static void group_chat_lime_x3dh_encrypted_chatrooms (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	LinphoneChatRoom *marieEncryptedCr = NULL;
	LinphoneChatRoom *paulineEncryptedCr = NULL;
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie creates a new regular chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *mariePlainCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);
	LinphoneAddress *plainConfAddr = linphone_address_clone(linphone_chat_room_get_conference_address(mariePlainCr));
	BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(mariePlainCr) & LinphoneChatRoomCapabilitiesBasic);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(mariePlainCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(mariePlainCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulinePlainCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, plainConfAddr, initialSubject, 1, 0);
	BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(paulinePlainCr) & LinphoneChatRoomCapabilitiesBasic);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulinePlainCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(paulinePlainCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Marie sends a plain message
	const char *marieMessage = "Hey ! What's up ?";
	_send_message(mariePlainCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message is received by Pauline
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr);

	// Reset stats for new chatroom creation
	reset_counters(&marie->stat);
	reset_counters(&pauline->stat);

	// Marie creates an encrypted chatroom
	initialSubject = "Encrypted Friends";
	participantsAddresses = bctbx_list_append(NULL, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	marieEncryptedCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	LinphoneAddress *encryptedConfAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieEncryptedCr));
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineEncryptedCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, encryptedConfAddr, initialSubject, 1, 0);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineEncryptedCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineEncryptedCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Marie sends an encrypted message
	marieMessage = "We can say whatever we want in this chatrooom!";
	_send_message(marieEncryptedCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message is received and decrypted by Pauline
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr);

	// Marie deletes the regular chat room
	linphone_core_manager_delete_chat_room(marie, mariePlainCr, coresList);
	wait_for_list(coresList, 0, 1, 2000);
	BC_ASSERT_EQUAL(pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed, int, "%d");

	// Marie creates the regular chat room again
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	initialSubject = "Friends";
	participantsAddresses = bctbx_list_append(NULL, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	mariePlainCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	// Marie sends a new plain message
	marieMessage = "Hey again";
	_send_message(mariePlainCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, initialMarieStats.number_of_LinphoneMessageDelivered + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), marieMessage);

	// Check that the recreated regular chat room address is the same as before and the capabilities are correct
	const LinphoneAddress *newPlainConfAddr = linphone_chat_room_get_conference_address(mariePlainCr);
	BC_ASSERT_TRUE(linphone_address_weak_equal(plainConfAddr, newPlainConfAddr));
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(mariePlainCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(mariePlainCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Marie deletes the encrypted chat room
	linphone_core_manager_delete_chat_room(marie, marieEncryptedCr, coresList);
	wait_for_list(coresList, 0, 1, 2000);
	BC_ASSERT_EQUAL(pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed, int, "%d");

	// Marie creates the encrypted chat room again
	initialMarieStats = marie->stat;
	initialPaulineStats = pauline->stat;
	initialSubject = "Encrypted Friends";
	participantsAddresses = bctbx_list_append(NULL, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	marieEncryptedCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);

	// Marie sends a new encrypted message
	marieMessage = "Hey again from the encrypted chatroom";
	_send_message(marieEncryptedCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDelivered, initialMarieStats.number_of_LinphoneMessageDelivered + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), marieMessage);

	// Check that the recreated encrypted chat room address is the same as before and the capabilities are correct
	const LinphoneAddress *newEncryptedConfAddr = linphone_chat_room_get_conference_address(marieEncryptedCr);
	BC_ASSERT_TRUE(linphone_address_weak_equal(encryptedConfAddr, newEncryptedConfAddr));
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesEncrypted);

	linphone_address_unref(plainConfAddr);
	linphone_address_unref(encryptedConfAddr);

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, mariePlainCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulinePlainCr, coresList);
	if (marieEncryptedCr) linphone_core_manager_delete_chat_room(marie, marieEncryptedCr, coresList);
	if (paulineEncryptedCr) linphone_core_manager_delete_chat_room(pauline, paulineEncryptedCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_basic_chat_rooms (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Send message in basic chat room
	LinphoneChatRoom *marieBasicCr = linphone_core_get_chat_room(marie->lc, pauline->identity);
	LinphoneChatMessage *basicMessage1 = linphone_chat_room_create_message(marieBasicCr, "Hello from our basic chat room");
	LinphoneChatMessageCbs *cbs1 = linphone_chat_message_get_callbacks(basicMessage1);
	linphone_chat_message_cbs_set_msg_state_changed(cbs1, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(basicMessage1);

	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneMessageDelivered,initialMarieStats.number_of_LinphoneMessageReceived + 1));
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneMessageReceived,initialPaulineStats.number_of_LinphoneMessageReceived + 1));
	BC_ASSERT_PTR_NOT_NULL(pauline->stat.last_received_chat_message);
	if (pauline->stat.last_received_chat_message != NULL) {
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(pauline->stat.last_received_chat_message), "text/plain");
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), linphone_chat_message_get_text(basicMessage1));
	}
	LinphoneChatRoom *paulineBasicCr = linphone_core_get_chat_room(pauline->lc, marie->identity);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieBasicCr) & LinphoneChatRoomCapabilitiesBasic);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineBasicCr) & LinphoneChatRoomCapabilitiesBasic);
	BC_ASSERT_PTR_NOT_NULL(paulineBasicCr);
	linphone_chat_message_unref(basicMessage1);

	// Marie creates an encrypted chatroom
	const char *initialSubject = "Encrypted Friends";
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	LinphoneChatRoom *marieEncryptedCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	LinphoneAddress *encryptedConfAddr = linphone_address_clone(linphone_chat_room_get_conference_address(marieEncryptedCr));
	BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesBasic);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(marieEncryptedCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineEncryptedCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, encryptedConfAddr, initialSubject, 1, 0);
	BC_ASSERT_FALSE(linphone_chat_room_get_capabilities(paulineEncryptedCr) & LinphoneChatRoomCapabilitiesBasic);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineEncryptedCr) & LinphoneChatRoomCapabilitiesOneToOne);
	BC_ASSERT_TRUE(linphone_chat_room_get_capabilities(paulineEncryptedCr) & LinphoneChatRoomCapabilitiesEncrypted);

	// Marie sends an encrypted message
	const char *marieEncryptedMessage1 = "Hello from our secured chat room";
	_send_message(marieEncryptedCr, marieEncryptedMessage1);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 2, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message is received and decrypted by Pauline
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieEncryptedMessage1);
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));

	// Marie deletes the basic chat room
	linphone_core_manager_delete_chat_room(marie, marieBasicCr, coresList);
	wait_for_list(coresList, 0, 1, 2000);
	BC_ASSERT_EQUAL(pauline->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed, int, "%d");

	// Marie creates the basic chat room again
	LinphoneChatRoom *marieNewBasicCr = linphone_core_get_chat_room(marie->lc, pauline->identity);
	LinphoneChatMessage *basicMessage2 = linphone_chat_room_create_message(marieNewBasicCr, "Hello again from our basic chat room");
	LinphoneChatMessageCbs *cbs2 = linphone_chat_message_get_callbacks(basicMessage2);
	linphone_chat_message_cbs_set_msg_state_changed(cbs2, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(basicMessage2);

	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&marie->stat.number_of_LinphoneMessageDelivered,initialMarieStats.number_of_LinphoneMessageReceived + 3));
	BC_ASSERT_TRUE(wait_for(marie->lc,pauline->lc,&pauline->stat.number_of_LinphoneMessageReceived,initialPaulineStats.number_of_LinphoneMessageReceived + 3));
	BC_ASSERT_PTR_NOT_NULL(pauline->stat.last_received_chat_message);
	if (pauline->stat.last_received_chat_message != NULL) {
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(pauline->stat.last_received_chat_message), "text/plain");
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline->stat.last_received_chat_message), linphone_chat_message_get_text(basicMessage2));
	}
	linphone_chat_message_unref(basicMessage2);
	linphone_address_unref(encryptedConfAddr);
	linphone_address_unref(marieAddr);

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(pauline, paulineBasicCr, coresList);
	linphone_core_manager_delete_chat_room(marie, marieEncryptedCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineEncryptedCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void lime_x3dh_message_test (bool_t with_composing, bool_t with_response) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	if (with_composing) {
		// Marie starts composing a message
		linphone_chat_room_compose(marieCr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1, 10000));
	}

	// Marie sends the message
	const char *marieMessage = "Hey ! What's up ?";
	_send_message(marieCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message was correctly decrypted
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr);

	if (with_response) {
		if (with_composing) {
			// Pauline starts composing a response
			linphone_chat_room_compose(paulineCr);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneIsComposingActiveReceived, initialMarieStats.number_of_LinphoneIsComposingActiveReceived + 1, 10000));
		}

		// Pauline sends the response
		const char *paulineMessage = "I'm fine thank you ! And you ?";
		_send_message(paulineCr, paulineMessage);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 10000));
		LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
		if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg))
			goto end;

		// Check that the response was correctly decrypted
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marieLastMsg), paulineMessage);
		LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
		BC_ASSERT_TRUE(linphone_address_weak_equal(paulineAddr, linphone_chat_message_get_from_address(marieLastMsg)));
		linphone_address_unref(paulineAddr);
	}

	// Check chat room security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_send_encrypted_message (void) {
	lime_x3dh_message_test(FALSE, FALSE);
}

static void group_chat_lime_x3dh_send_encrypted_message_with_composing (void) {
	lime_x3dh_message_test(TRUE, FALSE);
}

static void group_chat_lime_x3dh_send_encrypted_message_with_response (void) {
	lime_x3dh_message_test(FALSE, TRUE);
}

static void group_chat_lime_x3dh_send_encrypted_message_with_response_and_composing (void) {
	lime_x3dh_message_test(TRUE, TRUE);
}

static void group_chat_lime_x3dh_encrypted_message_to_devices_with_and_without_keys (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));
	BC_ASSERT_FALSE(linphone_core_lime_x3dh_enabled(laure->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, 0);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, 0);

	// Marie sends the message
	const char *marieMessage = "Hey ! What's up ?";
	_send_message(marieCr, marieMessage);

	// Check that Pauline received and decrypted the message
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr);

	// Check that Laure did not receive the message because she did not post keys on the X3DH server
	BC_ASSERT_FALSE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 1000));

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);
	linphone_core_delete_local_encryption_db(laure->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_lime_x3dh_send_encrypted_file_with_or_without_text (bool_t with_text) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	int dummy = 0;
	char *sendFilepath = bc_tester_res("sounds/sintel_trailer_opus_h264.mkv");
	char *receivePaulineFilepath = bc_tester_file("receive_file_pauline.dump");
	char *receiveChloeFilepath = bc_tester_file("receive_file_chloe.dump");
	const char *text = "Hello Group !";

	// Globally configure an http file transfer server
	linphone_core_set_file_transfer_server(marie->lc, "https://www.linphone.org:444/lft.php");
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

	// Remove any previously downloaded file
	remove(receivePaulineFilepath);
	remove(receiveChloeFilepath);

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(chloe->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	LinphoneChatRoom *chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);

	// Send encrypted file
	if (with_text) {
		_send_file_plus_text(marieCr, sendFilepath, text);
	} else {
		_send_file(marieCr, sendFilepath);
	}

	// Check that chat rooms have received the file
	if (with_text) {
		_receive_file_plus_text(coresList, pauline, &initialPaulineStats, receivePaulineFilepath, sendFilepath, text);
		_receive_file_plus_text(coresList, chloe, &initialChloeStats, receiveChloeFilepath, sendFilepath, text);
	} else {
		_receive_file(coresList, pauline, &initialPaulineStats, receivePaulineFilepath, sendFilepath);
		_receive_file(coresList, chloe, &initialChloeStats, receiveChloeFilepath, sendFilepath);
	}

	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);
	linphone_core_delete_local_encryption_db(chloe->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	remove(receivePaulineFilepath);
	remove(receiveChloeFilepath);
	bc_free(sendFilepath);
	bc_free(receivePaulineFilepath);
	bc_free(receiveChloeFilepath);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(chloe);
}

static void group_chat_lime_x3dh_send_encrypted_file (void) {
	group_chat_lime_x3dh_send_encrypted_file_with_or_without_text(FALSE);
}

static void group_chat_lime_x3dh_send_encrypted_file_plus_text (void) {
	group_chat_lime_x3dh_send_encrypted_file_with_or_without_text(TRUE);
}

bool_t simple_zrtp_call_with_sas_validation(LinphoneCoreManager *caller, LinphoneCoreManager *callee, bool_t callerValidation, bool_t calleeValidation) {
	bool_t call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok=call(caller, callee)));
	if (!call_ok) return FALSE;

	// If caller set ZRTP or (callee set ZRTP and caller has no encryption requested), ZRTP shall take place, wait for the SAS
	if ((linphone_core_get_media_encryption(caller->lc) == LinphoneMediaEncryptionZRTP)
		|| ((linphone_core_get_media_encryption(callee->lc) == LinphoneMediaEncryptionZRTP) && (linphone_core_get_media_encryption(caller->lc) == LinphoneMediaEncryptionNone))) {

		// Simulate SAS validation or invalidation
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(caller->lc), callerValidation);
		linphone_call_set_authentication_token_verified(linphone_core_get_current_call(callee->lc), calleeValidation);
		BC_ASSERT_EQUAL(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(caller->lc)), callerValidation, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_get_authentication_token_verified(linphone_core_get_current_call(callee->lc)), calleeValidation, int, "%d");
	}
	end_call(caller, callee);
	return TRUE;
}

static void group_chat_lime_x3dh_verify_sas_before_message (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	LinphoneChatRoom *marieCr = NULL;
	LinphoneChatRoom *paulineCr = NULL;
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Enable ZRTP cache
	const char *filepath;
	const char *filepath2;
	remove(bc_tester_file("tmpZIDCacheMarie.sqlite"));
	remove(bc_tester_file("tmpZIDCachePauline.sqlite"));
	filepath = bc_tester_file("tmpZIDCacheMarie.sqlite");
	filepath2 = bc_tester_file("tmpZIDCachePauline.sqlite");
	linphone_core_set_media_encryption(marie->lc,LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionZRTP);
	linphone_core_set_zrtp_secrets_file(marie->lc, filepath);
	linphone_core_set_zrtp_secrets_file(pauline->lc, filepath2);

	// Check ZRTP status
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");

	// ZRTP verification call between Marie and Pauline
	bool_t call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, TRUE, TRUE)));
	if (!call_ok) goto end;

	// Check ZRTP status
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusValid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	// Check LIME X3DH and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusValid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	// Marie sends a message
	const char *marieMessage = "I have a sensitive piece of information for you";
	_send_message(marieCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message was correctly decrypted by Pauline
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));

	// Pauline sends a response
	const char *paulineMessage = "Are you sure this conversation is secure ?";
	_send_message(paulineCr, paulineMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg))
		goto end;

	// Check that the response was correctly decrypted
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marieLastMsg), paulineMessage);
	BC_ASSERT_TRUE(linphone_address_weak_equal(paulineAddr, linphone_chat_message_get_from_address(marieLastMsg)));

	// Check LIME X3DH and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusValid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	// ZRTP verification call Marie rejects the SAS
	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, FALSE, TRUE)));
	if (!call_ok) goto end;

	// Check LIME X3DH and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelUnsafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusInvalid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	// Delete chatrooms
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	// Check ZRTP status
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusInvalid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	linphone_address_unref(marieAddr);
	linphone_address_unref(paulineAddr);

end:
	// Clean local ZRTP databases
	remove(bc_tester_file("tmpZIDCacheMarie.sqlite"));
	remove(bc_tester_file("tmpZIDCachePauline.sqlite"));

	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_reject_sas_before_message (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	LinphoneChatRoom *marieCr = NULL;
	LinphoneChatRoom *paulineCr = NULL;
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Enable ZRTP cache
	const char *filepath;
	const char *filepath2;
	remove(bc_tester_file("tmpZIDCacheMarie.sqlite"));
	remove(bc_tester_file("tmpZIDCachePauline.sqlite"));
	filepath = bc_tester_file("tmpZIDCacheMarie.sqlite");
	filepath2 = bc_tester_file("tmpZIDCachePauline.sqlite");
	linphone_core_set_media_encryption(marie->lc,LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionZRTP);
	linphone_core_set_zrtp_secrets_file(marie->lc, filepath);
	linphone_core_set_zrtp_secrets_file(pauline->lc, filepath2);

	// Check ZRTP status
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");

	// ZRTP verification call Marie rejects the SAS
	bool_t call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, FALSE, TRUE)));
	if (!call_ok) goto end;

	// Check ZRTP status
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusInvalid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	// Check LIME X3DH and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelUnsafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusInvalid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	// Marie sends a message
	const char *marieMessage = "I have a sensitive piece of information for you";
	_send_message(marieCr, marieMessage);

	if (lp_config_get_int(linphone_core_get_config(marie->lc), "lime", "allow_message_in_unsafe_chatroom", 0) == 1) {
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
		LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
		if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
			goto end;

		// Check that the message was correctly decrypted by Pauline
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
		BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	} else {
		BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));
	}

	// Pauline sends a response
	const char *paulineMessage = "Are you sure this conversation is secure ?";
	_send_message(paulineCr, paulineMessage);

	// Marie does not receive Pauline's message because Pauline is unsafe for Marie
	BC_ASSERT_FALSE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 3000));

	// ZRTP verification call between Marie and Pauline
	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, TRUE, TRUE)));
	if (!call_ok) goto end;

	// Check LIME X3DH and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusValid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	// Delete chatrooms
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	// Check ZRTP status
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusValid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	linphone_address_unref(marieAddr);
	linphone_address_unref(paulineAddr);

end:
	// Clean local ZRTP databases
	remove(bc_tester_file("tmpZIDCacheMarie.sqlite"));
	remove(bc_tester_file("tmpZIDCachePauline.sqlite"));

	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_message_before_verify_sas (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	LinphoneChatRoom *marieCr = NULL;
	LinphoneChatRoom *paulineCr = NULL;
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Enable ZRTP cache
	const char *filepath;
	const char *filepath2;
	remove(bc_tester_file("tmpZIDCacheMarie.sqlite"));
	remove(bc_tester_file("tmpZIDCachePauline.sqlite"));
	filepath = bc_tester_file("tmpZIDCacheMarie.sqlite");
	filepath2 = bc_tester_file("tmpZIDCachePauline.sqlite");
	linphone_core_set_media_encryption(marie->lc,LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionZRTP);
	linphone_core_set_zrtp_secrets_file(marie->lc, filepath);
	linphone_core_set_zrtp_secrets_file(pauline->lc, filepath2);

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	// Check LIME X3DH and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");

	// Marie sends a message
	const char *marieMessage = "I have a sensitive piece of information for you";
	_send_message(marieCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message was correctly decrypted by Pauline
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));

	// Pauline sends a response
	const char *paulineMessage = "Are you sure this conversation is secure ?";
	_send_message(paulineCr, paulineMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageReceived, initialMarieStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *marieLastMsg = marie->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marieLastMsg))
		goto end;

	// Check that the response was correctly decrypted
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marieLastMsg), paulineMessage);
	BC_ASSERT_TRUE(linphone_address_weak_equal(paulineAddr, linphone_chat_message_get_from_address(marieLastMsg)));

	// Check LIME X3DH and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");

	// ZRTP verification call between Marie and Pauline
	bool_t call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, TRUE, TRUE)));
	if (!call_ok) goto end;

	// Check LIME X3DH and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusValid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	linphone_address_unref(marieAddr);
	linphone_address_unref(paulineAddr);

end:
	// Clean local ZRTP databases
	remove(bc_tester_file("tmpZIDCacheMarie.sqlite"));
	remove(bc_tester_file("tmpZIDCachePauline.sqlite"));

	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_message_before_reject_sas (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Enable ZRTP cache
	const char *filepath;
	const char *filepath2;

	remove(bc_tester_file("tmpZIDCacheMarie.sqlite"));
	remove(bc_tester_file("tmpZIDCachePauline.sqlite"));
	filepath = bc_tester_file("tmpZIDCacheMarie.sqlite");
	filepath2 = bc_tester_file("tmpZIDCachePauline.sqlite");

	linphone_core_set_media_encryption(marie->lc,LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(pauline->lc,LinphoneMediaEncryptionZRTP);
	linphone_core_set_zrtp_secrets_file(marie->lc, filepath);
	linphone_core_set_zrtp_secrets_file(pauline->lc, filepath2);

	// Check ZRTP status
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	// Marie sends a message to the chatroom
	const char *marieMessage = "Hi Pauline, how are you ?";
	_send_message(marieCr, marieMessage);

	// Check that the message was correctly received and decrypted by Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));

	// Check LIME and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusUnknown, int , "%d");

	// ZRTP call both validate the SAS
	bool_t call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, TRUE, TRUE)));
	if (!call_ok) goto end;

	// Check LIME and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusValid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	// ZRTP call Marie rejects the SAS
	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, FALSE, TRUE)));
	if (!call_ok) goto end;

	// Check LIME and ZRTP status
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelUnsafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(marie->lc, linphone_address_as_string_uri_only(paulineAddr)), LinphoneZrtpPeerStatusInvalid, int , "%d");
	BC_ASSERT_EQUAL(linphone_core_get_zrtp_status(pauline->lc, linphone_address_as_string_uri_only(marieAddr)), LinphoneZrtpPeerStatusValid, int , "%d");

	// Check security event
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_ManInTheMiddleDetected, initialMarieStats.number_of_ManInTheMiddleDetected + 1, 3000));

	linphone_address_unref(marieAddr);
	linphone_address_unref(paulineAddr);

end:
	// Clean local ZRTP databases
	remove(bc_tester_file("tmpZIDCacheMarie.sqlite"));
	remove(bc_tester_file("tmpZIDCachePauline.sqlite"));

	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}


static void group_chat_lime_x3dh_chatroom_security_level_upgrade (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_lime_x3dh_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;
	stats initialChloeStats = chloe->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(chloe->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline and Laure sides and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 3, 0);
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 3, 0);
	LinphoneChatRoom *chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 3, 0);

	// Marie sends a message to the chatroom
	const char *marieMessage = "Hey guys ! What's up ?";
	_send_message(marieCr, marieMessage);

	// Check that the message was correctly received and decrypted by Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr1 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr1, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr1);

	// Check that the message was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(laureLastMsg), marieMessage);
	LinphoneAddress *marieAddr2 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr2, linphone_chat_message_get_from_address(laureLastMsg)));
	linphone_address_unref(marieAddr2);

	// Check that the message was correctly received and decrypted by Chloe
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneMessageReceived, initialChloeStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *chloeLastMsg = chloe->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(chloeLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(chloeLastMsg), marieMessage);
	LinphoneAddress *marieAddr3 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr3, linphone_chat_message_get_from_address(chloeLastMsg)));
	linphone_address_unref(marieAddr3);

	// Check chat room security level is encrypted
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(chloeCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(laure->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(chloe->lc, LinphoneMediaEncryptionZRTP);

	// ZRTP verification call between Marie and Pauline
	bool_t pauline_call_ok = FALSE;
	BC_ASSERT_TRUE((pauline_call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, TRUE, TRUE)));
	if (!pauline_call_ok) goto end;

	// Check chat room security level has not changed
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(chloeCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

	// ZRTP verification call between Marie and Laure
	bool_t laure_call_ok = FALSE;
	BC_ASSERT_TRUE((laure_call_ok = simple_zrtp_call_with_sas_validation(marie, laure, TRUE, TRUE)));
	if (!laure_call_ok) goto end;

	// Check chat room security level has not changed
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(chloeCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

	// ZRTP verification call between Marie and Chloe
	bool_t chloe_call_ok = FALSE;
	BC_ASSERT_TRUE((chloe_call_ok = simple_zrtp_call_with_sas_validation(marie, chloe, TRUE, TRUE)));
	if (!chloe_call_ok) goto end;

	// Check that Marie is now in a safe chatroom
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(chloeCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);
	linphone_core_delete_local_encryption_db(laure->lc);
	linphone_core_delete_local_encryption_db(chloe->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(chloe);
}

static void group_chat_lime_x3dh_chatroom_security_level_downgrade_adding_participant (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_lime_x3dh_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_lime_x3dh_rc");
	LinphoneChatRoom *chloeCr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;
	stats initialChloeStats = chloe->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(chloe->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline and Laure sides and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, 0);
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, 0);

	// Marie sends a message to the chatroom
	const char *marieMessage = "Hey guys ! What's up ?";
	_send_message(marieCr, marieMessage);

	// Check that the message was correctly received and decrypted by Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr1 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr1, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr1);

	// Check that the message was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(laureLastMsg), marieMessage);
	LinphoneAddress *marieAddr2 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr2, linphone_chat_message_get_from_address(laureLastMsg)));
	linphone_address_unref(marieAddr2);

	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(laure->lc, LinphoneMediaEncryptionZRTP);

	// ZRTP verification call between Marie and Pauline
	bool_t call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, TRUE, TRUE)));
	if (!call_ok) goto end;

	// ZRTP verification call between Marie and Laure
	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, laure, TRUE, TRUE)));
	if (!call_ok) goto end;

	// ZRTP verification call between Pauline and Laure
	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(pauline, laure, TRUE, TRUE)));
	if (!call_ok) goto end;

	// Check that the maximum security level is reached for everyone
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");

	// Marie adds Chloe to the chat room
	participantsAddresses = NULL;
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));
	linphone_chat_room_add_participants(marieCr, participantsAddresses);
	bctbx_list_free_with_data(participantsAddresses, (bctbx_list_free_func)linphone_address_unref);
	participantsAddresses = NULL;

	// Check that the chat room is correctly created on Chloe's side and that she was added everywhere
	chloeCr = check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 3, 0);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participants_added, initialLaureStats.number_of_participants_added + 1, 3000));
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(marieCr), 3, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(paulineCr), 3, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_nb_participants(laureCr), 3, int, "%d");

	// Check the chat room security level got downgraded
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(chloeCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

	// Check that participants have received a SecurityLevelDowngraded event
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_SecurityLevelDowngraded, initialLaureStats.number_of_SecurityLevelDowngraded + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_SecurityLevelDowngraded, initialLaureStats.number_of_SecurityLevelDowngraded + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_SecurityLevelDowngraded, initialLaureStats.number_of_SecurityLevelDowngraded + 1, 3000));

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);
	linphone_core_delete_local_encryption_db(laure->lc);
	linphone_core_delete_local_encryption_db(chloe->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	if (chloeCr) linphone_core_manager_delete_chat_room(chloe, chloeCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(chloe);
}

static void group_chat_lime_x3dh_chatroom_security_level_downgrade_resetting_zrtp (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline and Laure sides and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, 0);
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, 0);

	// Marie sends a message to the chatroom
	const char *marieMessage = "Hey guys ! What's up ?";
	_send_message(marieCr, marieMessage);

	// Check that the message was correctly received and decrypted by Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr1 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr1, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr1);

	// Check that the message was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(laureLastMsg), marieMessage);
	LinphoneAddress *marieAddr2 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr2, linphone_chat_message_get_from_address(laureLastMsg)));
	linphone_address_unref(marieAddr2);

	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(laure->lc, LinphoneMediaEncryptionZRTP);

	// ZRTP verification call between Marie and Pauline
	bool_t call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, TRUE, TRUE)));
	if (!call_ok) goto end;

	// ZRTP verification call between Marie and Laure
	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, laure, TRUE, TRUE)));
	if (!call_ok) goto end;

	// ZRTP verification call between Pauline and Laure
	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(pauline, laure, TRUE, TRUE)));
	if (!call_ok) goto end;

	// Check that the maximum security level is reached for everyone
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");

	// New call with ZRTP verification but pauline refuses the SAS
	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(pauline, marie, FALSE, TRUE)));
	if (!call_ok) goto end;

	// Check the chat room security level got downgraded for Pauline
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	if (lp_config_get_int(linphone_core_get_config(pauline->lc), "lime", "unsafe_if_sas_refused", 1) == 1) {
		BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelUnsafe, int, "%d");
	} else {
		BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	}

	// Check that pauline's chatroom received a security event
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_ManInTheMiddleDetected, initialPaulineStats.number_of_ManInTheMiddleDetected + 1, 3000));

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);
	linphone_core_delete_local_encryption_db(laure->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_lime_x3dh_chatroom_security_alert (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline1 = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_lime_x3dh_rc");
	LinphoneCoreManager *pauline2 = NULL;
	LinphoneChatRoom *pauline2Cr = NULL;
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline1);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPauline1Stats = pauline1->stat;
	stats initialLaureStats = laure->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline1->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline1 and Laure sides and that the participants are added
	LinphoneChatRoom *pauline1Cr = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr, initialSubject, 2, 0);
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, 0);

	// Marie sends a message to the chatroom
	const char *marieMessage = "Hey guys ! What's up ?";
	_send_message(marieCr, marieMessage);

	// Check that the message was correctly received and decrypted by Pauline1
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneMessageReceived, initialPauline1Stats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *pauline1LastMsg = pauline1->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(pauline1LastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline1LastMsg), marieMessage);
	LinphoneAddress *marieAddr1 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr1, linphone_chat_message_get_from_address(pauline1LastMsg)));
	linphone_address_unref(marieAddr1);
	pauline1LastMsg = NULL;

	// Check that the message was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(laureLastMsg), marieMessage);
	LinphoneAddress *marieAddr2 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr2, linphone_chat_message_get_from_address(laureLastMsg)));
	linphone_address_unref(marieAddr2);
	laureLastMsg = NULL;

	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(pauline1->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(laure->lc, LinphoneMediaEncryptionZRTP);

	// ZRTP verification call between Marie and Pauline1
	bool_t call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline1, TRUE, TRUE)));
	if (!call_ok) goto end;

	// ZRTP verification call between Marie and Laure
	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, laure, TRUE, TRUE)));
	if (!call_ok) goto end;

	// ZRTP verification call between Pauline1 and Laure
	call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(pauline1, laure, TRUE, TRUE)));
	if (!call_ok) goto end;

	// Check that the maximum security level is reached for everyone
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(pauline1Cr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");

	// Marie sends a message to the chatroom
	marieMessage = "What are you doing tonight ?";
	_send_message(marieCr, marieMessage);

	// Check that the message was correctly received and decrypted by Pauline1
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneMessageReceived, initialPauline1Stats.number_of_LinphoneMessageReceived + 2, 10000));
	pauline1LastMsg = pauline1->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(pauline1LastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline1LastMsg), marieMessage);
	pauline1LastMsg = NULL;

	// Check that the message was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 2, 10000));
	laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(laureLastMsg), marieMessage);

	// Create second device for Pauline
	pauline2 = linphone_core_manager_create("pauline_lime_x3dh_rc");
	stats initialPauline2Stats = pauline2->stat;
	bctbx_list_t *newCoresManagerList = bctbx_list_append(NULL, pauline2);
	bctbx_list_t *newCoresList = init_core_for_conference(newCoresManagerList);
	start_core_for_conference(newCoresManagerList);
	coresManagerList = bctbx_list_concat(coresManagerList, newCoresManagerList);
	coresList = bctbx_list_concat(coresList, newCoresList);

	// Wait for Pauline2 lime user to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline2->lc));
	linphone_core_set_media_encryption(pauline2->lc, LinphoneMediaEncryptionZRTP);

	// Pauline2 is automatically added to the chatroom

	// Check that the chat room is correctly created on Pauline2's side and that she was added everywhere
	pauline2Cr = check_creation_chat_room_client_side(coresList, pauline2, &initialPauline2Stats, confAddr, initialSubject, 2, 0);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_devices_added, initialMarieStats.number_of_participant_devices_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_participant_devices_added, initialPauline1Stats.number_of_participant_devices_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_devices_added, initialLaureStats.number_of_participant_devices_added + 1, 3000));

	// Check that the participants have received a security alert because Pauline2 is forbidden
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_ParticipantMaxDeviceCountExceeded, initialMarieStats.number_of_ParticipantMaxDeviceCountExceeded + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_ParticipantMaxDeviceCountExceeded, initialPauline1Stats.number_of_ParticipantMaxDeviceCountExceeded + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_ParticipantMaxDeviceCountExceeded, initialLaureStats.number_of_ParticipantMaxDeviceCountExceeded + 1, 3000));

	// Check the security level was downgraded for Marie and Laure
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelUnsafe, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelUnsafe, int, "%d");

	// Laure sends a messages to trigger a LIME X3DH security alerts because maxNumberOfDevicePerParticipant has been exceeded
	if (lp_config_get_int(linphone_core_get_config(laure->lc), "lime", "allow_message_in_unsafe_chatroom", 0) == 0) {
		const char *laureMessage = "I'm going to the cinema";
		_send_message(laureCr, laureMessage);
		wait_for_list(coresList, &dummy, 1, 500);
		BC_ASSERT_FALSE((marie->stat.number_of_LinphoneMessageReceived == initialPauline1Stats.number_of_LinphoneMessageReceived + 3));
		BC_ASSERT_FALSE((pauline1->stat.number_of_LinphoneMessageReceived == initialPauline1Stats.number_of_LinphoneMessageReceived + 3));
		BC_ASSERT_FALSE((pauline2->stat.number_of_LinphoneMessageReceived == initialPauline2Stats.number_of_LinphoneMessageReceived + 1));
	}

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline1->lc);
	if (pauline2) linphone_core_delete_local_encryption_db(pauline2->lc);
	linphone_core_delete_local_encryption_db(laure->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline1, pauline1Cr, coresList);
	if (pauline2Cr) linphone_core_manager_delete_chat_room(pauline2, pauline2Cr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline1);
	if (pauline2) linphone_core_manager_destroy(pauline2);
	linphone_core_manager_destroy(laure);
}

static void group_chat_lime_x3dh_call_security_alert (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for both participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	// Marie sends the message
	const char *marieMessage = "Hey ! What's up ?";
	_send_message(marieCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message was correctly decrypted
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr);

	// Check chatroom security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

	linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionZRTP);
	linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionZRTP);

	// ZRTP verification call between Marie and Pauline
	bool_t call_ok = FALSE;
	BC_ASSERT_TRUE((call_ok = simple_zrtp_call_with_sas_validation(marie, pauline, TRUE, FALSE)));
	if (!call_ok) goto end;

	// Check chatroom security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelSafe, int, "%d");
	if (lp_config_get_int(linphone_core_get_config(pauline->lc), "lime", "unsafe_if_sas_refused", 1) == 1) {
		BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelUnsafe, int, "%d");
	} else {
		BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	}

	// Check chatroom security event
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_ManInTheMiddleDetected, initialPaulineStats.number_of_ManInTheMiddleDetected + 1, 3000));

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_send_multiple_successive_encrypted_messages (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialLaureStats = laure->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for all participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, 0);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, pauline, &initialLaureStats, confAddr, initialSubject, 2, 0);

	// Marie sends the message
	const char *marieMessage1 = "Hey !";
	_send_message(marieCr, marieMessage1);

	// Check that message 1 was correctly received and decrypted by Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage1);
	LinphoneAddress *marieAddr1 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr1, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr1);
	paulineLastMsg = NULL;

	// Check that message 1 was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(laureLastMsg), marieMessage1);
	LinphoneAddress *marieAddr2 = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr2, linphone_chat_message_get_from_address(laureLastMsg)));
	linphone_address_unref(marieAddr2);
	laureLastMsg = NULL;

	// Marie sends another message
	const char *marieMessage2 = "What's up ?";
	_send_message(marieCr, marieMessage2);

	// Check that message 2 was correctly received and decrypted by Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 2, 10000));
	paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage2);
	paulineLastMsg = NULL;

	// Check that message 2 was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 2, 10000));
	laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(laureLastMsg), marieMessage2);
	laureLastMsg = NULL;

	// Marie sends yet another message
	const char *marieMessage3 = "I need to talk to you.";
	_send_message(marieCr, marieMessage3);

	// Check that message 3 was correctly received and decrypted by Pauline
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 3, 10000));
	paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage3);

	// Check that message 3 was correctly received and decrypted by Laure
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 3, 10000));
	laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(laureLastMsg), marieMessage3);

	// Check chatroom security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);
	linphone_core_delete_local_encryption_db(laure->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);
// 	linphone_core_manager_delete_chat_room(laure, laureCr, coresList); // TODO crash in c-wrapper because Cpp Object is null

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
}

static void group_chat_lime_x3dh_send_encrypted_message_to_disabled_lime_x3dh (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	// Pauline disables LIME X3DH
	linphone_core_enable_lime_x3dh(pauline->lc, FALSE);

	// Check encryption status
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_FALSE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie starts composing a message
	linphone_chat_room_compose(marieCr);

	// Check that the IsComposing is undecipherable and that an undecipherable message error IMDN is returned to Marie
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1, 1000));

	// Marie sends the message
	const char *marieMessage = "What's up ?";
	_send_message(marieCr, marieMessage);

	// Check that the message is discarded and that an undecipherable message error IMDN is returned to Marie
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 1000));

	// Check the chatrooms security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelClearText, int, "%d");

	// Clean local LIME X3DH databases
	linphone_core_enable_lime_x3dh(pauline->lc, TRUE);
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_send_plain_message_to_enabled_lime_x3dh (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	// Marie disables LIME X3DH
	linphone_core_enable_lime_x3dh(marie->lc, FALSE);

	// Check encryption status
	BC_ASSERT_FALSE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie starts composing a message
	linphone_chat_room_compose(marieCr);

	// Check that the IsComposing is correctly discarded
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1, 3000));

	// Marie sends a message
	const char *marieMessage = "What's up ?";
	_send_message(marieCr, marieMessage);

	// Check that the message is correctly discarded
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 3000));

	// Clean local LIME X3DH databases
	linphone_core_enable_lime_x3dh(marie->lc, TRUE);
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_send_encrypted_message_to_multidevice_participants (void) {
	LinphoneCoreManager *marie1 = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *marie2 = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline1 = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *pauline2 = linphone_core_manager_create("pauline_lime_x3dh_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie1);
	coresManagerList = bctbx_list_append(coresManagerList, marie2);
	coresManagerList = bctbx_list_append(coresManagerList, pauline1);
	coresManagerList = bctbx_list_append(coresManagerList, pauline2);
	coresManagerList = bctbx_list_append(coresManagerList, laure);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline1->lc)));
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(laure->lc)));
	stats initialMarie1Stats = marie1->stat;
	stats initialMarie2Stats = marie2->stat;
	stats initialPauline1Stats = pauline1->stat;
	stats initialPauline2Stats = pauline2->stat;
	stats initialLaureStats = laure->stat;

	// Wait for lime users to be created on x3dh server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status for all participants
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie1->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie2->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline1->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline2->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure->lc));

	// Change the value of max_nb_device_per_participant to allow multidevice
	linphone_config_set_int(linphone_core_get_config(marie1->lc), "lime", "max_nb_device_per_participant", 2);
	linphone_config_set_int(linphone_core_get_config(marie2->lc), "lime", "max_nb_device_per_participant", 2);
	linphone_config_set_int(linphone_core_get_config(pauline1->lc), "lime", "max_nb_device_per_participant", 2);
	linphone_config_set_int(linphone_core_get_config(pauline2->lc), "lime", "max_nb_device_per_participant", 2);
	linphone_config_set_int(linphone_core_get_config(laure->lc), "lime", "max_nb_device_per_participant", 2);

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr1 = create_chat_room_client_side(coresList, marie1, &initialMarie1Stats, participantsAddresses, initialSubject, TRUE);
	participantsAddresses = NULL;
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr1);

	// Check that the chat room is correctly created on Marie's second device
	LinphoneChatRoom *marieCr2 = check_creation_chat_room_client_side(coresList, marie2, &initialMarie2Stats, confAddr, initialSubject, 2, 1);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr1 = check_creation_chat_room_client_side(coresList, pauline1, &initialPauline1Stats, confAddr, initialSubject, 2, 0);
	LinphoneChatRoom *paulineCr2 = check_creation_chat_room_client_side(coresList, pauline2, &initialPauline2Stats, confAddr, initialSubject, 2, 0);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, 0);

	// Check chatroom security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr2), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr1), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr2), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

	// Marie sends a message
	const char *marie1Message = "Hey ! What's up guys ?";
	_send_message(marieCr1, marie1Message);

	// Check that the message was received by everybody
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie2->stat.number_of_LinphoneMessageReceived, initialMarie2Stats.number_of_LinphoneMessageReceived + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline1->stat.number_of_LinphoneMessageReceived, initialPauline1Stats.number_of_LinphoneMessageReceived + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_LinphoneMessageReceived, initialPauline2Stats.number_of_LinphoneMessageReceived + 1, 10000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_LinphoneMessageReceived, initialLaureStats.number_of_LinphoneMessageReceived + 1, 10000));

	LinphoneChatMessage *marie2LastMsg = marie2->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(marie2LastMsg))
		goto end;
	LinphoneChatMessage *pauline1LastMsg = pauline1->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(pauline1LastMsg))
		goto end;
	LinphoneChatMessage *pauline2LastMsg = pauline2->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(pauline2LastMsg))
		goto end;
	LinphoneChatMessage *laureLastMsg = laure->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg))
		goto end;

	// Check that the messages were correctly decrypted by everybody
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie2LastMsg), marie1Message);
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline1LastMsg), marie1Message);
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline2LastMsg), marie1Message);
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(laureLastMsg), marie1Message);

	// Check chatroom security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr2), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr1), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr2), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(laureCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie1->lc);
	linphone_core_delete_local_encryption_db(marie2->lc);
	linphone_core_delete_local_encryption_db(pauline1->lc);
	linphone_core_delete_local_encryption_db(pauline2->lc);
	linphone_core_delete_local_encryption_db(laure->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie1, marieCr1, coresList);
	linphone_core_manager_delete_chat_room(marie2, marieCr2, coresList);
	linphone_core_manager_delete_chat_room(pauline1, paulineCr1, coresList);
	linphone_core_manager_delete_chat_room(pauline2, paulineCr2, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie1);
	linphone_core_manager_destroy(marie2);
	linphone_core_manager_destroy(pauline1);
	linphone_core_manager_destroy(pauline2);
	linphone_core_manager_destroy(laure);
}

static void group_chat_lime_x3dh_message_while_network_unreachable (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;

	// Wait for lime users to be created on X3DH server
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	// Simulate pauline has disconnected
	linphone_core_set_network_reachable(pauline->lc, FALSE);

	// Marie starts composing a message
	linphone_chat_room_compose(marieCr);

	// Marie sends the message
	const char *marieMessage = "Hey ! What's up ?";
	_send_message(marieCr, marieMessage);

	// Reconnect pauline
	linphone_core_set_network_reachable(pauline->lc, TRUE);

	// Check if the message is received
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message was correctly decrypted
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr);

	// Check chatroom security level
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(marieCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");
	BC_ASSERT_EQUAL(linphone_chat_room_get_security_level(paulineCr), LinphoneChatRoomSecurityLevelEncrypted, int, "%d");

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_lime_x3dh_update_keys (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_lime_x3dh_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_lime_x3dh_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	int dummy = 0;

	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);

	// Wait for lime user creation
	wait_for_list(coresList, &dummy, 1, x3dhServerDelay);

	// Check encryption status
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline->lc));

	// Marie creates a new group chat room
	const char *initialSubject = "Friends";
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, TRUE);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 1, 0);

	// Marie starts composing a message
	linphone_chat_room_compose(marieCr);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, initialPaulineStats.number_of_LinphoneIsComposingActiveReceived + 1, 10000));

	// Marie sends the message
	const char *marieMessage = "Hey ! What's up ?";
	_send_message(marieCr, marieMessage);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageReceived, initialPaulineStats.number_of_LinphoneMessageReceived + 1, 10000));
	LinphoneChatMessage *paulineLastMsg = pauline->stat.last_received_chat_message;
	if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg))
		goto end;

	// Check that the message was correctly decrypted
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(paulineLastMsg), marieMessage);
	LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
	BC_ASSERT_TRUE(linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
	linphone_address_unref(marieAddr);

	// Set the last time a lime update was performed to an recent date
	LinphoneConfig *config = linphone_core_get_config(marie->lc);
	time_t recentUpdateTime = ms_time(NULL)-22000; // 6 hours = 21600 ms
	linphone_config_set_int(config, "misc", "last_update_time", (int)recentUpdateTime);

	linphone_core_set_network_reachable(marie->lc, FALSE);
	wait_for_list(coresList, &dummy, 1, 2000);
	linphone_core_set_network_reachable(marie->lc, TRUE);

	// Check if Marie's encryption is still active after restart
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));

	// Check that we have not performed an update
	int newUpdateTime = linphone_config_get_int(config, "misc", "last_update_time", -1);
	BC_ASSERT_EQUAL(newUpdateTime, (int)recentUpdateTime, int, "%d");

	// Set the last time a lime update was performed to an old date
	time_t oldUpdateTime = ms_time(NULL)-88000; // 24 hours = 86400 ms
	linphone_config_set_int(config, "misc", "last_update_time", (int)oldUpdateTime);

	linphone_core_set_network_reachable(marie->lc, FALSE);
	wait_for_list(coresList, &dummy, 1, 2000);
	linphone_core_set_network_reachable(marie->lc, TRUE);

	// Check if Marie's encryption is still active after restart
	BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie->lc));

	// Wait for update callback
	wait_for_list(coresList, &dummy, 1, 2000);

	// Check that we correctly performed an update
	newUpdateTime = linphone_config_get_int(config, "misc", "last_update_time", -1);
	BC_ASSERT_GREATER(newUpdateTime, (int)oldUpdateTime, int, "%d");

end:
	// Clean local LIME X3DH databases
	linphone_core_delete_local_encryption_db(marie->lc);
	linphone_core_delete_local_encryption_db(pauline->lc);

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void group_chat_room_subscription_denied (void) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *laure = linphone_core_manager_create("laure_tcp_rc");
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
	LinphoneChatRoom *marieCr = create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject, FALSE);

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats, confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Laure's side and that the participants are added
	LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure, &initialLaureStats, confAddr, initialSubject, 2, FALSE);

	// Simulate pauline has disconnected
	linphone_core_set_network_reachable(pauline->lc, FALSE);

	LinphoneParticipant *paulineParticipant = linphone_chat_room_find_participant(marieCr, paulineAddress);
	BC_ASSERT_PTR_NOT_NULL(paulineParticipant);
	linphone_chat_room_remove_participant(marieCr, paulineParticipant);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_removed, initialMarieStats.number_of_participants_removed + 1, 1000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participants_removed, initialPaulineStats.number_of_participants_removed + 1, 1000));

	// Reconnect pauline
	linphone_core_set_network_reachable(pauline->lc, TRUE);

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateTerminated, initialPaulineStats.number_of_LinphoneChatRoomStateTerminated + 1, 1000));

	// Clean db from chat room
	linphone_core_manager_delete_chat_room(marie, marieCr, coresList);
	linphone_core_manager_delete_chat_room(laure, laureCr, coresList);
	linphone_core_manager_delete_chat_room(pauline, paulineCr, coresList);

	linphone_address_unref(paulineAddress);

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(laure);
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
	linphone_core_set_user_agent(laure->lc, "(Laure device)", NULL);
	linphone_core_set_user_agent(chloe->lc, "blabla (Chloe device 1) blibli/blublu (bloblo)", NULL);
	linphone_core_set_user_agent(chloe2->lc, "blabla (Chloe device 2) blibli/blublu (bloblo)", NULL);
	start_core_for_conference(coresManagerList);
	LinphoneAddress *marieAddress = linphone_address_new(linphone_core_get_identity(marie->lc));
	LinphoneAddress *paulineAddress = linphone_address_ref(linphone_address_new(linphone_core_get_identity(pauline->lc)));
	LinphoneAddress *laureAddress = linphone_address_ref(linphone_address_new(linphone_core_get_identity(laure->lc)));
	LinphoneAddress *chloeAddress = linphone_address_new(linphone_core_get_identity(chloe->lc));
	const LinphoneAddress *marieDeviceAddress =  linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(marie->lc));
	const LinphoneAddress *paulineDeviceAddress1 =  linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(pauline->lc));
	const LinphoneAddress *paulineDeviceAddress2 =  linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(pauline2->lc));
	const LinphoneAddress *laureDeviceAddress =  linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(laure->lc));
	const LinphoneAddress *chloeDeviceAddress1 =  linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(chloe->lc));
	const LinphoneAddress *chloeDeviceAddress2 =  linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(chloe2->lc));
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

	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

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
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(laureDevice), "Laure device");

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
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(laureDevice), "Laure device");

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
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(laureDevice), "Laure device");

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
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(laureDevice), "Laure device");

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
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participants_added, initialMarieStats.number_of_participants_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_devices_added, initialMarieStats.number_of_participant_devices_added + 2, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participants_added, initialPaulineStats.number_of_participants_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_devices_added, initialPaulineStats.number_of_participant_devices_added + 2, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_participants_added, initialPaulineStats2.number_of_participants_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_participant_devices_added, initialPaulineStats2.number_of_participant_devices_added + 2, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participants_added, initialLaureStats.number_of_participants_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_devices_added, initialLaureStats.number_of_participant_devices_added + 2, 3000));

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
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(laureDevice), "Laure device");
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
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(laureDevice), "Laure device");
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
	const LinphoneAddress *chloeDeviceAddress3 =  linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(chloe3->lc));
	LinphoneChatRoom *chloeCr3 = check_creation_chat_room_client_side(coresList, chloe3, &initialChloeStats3, confAddr, initialSubject, 3, 0);
	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_participant_devices_added, initialMarieStats.number_of_participant_devices_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_participant_devices_added, initialPaulineStats.number_of_participant_devices_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2->stat.number_of_participant_devices_added, initialPaulineStats2.number_of_participant_devices_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &laure->stat.number_of_participant_devices_added, initialLaureStats.number_of_participant_devices_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_participant_devices_added, initialChloeStats.number_of_participant_devices_added + 1, 3000));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe2->stat.number_of_participant_devices_added, initialChloeStats2.number_of_participant_devices_added + 1, 3000));

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
	BC_ASSERT_STRING_EQUAL(linphone_participant_device_get_name(laureDevice), "Laure device");
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

	linphone_address_unref(marieAddress);
	linphone_address_unref(paulineAddress);
	linphone_address_unref(laureAddress);
	linphone_address_unref(chloeAddress);

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

test_t group_chat_tests[] = {
	TEST_NO_TAG("Group chat room creation server", group_chat_room_creation_server),
	TEST_ONE_TAG("Add participant", group_chat_room_add_participant, "LeaksMemory"),
	TEST_NO_TAG("Send message", group_chat_room_send_message),
	TEST_NO_TAG("Send encrypted message", group_chat_room_send_message_encrypted),
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
	TEST_NO_TAG("Come back on a group chat room after a disconnection", group_chat_room_come_back_after_disconnection),
	TEST_ONE_TAG("Create chat room with disconnected friends", group_chat_room_create_room_with_disconnected_friends, "LeaksMemory"),
	TEST_ONE_TAG("Create chat room with disconnected friends and initial message", group_chat_room_create_room_with_disconnected_friends_and_initial_message, "LeaksMemory"),
	TEST_NO_TAG("Reinvited after removed from group chat room", group_chat_room_reinvited_after_removed),
	TEST_ONE_TAG("Reinvited after removed from group chat room 2", group_chat_room_reinvited_after_removed_2, "LeaksMemory"),
	TEST_ONE_TAG("Reinvited after removed from group chat room while offline", group_chat_room_reinvited_after_removed_while_offline, "LeaksMemory"),
	TEST_ONE_TAG("Reinvited after removed from group chat room while offline 2", group_chat_room_reinvited_after_removed_while_offline_2, "LeaksMemory"),
	TEST_NO_TAG("Reinvited after removed from group chat room with several devices", group_chat_room_reinvited_after_removed_with_several_devices),
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
	TEST_NO_TAG("Send file + text", group_chat_room_send_file_plus_text),
	TEST_NO_TAG("Unique one-to-one chatroom", group_chat_room_unique_one_to_one_chat_room),
	TEST_NO_TAG("Unique one-to-one chatroom recreated from message", group_chat_room_unique_one_to_one_chat_room_recreated_from_message),
	TEST_ONE_TAG("Unique one-to-one chatroom recreated from message with app restart", group_chat_room_unique_one_to_one_chat_room_recreated_from_message_with_app_restart, "LeaksMemory"),
	TEST_NO_TAG("Join one-to-one chat room with a new device", group_chat_room_join_one_to_one_chat_room_with_a_new_device),
	TEST_NO_TAG("New unique one-to-one chatroom after both participants left", group_chat_room_new_unique_one_to_one_chat_room_after_both_participants_left),
	TEST_ONE_TAG("Unique one-to-one chatroom re-created from the party that deleted it, with inactive devices", group_chat_room_unique_one_to_one_chat_room_recreated_from_message_2, "LeaksMemory"),
	TEST_NO_TAG("IMDN for group chat room", imdn_for_group_chat_room),
	TEST_NO_TAG("Aggregated IMDN for group chat room", aggregated_imdn_for_group_chat_room),
	TEST_NO_TAG("Aggregated IMDN for group chat room read while offline", aggregated_imdn_for_group_chat_room_read_while_offline),
	TEST_ONE_TAG("IMDN sent from DB state", imdn_sent_from_db_state, "LeaksMemory"),
	TEST_NO_TAG("Find one to one chat room", find_one_to_one_chat_room),
	TEST_NO_TAG("New device after group chat room creation", group_chat_room_new_device_after_creation),
	TEST_ONE_TAG("Chat room list subscription", group_chat_room_list_subscription, "LeaksMemory"),
	TEST_ONE_TAG("Complex participant removal scenario", group_chat_room_complex_participant_removal_scenario, "LeaksMemory"),
	TEST_ONE_TAG("LIME X3DH create lime user", group_chat_lime_x3dh_create_lime_user, "LimeX3DH"),
	TEST_TWO_TAGS("LIME X3DH encrypted chatrooms", group_chat_lime_x3dh_encrypted_chatrooms, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH basic chatrooms", group_chat_lime_x3dh_basic_chat_rooms, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH message", group_chat_lime_x3dh_send_encrypted_message, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH message with composing", group_chat_lime_x3dh_send_encrypted_message_with_composing, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH message with response", group_chat_lime_x3dh_send_encrypted_message_with_response, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH message with response and composing", group_chat_lime_x3dh_send_encrypted_message_with_response_and_composing, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH message to devices with and without keys on server", group_chat_lime_x3dh_encrypted_message_to_devices_with_and_without_keys, "LimeX3DH", "LeaksMemory"),
	TEST_ONE_TAG("LIME X3DH send encrypted file", group_chat_lime_x3dh_send_encrypted_file, "LimeX3DH"),
	TEST_ONE_TAG("LIME X3DH send encrypted file + text", group_chat_lime_x3dh_send_encrypted_file_plus_text, "LimeX3DH"),
	TEST_TWO_TAGS("LIME X3DH verify SAS before message", group_chat_lime_x3dh_verify_sas_before_message, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH reject SAS before message", group_chat_lime_x3dh_reject_sas_before_message, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH message before verify SAS", group_chat_lime_x3dh_message_before_verify_sas, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH message before reject SAS", group_chat_lime_x3dh_message_before_reject_sas, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH chatroom security level upgrade", group_chat_lime_x3dh_chatroom_security_level_upgrade, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH chatroom security level downgrade adding participant", group_chat_lime_x3dh_chatroom_security_level_downgrade_adding_participant, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH chatroom security level downgrade resetting zrtp", group_chat_lime_x3dh_chatroom_security_level_downgrade_resetting_zrtp, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH chatroom security alert", group_chat_lime_x3dh_chatroom_security_alert, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH call security alert", group_chat_lime_x3dh_call_security_alert, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH multiple successive messages", group_chat_lime_x3dh_send_multiple_successive_encrypted_messages, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH encrypted message to disabled LIME X3DH", group_chat_lime_x3dh_send_encrypted_message_to_disabled_lime_x3dh, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH plain message to enabled LIME X3DH", group_chat_lime_x3dh_send_plain_message_to_enabled_lime_x3dh, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH message to multidevice participants", group_chat_lime_x3dh_send_encrypted_message_to_multidevice_participants, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH messages while network unreachable", group_chat_lime_x3dh_message_while_network_unreachable, "LimeX3DH", "LeaksMemory"),
	TEST_TWO_TAGS("LIME X3DH update keys", group_chat_lime_x3dh_update_keys, "LimeX3DH", "LeaksMemory"),
	TEST_ONE_TAG("Group chat room subscription denied", group_chat_room_subscription_denied, "LeaksMemory"),
	TEST_NO_TAG("Group chat room notify participant devices name", group_chat_room_participant_devices_name),
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
