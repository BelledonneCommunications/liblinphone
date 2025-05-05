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

#include <ctype.h>

#include "bctoolbox/crypto.h"
#include <bctoolbox/defs.h>
#include <bctoolbox/vfs.h>

#include <belle-sip/object.h>

#include "liblinphone_tester.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-auth-info.h"
#include "linphone/api/c-chat-message.h"
#include "linphone/api/c-chat-room.h"
#include "linphone/api/c-content.h"
#include "linphone/api/c-recorder.h"
#include "linphone/chat.h"
#include "linphone/core.h"
#include "linphone/core_utils.h"
#include "linphone/wrapper_utils.h"
#include "shared_tester_functions.h"
#include "tester_utils.h"

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

void liblinphone_tester_chat_message_state_change(LinphoneChatMessage *msg,
                                                  LinphoneChatMessageState state,
                                                  BCTBX_UNUSED(void *ud)) {
	liblinphone_tester_chat_message_msg_state_changed(msg, state);
}

LinphoneChatMessage *
create_message_from_sintel_trailer_legacy(LinphoneChatRoom *chat_room, const char *filepath, char *filename) {
	FILE *file_to_send = NULL;
	LinphoneChatMessageCbs *cbs;
	LinphoneContent *content;
	LinphoneChatMessage *msg;
	char *send_filepath = bc_tester_res(filepath);
	size_t file_size;
	file_to_send = fopen(send_filepath, "rb");
	fseek(file_to_send, 0, SEEK_END);
	file_size = ftell(file_to_send);
	fseek(file_to_send, 0, SEEK_SET);

	content = linphone_core_create_content(linphone_chat_room_get_core(chat_room));
	belle_sip_object_set_name(BELLE_SIP_OBJECT(content), "sintel trailer content");
	linphone_content_set_type(content, "video");
	linphone_content_set_subtype(content, "mkv");
	linphone_content_set_size(content, file_size); /*total size to be transferred*/
	linphone_content_set_name(content, filename);
	linphone_content_set_user_data(content, file_to_send);

	msg = linphone_chat_room_create_file_transfer_message(chat_room, content);
	cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_file_transfer_send(cbs, tester_file_transfer_send);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);
	BC_ASSERT_PTR_NOT_NULL(linphone_content_get_user_data(content));

	linphone_content_unref(content);
	bctbx_free(send_filepath);
	return msg;
}

LinphoneChatMessage *_create_message_from_sintel_trailer(LinphoneChatRoom *chat_room,
                                                         bool_t use_alt_file_transfer_progress_indication_cb,
                                                         const char *filepath,
                                                         char *filename) {
	FILE *file_to_send = NULL;
	LinphoneChatMessageCbs *cbs;
	LinphoneContent *content;
	LinphoneChatMessage *msg;
	char *send_filepath = bc_tester_res(filepath);
	size_t file_size;
	file_to_send = fopen(send_filepath, "rb");
	fseek(file_to_send, 0, SEEK_END);
	file_size = ftell(file_to_send);
	fseek(file_to_send, 0, SEEK_SET);

	content = linphone_core_create_content(linphone_chat_room_get_core(chat_room));
	belle_sip_object_set_name(BELLE_SIP_OBJECT(content), "sintel trailer content");
	linphone_content_set_type(content, "video");
	linphone_content_set_subtype(content, "mkv");
	linphone_content_set_size(content, file_size); /*total size to be transferred*/
	linphone_content_set_name(content, filename);
	linphone_content_set_user_data(content, file_to_send);

	msg = linphone_chat_room_create_file_transfer_message(chat_room, content);
	cbs = linphone_factory_create_chat_message_cbs(linphone_factory_get());
	linphone_chat_message_cbs_set_file_transfer_send_chunk(cbs, tester_file_transfer_send_2);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	if (use_alt_file_transfer_progress_indication_cb) {
		linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication_2);
	} else {
		linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);
	}
	BC_ASSERT_PTR_NOT_NULL(linphone_content_get_user_data(content));
	linphone_chat_message_add_callbacks(msg, cbs);
	linphone_chat_message_cbs_unref(cbs);

	linphone_content_unref(content);
	bctbx_free(send_filepath);
	return msg;
}

LinphoneChatMessage *create_message_from_sintel_trailer(LinphoneChatRoom *chat_room) {
	char *filename = random_filename("sintel_trailer_opus_h264", "mkv");
	LinphoneChatMessage *msg =
	    _create_message_from_sintel_trailer(chat_room, FALSE, "sounds/sintel_trailer_opus_h264.mkv", filename);
	bctbx_free(filename);
	return msg;
}

LinphoneChatMessage *
create_file_transfer_message_from_file(LinphoneChatRoom *chat_room, const char *filepath, char *filename) {
	FILE *file_to_send = NULL;
	LinphoneChatMessageCbs *cbs;
	LinphoneContent *content;
	LinphoneChatMessage *msg;
	char *send_filepath = bc_tester_res(filepath);
	size_t file_size;
	file_to_send = fopen(send_filepath, "rb");
	fseek(file_to_send, 0, SEEK_END);
	file_size = ftell(file_to_send);
	fseek(file_to_send, 0, SEEK_SET);

	content = linphone_core_create_content(linphone_chat_room_get_core(chat_room));
	belle_sip_object_set_name(BELLE_SIP_OBJECT(content), "sintel trailer content");
	linphone_content_set_type(content, "video");
	linphone_content_set_subtype(content, "mkv");
	linphone_content_set_name(content, filename);
	linphone_content_set_file_path(content, send_filepath);
	linphone_content_set_size(content, file_size); /*total size to be transferred*/

	msg = linphone_chat_room_create_file_transfer_message(chat_room, content);
	cbs = linphone_factory_create_chat_message_cbs(linphone_factory_get());
	linphone_chat_message_cbs_set_file_transfer_send_chunk(cbs, tester_file_transfer_send_2);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);
	linphone_chat_message_add_callbacks(msg, cbs);
	linphone_chat_message_cbs_unref(cbs);

	linphone_content_unref(content);
	bctbx_free(send_filepath);
	return msg;
}

LinphoneChatMessage *create_file_transfer_message_from_sintel_trailer(LinphoneChatRoom *chat_room) {
	char *filename = random_filename("sintel_trailer_opus_h264", "mkv");
	LinphoneChatMessage *msg =
	    create_file_transfer_message_from_file(chat_room, "sounds/sintel_trailer_opus_h264.mkv", filename);
	bctbx_free(filename);
	return msg;
}

void chat_room_message_received_callback(LinphoneChatRoom *chat_room, BCTBX_UNUSED(LinphoneChatMessage *message)) {
	LinphoneChatRoomCbs *cbs = linphone_chat_room_get_current_callbacks(chat_room);
	LinphoneCoreManager *mgr = (LinphoneCoreManager *)linphone_chat_room_cbs_get_user_data(cbs);

	// Set network reachable false so that the mgr cannot send the 200 Ok
	linphone_core_set_network_reachable(mgr->lc, FALSE);
}

void text_message_base_with_text_and_forward(LinphoneCoreManager *marie,
                                             LinphoneCoreManager *pauline,
                                             const char *text,
                                             const char *content_type,
                                             bool_t forward_message,
                                             bool_t reply_message,
                                             bool_t allow_cpim_in_basic_chat_room_sender,
                                             bool_t allow_cpim_in_basic_chat_room_receiver,
                                             bool_t reaction_message,
                                             bool_t check_duplication) {
	if (allow_cpim_in_basic_chat_room_sender) {
		LinphoneCore *marieCore = marie->lc;
		LinphoneAccount *marieAccount = linphone_core_get_default_account(marieCore);
		const LinphoneAccountParams *marieAccountParams = linphone_account_get_params(marieAccount);
		bool_t cpim_enabled = linphone_account_params_cpim_in_basic_chat_room_enabled(marieAccountParams);
		BC_ASSERT_FALSE(cpim_enabled);
		LinphoneAccountParams *clonedMarieAccountParams = linphone_account_params_clone(marieAccountParams);
		linphone_account_params_enable_cpim_in_basic_chat_room(clonedMarieAccountParams, TRUE);
		linphone_account_set_params(marieAccount, clonedMarieAccountParams);
		linphone_account_params_unref(clonedMarieAccountParams);
	}
	if (allow_cpim_in_basic_chat_room_receiver) {
		LinphoneCore *paulineCore = pauline->lc;
		LinphoneAccount *paulineAccount = linphone_core_get_default_account(paulineCore);
		const LinphoneAccountParams *paulineAccountParams = linphone_account_get_params(paulineAccount);
		bool_t cpim_enabled = linphone_account_params_cpim_in_basic_chat_room_enabled(paulineAccountParams);
		BC_ASSERT_FALSE(cpim_enabled);
		LinphoneAccountParams *clonedPaulineAccountParams = linphone_account_params_clone(paulineAccountParams);
		linphone_account_params_enable_cpim_in_basic_chat_room(clonedPaulineAccountParams, TRUE);
		linphone_account_set_params(paulineAccount, clonedPaulineAccountParams);
		linphone_account_params_unref(clonedPaulineAccountParams);
	}

	LinphoneChatRoom *room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	BC_ASSERT_TRUE(linphone_chat_room_is_empty(room));

	if (check_duplication) {
		LinphoneChatRoom *marie_room = linphone_core_get_chat_room(marie->lc, pauline->identity);
		LinphoneChatRoomCbs *room_cbs = linphone_factory_create_chat_room_cbs(linphone_factory_get());
		linphone_chat_room_cbs_set_message_received(room_cbs, chat_room_message_received_callback);
		linphone_chat_room_cbs_set_user_data(room_cbs, marie);
		linphone_chat_room_add_callbacks(marie_room, room_cbs);
		linphone_chat_room_cbs_unref(room_cbs);
	}

	LinphoneChatMessage *msg = linphone_chat_room_create_message_from_utf8(room, text);
	linphone_chat_message_set_content_type(msg, content_type);
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDelivered, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));

	if (check_duplication) {
		linphone_core_set_network_reachable(marie->lc, TRUE);
		wait_for_until(pauline->lc, marie->lc, NULL, 0, 1000);

		BC_ASSERT_EQUAL(linphone_core_get_number_of_duplicated_messages(marie->lc), 1, int, "%d");
	}

	BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message);
	if (marie->stat.last_received_chat_message != NULL) {
		LinphoneContent *content =
		    (LinphoneContent *)(linphone_chat_message_get_contents(marie->stat.last_received_chat_message)->data);
		char *content_type_header = ms_strdup_printf("Content-Type: %s", content_type);
		belle_sip_header_content_type_t *belle_sip_content_type =
		    belle_sip_header_content_type_parse(content_type_header);
		BC_ASSERT_STRING_EQUAL(linphone_content_get_type(content),
		                       belle_sip_header_content_type_get_type(belle_sip_content_type));
		BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(content),
		                       belle_sip_header_content_type_get_subtype(belle_sip_content_type));
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marie->stat.last_received_chat_message), text);
		ms_free(content_type_header);
		LinphoneChatRoom *marieCr;

		const LinphoneAddress *msg_from =
		    linphone_chat_message_get_from_address(marie->stat.last_received_chat_message);
		/* We have special case for anonymous message, that of course won't come in the chatroom to pauline.*/
		if (strcasecmp(linphone_address_get_username(msg_from), "anonymous") == 0) {
			marieCr = linphone_chat_message_get_chat_room(marie->stat.last_received_chat_message);
		} else {
			marieCr = linphone_core_get_chat_room(marie->lc, pauline->identity);
		}

		if (linphone_factory_is_database_storage_available(linphone_factory_get())) {
			BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), 1, int, " %i");
			if (linphone_chat_room_get_history_size(marieCr) > 0) {
				bctbx_list_t *history = linphone_chat_room_get_history(marieCr, 1);
				LinphoneChatMessage *recv_msg = (LinphoneChatMessage *)(history->data);
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(recv_msg), text);

				if (forward_message) {
					LinphoneChatMessage *fmsg = linphone_chat_room_create_forward_message(marieCr, recv_msg);
					LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(fmsg);
					linphone_chat_message_cbs_set_msg_state_changed(cbs,
					                                                liblinphone_tester_chat_message_msg_state_changed);
					linphone_chat_message_send(fmsg);

					BC_ASSERT_TRUE(
					    wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageDelivered, 1));
					BC_ASSERT_TRUE(
					    wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageReceived, 1));
					BC_ASSERT_PTR_NOT_NULL(pauline->stat.last_received_chat_message);
					if (pauline->stat.last_received_chat_message != NULL) {
						LinphoneContent *content =
						    (LinphoneContent
						         *)(linphone_chat_message_get_contents(pauline->stat.last_received_chat_message)->data);
						char *content_type_header = ms_strdup_printf("Content-Type: %s", content_type);
						belle_sip_header_content_type_t *belle_sip_content_type =
						    belle_sip_header_content_type_parse(content_type_header);
						BC_ASSERT_STRING_EQUAL(linphone_content_get_type(content),
						                       belle_sip_header_content_type_get_type(belle_sip_content_type));
						BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(content),
						                       belle_sip_header_content_type_get_subtype(belle_sip_content_type));
						BC_ASSERT_STRING_EQUAL(
						    linphone_chat_message_get_utf8_text(pauline->stat.last_received_chat_message), text);
						ms_free(content_type_header);
						LinphoneChatRoom *paulineCr;

						const LinphoneAddress *msg_from =
						    linphone_chat_message_get_from_address(pauline->stat.last_received_chat_message);
						/* We have special case for anonymous message, that of course won't come in the chatroom to
						 * pauline.*/
						if (strcasecmp(linphone_address_get_username(msg_from), "anonymous") == 0) {
							paulineCr = linphone_chat_message_get_chat_room(pauline->stat.last_received_chat_message);
						} else {
							paulineCr = linphone_core_get_chat_room(pauline->lc, marie->identity);
						}
						BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 2, int, " %i");

						if (linphone_chat_room_get_history_size(paulineCr) > 1) {
							LinphoneChatMessage *recv_msg = linphone_chat_room_get_last_message_in_history(paulineCr);
							BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(recv_msg), text);
							if (allow_cpim_in_basic_chat_room_sender) {
								BC_ASSERT_TRUE(linphone_chat_message_is_forward(recv_msg));
								BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_forward_info(recv_msg), "Anonymous");
								// Because of CPIM
								BC_ASSERT_STRING_NOT_EQUAL(linphone_chat_message_get_message_id(recv_msg),
								                           linphone_chat_message_get_call_id(recv_msg));
							} else {
								// On a basic chat room we won't have this information unless CPIM has been enabled, see
								// linphone_account_params_cpim_in_basic_chat_room_enabled()
								BC_ASSERT_FALSE(linphone_chat_message_is_forward(recv_msg));
								// Because of no CPIM
								BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_message_id(recv_msg),
								                       linphone_chat_message_get_call_id(recv_msg));
							}
							linphone_chat_message_unref(recv_msg);
						}
					}
					linphone_chat_message_unref(fmsg);
				} else if (reply_message) {
					LinphoneChatMessage *rmsg = linphone_chat_room_create_reply_message(marieCr, recv_msg);
					BC_ASSERT_TRUE(linphone_chat_message_is_reply(rmsg));
					LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(rmsg);
					linphone_chat_message_cbs_set_msg_state_changed(cbs,
					                                                liblinphone_tester_chat_message_msg_state_changed);
					linphone_chat_message_add_utf8_text_content(rmsg, "<3");
					BC_ASSERT_TRUE(
					    linphone_address_weak_equal(linphone_chat_message_get_reply_message_sender_address(rmsg),
					                                linphone_chat_message_get_from_address(recv_msg)));
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_reply_message_id(rmsg),
					                       linphone_chat_message_get_message_id(recv_msg));
					const bctbx_list_t *contents = linphone_chat_message_get_contents(msg);
					BC_ASSERT_EQUAL((int)bctbx_list_size(contents), 1, int, "%d");

					linphone_chat_message_send(rmsg);

					BC_ASSERT_TRUE(
					    wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageDelivered, 1));
					BC_ASSERT_TRUE(
					    wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageReceived, 1));
					BC_ASSERT_PTR_NOT_NULL(pauline->stat.last_received_chat_message);
					if (pauline->stat.last_received_chat_message != NULL) {
						LinphoneChatRoom *paulineCr = linphone_core_get_chat_room(pauline->lc, marie->identity);
						BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 2, int, " %i");

						if (linphone_chat_room_get_history_size(paulineCr) > 1) {
							LinphoneChatMessage *recv_msg = linphone_chat_room_get_last_message_in_history(paulineCr);
							BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(recv_msg), "<3");

							if (allow_cpim_in_basic_chat_room_sender) {
								BC_ASSERT_TRUE(linphone_chat_message_is_reply(recv_msg));
								BC_ASSERT_TRUE(linphone_address_weak_equal(
								    linphone_chat_message_get_reply_message_sender_address(rmsg),
								    linphone_chat_message_get_from_address(msg)));
								LinphoneChatMessage *replied_message =
								    linphone_chat_message_get_reply_message(recv_msg);
								BC_ASSERT_PTR_NOT_NULL(replied_message);
								BC_ASSERT_PTR_EQUAL(replied_message, msg);
								if (replied_message) {
									linphone_chat_message_unref(replied_message);
								}
								// Because of CPIM
								BC_ASSERT_STRING_NOT_EQUAL(linphone_chat_message_get_message_id(recv_msg),
								                           linphone_chat_message_get_call_id(recv_msg));
							} else {
								// On a basic chat room we won't have this information unless CPIM has been enabled, see
								// linphone_account_params_cpim_in_basic_chat_room_enabled()
								BC_ASSERT_FALSE(linphone_chat_message_is_reply(recv_msg));
								BC_ASSERT_PTR_NULL(linphone_chat_message_get_reply_message(recv_msg));
								BC_ASSERT_FALSE(linphone_address_weak_equal(
								    linphone_chat_message_get_reply_message_sender_address(rmsg),
								    linphone_chat_message_get_from_address(recv_msg)));
								BC_ASSERT_TRUE(linphone_chat_message_get_reply_message_id(recv_msg) == NULL);
								// Because of no CPIM
								BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_message_id(recv_msg),
								                       linphone_chat_message_get_call_id(recv_msg));
							}

							contents = linphone_chat_message_get_contents(recv_msg);
							BC_ASSERT_EQUAL((int)bctbx_list_size(contents), 1, int, "%d");
							linphone_chat_message_unref(recv_msg);
						}
					}
					linphone_chat_message_unref(rmsg);
				} else if (reaction_message) {
					LinphoneChatMessageReaction *reactionMsg = linphone_chat_message_create_reaction(recv_msg, "👍");
					const LinphoneAddress *reactionAddr = linphone_chat_message_reaction_get_from_address(reactionMsg);
					BC_ASSERT_TRUE(
					    linphone_address_weak_equal(reactionAddr, linphone_chat_message_get_to_address(recv_msg)));

					LinphoneChatMessageCbs *recv_cbs = linphone_chat_message_get_callbacks(recv_msg);
					linphone_chat_message_cbs_set_new_message_reaction(
					    recv_cbs, liblinphone_tester_chat_message_reaction_received);

					linphone_chat_message_reaction_send(reactionMsg);
					BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
					                              &marie->stat.number_of_LinphoneReactionSentOrReceived, 1, 1000));
					linphone_chat_message_reaction_unref(reactionMsg);

					if (allow_cpim_in_basic_chat_room_sender) {
						BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc,
						                        &pauline->stat.number_of_LinphoneReactionSentOrReceived, 1));
						BC_ASSERT_FALSE(wait_for_until(pauline->lc, marie->lc,
						                               &pauline->stat.number_of_LinphoneMessageReceived, 1, 1000));

						bctbx_list_t *expected_reaction = bctbx_list_append(NULL, "👍");
						const LinphoneAddress *contact =
						    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(marie->lc));
						LinphoneAddress *clonedContact = linphone_address_clone(contact);
						linphone_address_clean(clonedContact); // Needed to remove GRUU
						bctbx_list_t *expected_reaction_from =
						    bctbx_list_append(NULL, linphone_address_as_string_uri_only(clonedContact));
						linphone_address_unref(clonedContact);
						check_reactions(recv_msg, 1, expected_reaction, expected_reaction_from);
						bctbx_list_free(expected_reaction);
						bctbx_list_free_with_data(expected_reaction_from, (bctbx_list_free_func)ms_free);
					} else {
						BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
						                              &pauline->stat.number_of_LinphoneMessageReceived, 1, 3000));
						BC_ASSERT_FALSE(wait_for_until(
						    pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneReactionSentOrReceived, 1, 3000));
					}
					BC_ASSERT_FALSE(wait_for_until(pauline->lc, marie->lc,
					                               &marie->stat.number_of_LinphoneMessageDeliveredToUser, 1, 1000));
					BC_ASSERT_FALSE(wait_for_until(pauline->lc, marie->lc,
					                               &marie->stat.number_of_LinphoneMessageDisplayed, 1, 1000));
				}

				bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_chat_message_unref);
			}
		}
	}

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_chat_room(marie->lc, pauline->identity));
	linphone_chat_message_unref(msg);
}

void text_message_base_with_text(LinphoneCoreManager *marie,
                                 LinphoneCoreManager *pauline,
                                 const char *text,
                                 const char *content_type) {
	text_message_base_with_text_and_forward(marie, pauline, text, content_type, FALSE, FALSE, FALSE, FALSE, FALSE,
	                                        FALSE);
}

void text_message_base(LinphoneCoreManager *marie, LinphoneCoreManager *pauline) {
	text_message_base_with_text(marie, pauline, "Bli bli bli \n blu", "text/plain");
}

/****************************** Tests starting below ******************************/

static void file_transfer_content(void) {
	LinphoneCoreManager *laure = linphone_core_manager_new("laure_tcp_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	char *send_filepath = bc_tester_res("sounds/sintel_trailer_opus_h264.mkv");

	LinphoneChatRoom *chat_room = linphone_core_get_chat_room(pauline->lc, laure->identity);

	LinphoneContent *file_transfer_message_content =
	    linphone_core_create_content(linphone_chat_room_get_core(chat_room));
	linphone_content_set_file_path(file_transfer_message_content, send_filepath);

	LinphoneChatMessage *message =
	    linphone_chat_room_create_file_transfer_message(chat_room, file_transfer_message_content);
	const bctbx_list_t *contents = linphone_chat_message_get_contents(message);
	BC_ASSERT_EQUAL((int)bctbx_list_size(contents), 1, int, "%d");

	LinphoneContent *content = (LinphoneContent *)bctbx_list_get_data(contents);
	BC_ASSERT_PTR_NOT_NULL(content);
	if (content) {
		BC_ASSERT_STRING_EQUAL(linphone_content_get_name(content), "sintel_trailer_opus_h264.mkv");
		BC_ASSERT_STRING_EQUAL(linphone_content_get_file_path(content), send_filepath);
		BC_ASSERT_STRING_EQUAL(linphone_content_get_type(content), "application");
		BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(content), "octet-stream");
	}

	linphone_chat_message_unref(message);
	linphone_content_unref(file_transfer_message_content);
	bctbx_free(send_filepath);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(pauline);
}

static void message_with_two_attachments(void) {
	LinphoneCoreManager *laure = linphone_core_manager_new("laure_tcp_rc");
	// Disable autodownload
	linphone_core_set_max_size_for_auto_download_incoming_files(laure->lc, -1);
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	linphone_core_set_file_transfer_server(pauline->lc, file_transfer_url);

	char *send_filepath1 = bc_tester_res("sounds/sintel_trailer_opus_h264.mkv");
	char *send_filepath2 = bc_tester_res("images/nowebcamVGA.jpg");

	LinphoneChatRoom *chat_room = linphone_core_get_chat_room(pauline->lc, laure->identity);
	linphone_chat_room_allow_multipart(chat_room);

	LinphoneChatMessage *message = linphone_chat_room_create_empty_message(chat_room);

	LinphoneContent *file_transfer_content1 = linphone_core_create_content(linphone_chat_room_get_core(chat_room));
	linphone_content_set_file_path(file_transfer_content1, send_filepath1);
	linphone_chat_message_add_file_content(message, file_transfer_content1);

	LinphoneContent *file_transfer_content2 = linphone_core_create_content(linphone_chat_room_get_core(chat_room));
	linphone_content_set_file_path(file_transfer_content2, send_filepath2);
	linphone_chat_message_add_file_content(message, file_transfer_content2);

	const bctbx_list_t *contents = linphone_chat_message_get_contents(message);
	size_t nb_of_contents = bctbx_list_size(contents);
	BC_ASSERT_EQUAL(nb_of_contents, 2, size_t, "%zu");

	LinphoneContent *content = (LinphoneContent *)bctbx_list_get_data(contents);
	BC_ASSERT_PTR_NOT_NULL(content);
	if (content) {
		BC_ASSERT_STRING_EQUAL(linphone_content_get_name(content), "sintel_trailer_opus_h264.mkv");
		BC_ASSERT_STRING_EQUAL(linphone_content_get_file_path(content), send_filepath1);
		BC_ASSERT_STRING_EQUAL(linphone_content_get_type(content), "application");
		BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(content), "octet-stream");
	}

	if (nb_of_contents > 1) {
		content = (LinphoneContent *)bctbx_list_get_data(bctbx_list_next(contents));
		if (content) {
			BC_ASSERT_STRING_EQUAL(linphone_content_get_name(content), "nowebcamVGA.jpg");
			BC_ASSERT_STRING_EQUAL(linphone_content_get_file_path(content), send_filepath2);
			BC_ASSERT_STRING_EQUAL(linphone_content_get_type(content), "application");
			BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(content), "octet-stream");
		}
	}
	LinphoneChatMessageCbs *send_cbs = linphone_chat_message_get_callbacks(message);
	linphone_chat_message_cbs_set_msg_state_changed(send_cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_cbs_set_file_transfer_recv(send_cbs, file_transfer_received);
	linphone_chat_message_cbs_set_file_transfer_progress_indication(send_cbs, file_transfer_progress_indication);
	linphone_chat_message_send(message);

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageFileTransferInProgress, 1, int, "%d");
	BC_ASSERT_TRUE(wait_for(pauline->lc, laure->lc, &pauline->stat.number_of_LinphoneMessageDelivered, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, laure->lc, &laure->stat.number_of_LinphoneMessageReceivedWithFile, 1));
	LinphoneChatMessage *laure_recv_msg = laure->stat.last_received_chat_message;
	BC_ASSERT_PTR_NOT_NULL(laure_recv_msg);
	if (laure_recv_msg != NULL) {
		LinphoneChatMessageCbs *recv_cbs = linphone_chat_message_get_callbacks(laure_recv_msg);
		linphone_chat_message_cbs_set_msg_state_changed(recv_cbs, liblinphone_tester_chat_message_msg_state_changed);
		linphone_chat_message_cbs_set_file_transfer_recv(recv_cbs, file_transfer_received);
		linphone_chat_message_cbs_set_file_transfer_progress_indication(recv_cbs, file_transfer_progress_indication);
		linphone_chat_message_cbs_set_file_transfer_terminated(
		    recv_cbs, liblinphone_tester_chat_message_file_transfer_terminated);

		const bctbx_list_t *recv_contents = linphone_chat_message_get_contents(laure_recv_msg);
		size_t nb_of_recv_contents = bctbx_list_size(recv_contents);
		BC_ASSERT_EQUAL(nb_of_recv_contents, nb_of_contents, size_t, "%zu");
		linphone_chat_message_download_contents(laure_recv_msg);
		BC_ASSERT_TRUE(wait_for_until(laure->lc, pauline->lc,
		                              &laure->stat.number_of_LinphoneRemainingNumberOfFileTransferChanged,
		                              (int)nb_of_contents, liblinphone_tester_sip_timeout));
		BC_ASSERT_EQUAL(linphone_core_get_remaining_download_file_count(laure->lc), (unsigned int)nb_of_contents,
		                unsigned int, "%u");
		BC_ASSERT_TRUE(wait_for_until(laure->lc, pauline->lc,
		                              &laure->stat.number_of_LinphoneMessageFileTransferInProgress, (int)nb_of_contents,
		                              liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_until(laure->lc, pauline->lc, &laure->stat.number_of_LinphoneMessageFileTransferDone,
		                              (int)nb_of_contents, liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_until(laure->lc, pauline->lc,
		                              &laure->stat.number_of_LinphoneMessageFileTransferTerminated, (int)nb_of_contents,
		                              liblinphone_tester_sip_timeout));
	}
	BC_ASSERT_TRUE(wait_for_until(laure->lc, pauline->lc,
	                              &laure->stat.number_of_LinphoneRemainingNumberOfFileTransferChanged,
	                              2 * (int)nb_of_contents, liblinphone_tester_sip_timeout));
	BC_ASSERT_EQUAL(linphone_core_get_remaining_download_file_count(laure->lc), 0, unsigned int, "%u");
	linphone_chat_message_unref(message);
	linphone_content_unref(file_transfer_content1);
	linphone_content_unref(file_transfer_content2);
	bc_free(send_filepath1);
	bc_free(send_filepath2);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(pauline);
}

static void create_two_basic_chat_room_with_same_remote(void) {
	LinphoneCoreManager *laure = linphone_core_manager_new("laure_tcp_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	LinphoneChatRoomParams *chat_room_params = linphone_core_create_default_chat_room_params(pauline->lc);
	linphone_chat_room_params_set_backend(chat_room_params, LinphoneChatRoomBackendBasic);
	linphone_chat_room_params_enable_encryption(chat_room_params, FALSE);
	linphone_chat_room_params_enable_group(chat_room_params, FALSE);
	linphone_chat_room_params_enable_rtt(chat_room_params, FALSE);
	bctbx_list_t *participants = bctbx_list_append(NULL, laure->identity);
	LinphoneChatRoom *chat_room =
	    linphone_core_create_chat_room_6(pauline->lc, chat_room_params, pauline->identity, participants);
	BC_ASSERT_PTR_NOT_NULL(chat_room);

	LinphoneChatRoom *chat_room2 =
	    linphone_core_create_chat_room_6(pauline->lc, chat_room_params, pauline->identity, participants);
	BC_ASSERT_PTR_NOT_NULL(chat_room2);
	BC_ASSERT_PTR_EQUAL(chat_room, chat_room2);

	bctbx_list_free(participants);
	linphone_chat_room_params_unref(chat_room_params);
	linphone_chat_room_unref(chat_room);
	linphone_chat_room_unref(chat_room2);
	linphone_core_manager_destroy(laure);
	linphone_core_manager_destroy(pauline);
}

static void text_message(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	text_message_base(marie, pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_forward_message(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	text_message_base_with_text_and_forward(marie, pauline, "Bli bli bli \n blu", "text/plain", TRUE, FALSE, FALSE,
	                                        FALSE, FALSE, FALSE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_forward_message_cpim_enabled_backward_compat(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	text_message_base_with_text_and_forward(marie, pauline, "Bla bla bla \n blu", "text/plain", TRUE, FALSE, TRUE,
	                                        FALSE, FALSE, FALSE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_forward_message_cpim_enabled(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	text_message_base_with_text_and_forward(marie, pauline, "Bla bla bla \n blu", "text/plain", TRUE, FALSE, TRUE, TRUE,
	                                        FALSE, FALSE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_forward_transfer_message_not_downloaded(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	linphone_core_set_file_transfer_server(pauline->lc, file_transfer_url);

	LinphoneChatRoom *room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	BC_ASSERT_TRUE(linphone_chat_room_is_empty(room));

	const char *send_filename = "sounds/sintel_trailer_opus_h264.mkv";
	char *filename = random_filename("sintel_trailer_opus_h264", "mkv");
	LinphoneChatMessage *msg = msg = create_file_transfer_message_from_file(room, send_filename, filename);
	bctbx_free(filename);
	linphone_chat_message_send(msg);

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageFileTransferInProgress, 1, int, "%d");
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDelivered, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceivedWithFile, 1));
	LinphoneChatMessage *marie_recv_msg = marie->stat.last_received_chat_message;
	BC_ASSERT_PTR_NOT_NULL(marie_recv_msg);
	if (marie_recv_msg != NULL) {
		const LinphoneContent *content =
		    (const LinphoneContent *)(linphone_chat_message_get_contents(marie_recv_msg)->data);
		BC_ASSERT_TRUE(linphone_content_is_file_transfer(content));
		// Do not download the file, transfer it as-is.

		LinphoneChatRoom *marieCr;
		const LinphoneAddress *msg_from = linphone_chat_message_get_from_address(marie_recv_msg);
		/* We have special case for anonymous message, that of course won't come in the chatroom to pauline.*/
		if (strcasecmp(linphone_address_get_username(msg_from), "anonymous") == 0) {
			marieCr = linphone_chat_message_get_chat_room(marie_recv_msg);
		} else {
			marieCr = linphone_core_get_chat_room(marie->lc, pauline->identity);
		}
		LinphoneChatMessage *fmsg = linphone_chat_room_create_forward_message(marieCr, marie_recv_msg);
		LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(fmsg);
		linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
		linphone_chat_message_send(fmsg);

		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageDelivered, 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageReceivedWithFile, 1));

		LinphoneChatMessage *pauline_recv_msg = pauline->stat.last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(pauline_recv_msg);
		if (pauline_recv_msg != NULL) {
			LinphoneContent *content = (LinphoneContent *)(linphone_chat_message_get_contents(pauline_recv_msg)->data);
			BC_ASSERT_TRUE(linphone_content_is_file_transfer(content));

			cbs = linphone_chat_message_get_callbacks(pauline_recv_msg);
			linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
			linphone_chat_message_cbs_set_file_transfer_recv(cbs, file_transfer_received);
			linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);
			char *receive_filepath = random_filepath("receive_file", "dump");
			remove(receive_filepath);
			linphone_chat_message_set_file_transfer_filepath(pauline_recv_msg, receive_filepath);
			bctbx_free(receive_filepath);
			linphone_chat_message_download_file(pauline_recv_msg);

			BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageFileTransferInProgress, 2, int, "%d");
			if (BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
			                                  &pauline->stat.number_of_LinphoneFileTransferDownloadSuccessful, 2,
			                                  55000))) {
				char *send_filepath = bc_tester_res(send_filename);
				compare_files(send_filepath, linphone_chat_message_get_file_transfer_filepath(pauline_recv_msg));
				remove(linphone_chat_message_get_file_transfer_filepath(pauline_recv_msg));
				bctbx_free(send_filepath);
			}
		}
		linphone_chat_message_unref(fmsg);
	}

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_chat_room(marie->lc, pauline->identity));
	linphone_chat_message_unref(msg);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_reply_message(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	text_message_base_with_text_and_forward(marie, pauline, "Bli bli bli \n blu", "text/plain", FALSE, TRUE, FALSE,
	                                        FALSE, FALSE, FALSE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_reply_message_cpim_enabled_backward_compat(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	text_message_base_with_text_and_forward(marie, pauline, "Bla bla bla \n blu", "text/plain", FALSE, TRUE, TRUE,
	                                        FALSE, FALSE, FALSE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_reply_message_cpim_enabled(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	text_message_base_with_text_and_forward(marie, pauline, "Bla bla bla \n blu", "text/plain", FALSE, TRUE, TRUE, TRUE,
	                                        FALSE, FALSE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_reaction_message(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	text_message_base_with_text_and_forward(marie, pauline, "Bli bli bli \n blu", "text/plain", FALSE, FALSE, FALSE,
	                                        FALSE, TRUE, FALSE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_reaction_message_cpim_enabled_backward_compat(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	text_message_base_with_text_and_forward(marie, pauline, "Bla bla bla \n blu", "text/plain", FALSE, FALSE, TRUE,
	                                        FALSE, TRUE, FALSE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_reaction_message_cpim_enabled(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	text_message_base_with_text_and_forward(marie, pauline, "Bla bla bla \n blu", "text/plain", FALSE, FALSE, TRUE,
	                                        TRUE, TRUE, FALSE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_message_with_utf8(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	text_message_base_with_text(marie, pauline, "Salut Fran\xc3\xa7ois", "text/plain;charset=UTF-8");

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_message_within_call_dialog(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "chat_use_call_dialogs", 1);

	if (BC_ASSERT_TRUE(call(marie, pauline))) {
		linphone_chat_room_send_message(linphone_core_get_chat_room(pauline->lc, marie->identity), "Bla bla bla bla");

		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
		// when using call dialogs, we will never receive delivered status
		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageDelivered, 0, int, "%d");

		// Send a second message, so that we can check that it is not erroneously filtered out because
		// it bears the same call-id.
		linphone_chat_room_send_message(linphone_core_get_chat_room(pauline->lc, marie->identity), "Bouhbouhbouh");

		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 2));

		end_call(marie, pauline);
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static LinphoneAuthInfo *text_message_with_credential_from_auth_cb_auth_info;
static void text_message_with_credential_from_auth_cb_auth_info_requested(LinphoneCore *lc,
                                                                          const char *realm,
                                                                          const char *username,
                                                                          BCTBX_UNUSED(const char *domain)) {
	ms_message("text_message_with_credential_from_auth_callback:Auth info requested  for user id [%s] at realm [%s]\n",
	           username, realm);
	linphone_core_add_auth_info(
	    lc, text_message_with_credential_from_auth_cb_auth_info); /*add stored authentication info to LinphoneCore*/
}

static void text_message_with_credential_from_auth_callback(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());

	/*to force cb to be called*/
	text_message_with_credential_from_auth_cb_auth_info =
	    linphone_auth_info_clone((LinphoneAuthInfo *)(linphone_core_get_auth_info_list(pauline->lc)->data));
	linphone_core_clear_all_auth_info(pauline->lc);
	linphone_core_cbs_set_auth_info_requested(cbs, text_message_with_credential_from_auth_cb_auth_info_requested);
	linphone_core_add_callbacks(pauline->lc, cbs);
	linphone_core_cbs_unref(cbs);

	text_message_base(marie, pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_auth_info_unref(text_message_with_credential_from_auth_cb_auth_info);
	text_message_with_credential_from_auth_cb_auth_info = NULL;
}

static void text_message_with_privacy(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneProxyConfig *config = linphone_core_get_default_proxy_config(pauline->lc);
	linphone_proxy_config_edit(config);
	linphone_proxy_config_set_privacy(config, LinphonePrivacyId);
	linphone_proxy_config_done(config);

	text_message_base(marie, pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_message_compatibility_mode(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneProxyConfig *proxy = linphone_core_get_default_proxy_config(marie->lc);
	LinphoneAddress *proxy_address = linphone_address_new(linphone_proxy_config_get_server_addr(proxy));
	char route[256];
	char *tmp;
	/*only keep tcp*/
	linphone_proxy_config_edit(proxy);
	LCSipTransports transport = {0, -1, 0, 0};
	linphone_address_clean(proxy_address);
	tmp = linphone_address_as_string_uri_only(proxy_address);
	linphone_proxy_config_set_server_addr(proxy, tmp);
	sprintf(route, "sip:%s", test_route);
	linphone_proxy_config_set_route(proxy, route);
	linphone_proxy_config_done(proxy);
	ms_free(tmp);
	linphone_address_unref(proxy_address);
	linphone_core_set_sip_transports(marie->lc, &transport);
	marie->stat.number_of_LinphoneRegistrationOk = 0;
	BC_ASSERT_TRUE(wait_for(marie->lc, marie->lc, &marie->stat.number_of_LinphoneRegistrationOk, 1));

	text_message_base(marie, pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_message_with_ack(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	text_message_base(marie, pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_message_with_send_error(void) {
	if (!linphone_factory_is_database_storage_available(linphone_factory_get())) {
		ms_warning("Test skipped, database storage is not available");
		return;
	}

	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));

	LinphoneChatRoom *chat_room = linphone_core_get_chat_room(marie->lc, pauline->identity);
	LinphoneChatMessage *msg = linphone_chat_room_create_message_from_utf8(chat_room, "Bli bli bli \n blu");
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneMessageSent, 0, int, "%d");

	/*simulate a network error*/
	sal_set_send_error(linphone_core_get_sal(marie->lc), -1);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg);
	char *message_id = ms_strdup(linphone_chat_message_get_message_id(msg));
	BC_ASSERT_STRING_NOT_EQUAL(message_id, "");

	/* check transient msg list: the msg should be in it, and should be the only one */
	BC_ASSERT_EQUAL(_linphone_chat_room_get_transient_message_count(chat_room), 1, int, "%d");
	BC_ASSERT_PTR_EQUAL(_linphone_chat_room_get_first_transient_message(chat_room), msg);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessagePendingDelivery, 1));
	/*BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneMessageInProgress,1, int, "%d");*/
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageReceived, 0, int, "%d");

	/* the msg should have been discarded from transient list after an error */
	BC_ASSERT_EQUAL(_linphone_chat_room_get_transient_message_count(chat_room), 1, int, "%d");

	// Even if error the message should be notified in sent callback
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneMessageSent, 1, int, "%d");

	sal_set_send_error(linphone_core_get_sal(marie->lc), 0);

	// resend the message
	linphone_chat_message_send(msg);
	const char *message_id_2 = linphone_chat_message_get_message_id(msg);
	BC_ASSERT_STRING_NOT_EQUAL(message_id_2, "");

	BC_ASSERT_STRING_NOT_EQUAL(message_id, message_id_2);
	ms_free(message_id);

	// if imdn received before 200 ok, chat message will change directly state to DeliveredToUser from Inprogress.
	// BC_ASSERT_TRUE(wait_for(pauline->lc,marie->lc,&marie->stat.number_of_LinphoneMessageDelivered,1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageDeliveredToUser, 1));
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageReceived, 1, int, "%d");

	// In case of resend the send callback should not be called again
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneMessageSent, 1, int, "%d");

	/*give a chance to register again to allow linphone_core_manager_destroy to properly unregister*/
	linphone_core_refresh_registers(marie->lc);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneRegistrationOk,
	                        marie->stat.number_of_LinphoneRegistrationOk + 1));

	linphone_chat_message_unref(msg);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}
void text_message_from_non_default_proxy_config_to_core_managers(LinphoneCoreManager *marie,
                                                                 LinphoneCoreManager *pauline) {
	const bctbx_list_t *proxyConfigs = linphone_core_get_proxy_config_list(marie->lc);
	BC_ASSERT_EQUAL((int)bctbx_list_size(proxyConfigs), 2, int, "%d");
	LinphoneProxyConfig *proxyConfig = NULL;
	const bctbx_list_t *prxCfgs = proxyConfigs;
	for (; prxCfgs != NULL; prxCfgs = prxCfgs->next) {
		LinphoneProxyConfig *prxCfg = (LinphoneProxyConfig *)prxCfgs->data;
		if (linphone_core_get_default_proxy_config(marie->lc) != prxCfg) {
			proxyConfig = prxCfg;
			break;
		}
	}
	BC_ASSERT_PTR_NOT_NULL(proxyConfig);
	BC_ASSERT_PTR_NOT_EQUAL(proxyConfig, linphone_core_get_default_proxy_config(marie->lc));
	BC_ASSERT_TRUE(linphone_proxy_config_get_state(proxyConfig) == LinphoneRegistrationOk);

	const LinphoneAddress *localAddr = linphone_proxy_config_get_identity_address(proxyConfig);
	const LinphoneAddress *remoteAddr =
	    linphone_proxy_config_get_identity_address(linphone_core_get_default_proxy_config(pauline->lc));
	LinphoneChatRoom *room = linphone_core_get_chat_room_2(marie->lc, remoteAddr, localAddr);

	LinphoneChatMessage *msg = linphone_chat_room_create_message_from_utf8(room, "Bli bli");
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg);
	linphone_chat_message_unref(msg);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneMessageReceived, 1));
	BC_ASSERT_PTR_NOT_NULL(pauline->stat.last_received_chat_message);
}

void text_message_from_non_default_proxy_config(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("marie_dual_proxy_2_rc", FALSE);
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 2));

	text_message_from_non_default_proxy_config_to_core_managers(marie, pauline);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void text_message_reply_from_non_default_proxy_config(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new_with_proxies_check("marie_dual_proxy_2_rc", FALSE);
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_rc");
	BC_ASSERT_TRUE(wait_for(marie->lc, NULL, &marie->stat.number_of_LinphoneRegistrationOk, 2));

	const bctbx_list_t *proxyConfigs = linphone_core_get_proxy_config_list(marie->lc);
	BC_ASSERT_EQUAL((int)bctbx_list_size(proxyConfigs), 2, int, "%d");
	LinphoneProxyConfig *proxyConfig = NULL;
	const bctbx_list_t *prxCfgs = proxyConfigs;
	for (; prxCfgs != NULL; prxCfgs = prxCfgs->next) {
		LinphoneProxyConfig *prxCfg = (LinphoneProxyConfig *)prxCfgs->data;
		if (linphone_core_get_default_proxy_config(marie->lc) != prxCfg) {
			proxyConfig = prxCfg;
			break;
		}
	}
	BC_ASSERT_PTR_NOT_NULL(proxyConfig);
	BC_ASSERT_PTR_NOT_EQUAL(proxyConfig, linphone_core_get_default_proxy_config(marie->lc));
	BC_ASSERT_TRUE(linphone_proxy_config_get_state(proxyConfig) == LinphoneRegistrationOk);

	const LinphoneAddress *marieLocalAddr = linphone_proxy_config_get_identity_address(proxyConfig);
	LinphoneChatRoom *room = linphone_core_get_chat_room(pauline->lc, marieLocalAddr);

	LinphoneChatMessage *msg = linphone_chat_room_create_message_from_utf8(room, "Bli bli");
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg);
	linphone_chat_message_unref(msg);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
	BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message);

	LinphoneChatRoom *remoteRoom = linphone_chat_message_get_chat_room(marie->stat.last_received_chat_message);
	msg = linphone_chat_room_create_message_from_utf8(remoteRoom, "Blu blu");
	cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg);
	linphone_chat_message_unref(msg);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneMessageReceived, 1));
	BC_ASSERT_PTR_NOT_NULL(pauline->stat.last_received_chat_message);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_message_in_call_chat_room_base(bool_t rtt_enabled_in_sender_but_denied,
                                                bool_t test_with_already_another_chat_room) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_dual_proxy_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCall *pauline_call, *marie_call;
	LinphoneChatRoom *pauline_chat_room, *marie_chat_room;

	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	LinphoneCallParams *pauline_params = linphone_core_create_call_params(pauline->lc, NULL);
	linphone_call_params_enable_realtime_text(marie_params, rtt_enabled_in_sender_but_denied);
	linphone_call_params_enable_realtime_text(pauline_params, FALSE);

	// Send a message from another proxy to create a chat room that is on different proxy
	if (test_with_already_another_chat_room)
		text_message_from_non_default_proxy_config_to_core_managers(marie, pauline);

	if (BC_ASSERT_TRUE(call_with_params(marie, pauline, marie_params, pauline_params))) {
		pauline_call = linphone_core_get_current_call(pauline->lc);
		marie_call = linphone_core_get_current_call(marie->lc);
		BC_ASSERT_FALSE(linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(pauline_call)));
		BC_ASSERT_FALSE(linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(marie_call)));
		BC_ASSERT_EQUAL(linphone_call_get_state(pauline_call), LinphoneCallStateStreamsRunning, int, "%d");
		BC_ASSERT_EQUAL(linphone_call_get_state(marie_call), LinphoneCallStateStreamsRunning, int, "%d");

		pauline_chat_room = linphone_call_get_chat_room(pauline_call);
		BC_ASSERT_PTR_NOT_NULL(pauline_chat_room);
		BC_ASSERT_EQUAL(linphone_chat_room_get_state(pauline_chat_room), LinphoneChatRoomStateCreated, int, "%d");
		marie_chat_room = linphone_call_get_chat_room(marie_call);
		BC_ASSERT_PTR_NOT_NULL(marie_chat_room);
		BC_ASSERT_EQUAL(linphone_chat_room_get_state(marie_chat_room), LinphoneChatRoomStateCreated, int, "%d");

		const LinphoneProxyConfig *marieProxyConfig = linphone_core_get_default_proxy_config(marie->lc);
		const LinphoneAddress *marieAddress = linphone_proxy_config_get_identity_address(marieProxyConfig);

		BC_ASSERT_TRUE(
		    linphone_address_weak_equal(linphone_chat_room_get_local_address(marie_chat_room), marieAddress));

		if (pauline_chat_room && marie_chat_room) {
			const char *pauline_text_message = "Hello marie!";
			LinphoneChatMessage *pauline_message =
			    linphone_chat_room_create_message_from_utf8(pauline_chat_room, pauline_text_message);
			BC_ASSERT_PTR_NOT_NULL(pauline_message);
			linphone_chat_message_send(pauline_message);

			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageSent, 1));
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
			{
				LinphoneChatMessage *msg = marie->stat.last_received_chat_message;
				BC_ASSERT_PTR_NOT_NULL(msg);
				if (msg) {
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), pauline_text_message);
				}
			}

			const char *marie_text_message = "Hi pauline :)";
			LinphoneChatMessage *marie_message =
			    linphone_chat_room_create_message_from_utf8(marie_chat_room, marie_text_message);
			BC_ASSERT_PTR_NOT_NULL(marie_message);
			linphone_chat_message_send(marie_message);

			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageSent, 1));
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageReceived,
			                        (test_with_already_another_chat_room ? 2 : 1)));
			{
				LinphoneChatMessage *msg = pauline->stat.last_received_chat_message;
				BC_ASSERT_PTR_NOT_NULL(msg);
				if (msg) {
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), marie_text_message);
				}
			}

			linphone_chat_message_unref(pauline_message);
			linphone_chat_message_unref(marie_message);
		}
		end_call(marie, pauline);
	}
	linphone_call_params_unref(marie_params);
	linphone_call_params_unref(pauline_params);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_message_in_call_chat_room(void) {
	text_message_in_call_chat_room_base(FALSE, FALSE);
}

static void text_message_in_call_chat_room_from_denied_text_offer(void) {
	text_message_in_call_chat_room_base(TRUE, FALSE);
}

static void text_message_in_call_chat_room_when_room_exists(void) {
	text_message_in_call_chat_room_base(FALSE, TRUE);
}

static void text_message_in_call_chat_room_from_denied_text_offer_when_room_exists(void) {
	text_message_in_call_chat_room_base(TRUE, TRUE);
}

static void text_message_duplication(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	text_message_base_with_text_and_forward(marie, pauline, "Bli bli bli \n blu", "text/plain", FALSE, FALSE, TRUE,
	                                        TRUE, FALSE, TRUE);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void text_message_auto_resent_after_failure_base(bool with_file_transfer) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	linphone_core_set_file_transfer_server(marie->lc, file_transfer_url);
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));

	/* Enable auto download on Pauline's Core */
	linphone_core_set_max_size_for_auto_download_incoming_files(pauline->lc, 0);

	LinphoneChatRoom *chat_room = linphone_core_get_chat_room(marie->lc, pauline->identity);
	linphone_chat_room_allow_multipart(chat_room);

	LinphoneChatMessage *msg = linphone_chat_room_create_empty_message(chat_room);

	LinphoneContent *text_content = linphone_core_create_content(linphone_chat_room_get_core(chat_room));
	const char *text = "Testing automatic message resend";
	linphone_content_set_type(text_content, "text");
	linphone_content_set_subtype(text_content, "plain");
	linphone_content_set_utf8_text(text_content, text);
	linphone_chat_message_add_content(msg, text_content);
	linphone_content_unref(text_content);

	char *send_filepath = bc_tester_res("images/nowebcamVGA.jpg");
	if (with_file_transfer) {
		LinphoneContent *file_transfer_content = linphone_core_create_content(linphone_chat_room_get_core(chat_room));
		linphone_content_set_file_path(file_transfer_content, send_filepath);
		linphone_chat_message_add_file_content(msg, file_transfer_content);
		linphone_content_unref(file_transfer_content);
	}

	const bctbx_list_t *contents = linphone_chat_message_get_contents(msg);
	size_t nb_of_contents = bctbx_list_size(contents);
	BC_ASSERT_EQUAL(nb_of_contents, (with_file_transfer) ? 2 : 1, size_t, "%zu");

	LinphoneContent *content = (LinphoneContent *)bctbx_list_get_data(contents);
	BC_ASSERT_PTR_NOT_NULL(content);
	if (content) {
		BC_ASSERT_STRING_EQUAL(linphone_content_get_utf8_text(content), text);
		BC_ASSERT_STRING_EQUAL(linphone_content_get_type(content), "text");
		BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(content), "plain");
	}

	if (nb_of_contents > 1) {
		content = (LinphoneContent *)bctbx_list_get_data(bctbx_list_next(contents));
		if (content) {
			BC_ASSERT_STRING_EQUAL(linphone_content_get_name(content), "nowebcamVGA.jpg");
			BC_ASSERT_STRING_EQUAL(linphone_content_get_file_path(content), send_filepath);
			BC_ASSERT_STRING_EQUAL(linphone_content_get_type(content), "application");
			BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(content), "octet-stream");
		}
	}
	bctbx_free(send_filepath);

	LinphoneChatMessageCbs *send_cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_msg_state_changed(send_cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_cbs_set_file_transfer_recv(send_cbs, file_transfer_received);
	linphone_chat_message_cbs_set_file_transfer_progress_indication(send_cbs, file_transfer_progress_indication);

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneMessageSent, 0, int, "%d");

	/*simulate a network error*/
	sal_set_send_error(linphone_core_get_sal(marie->lc), -1);
	linphone_chat_message_send(msg);
	char *message_id = ms_strdup(linphone_chat_message_get_message_id(msg));
	if (with_file_transfer) {
		// The message ID is not computed because files cannot be uploaded to file transfer server
		BC_ASSERT_STRING_EQUAL(message_id, "");
	} else {
		BC_ASSERT_STRING_NOT_EQUAL(message_id, "");
	}
	ms_free(message_id);

	/* check transient msg list: the msg should be in it, and should be the only one */
	BC_ASSERT_EQUAL(_linphone_chat_room_get_transient_message_count(chat_room), 1, int, "%d");
	BC_ASSERT_PTR_EQUAL(_linphone_chat_room_get_first_transient_message(chat_room), msg);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessagePendingDelivery, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageNotDelivered, 0));
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageReceived, 0, int, "%d");

	/* the msg should have been added to the transient list after an error */
	BC_ASSERT_EQUAL(_linphone_chat_room_get_transient_message_count(chat_room), 1, int, "%d");

	// The upload to the file transfer server fails therefore the message sent callback is not called.
	// On the other hand, if the message contains only text, then it is sent but the channel errors out
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneMessageSent, (with_file_transfer) ? 0 : 1, int, "%d");

	sal_set_send_error(linphone_core_get_sal(marie->lc), 0);

	ms_message("%s toggles its network", linphone_core_get_identity(marie->lc));
	stats initial_marie_stat = marie->stat;
	linphone_core_set_network_reachable(marie->lc, FALSE);
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_NetworkReachableFalse,
	                              initial_marie_stat.number_of_NetworkReachableFalse + 1, 5000));
	linphone_core_set_network_reachable(marie->lc, TRUE);
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_NetworkReachableTrue,
	                              initial_marie_stat.number_of_NetworkReachableTrue + 1, 5000));

	/* wait for marie to receive pauline's msg */
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageSent, 1, 5000));

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageReceived, 1, 5000));
	LinphoneChatMessage *pauline_msg = pauline->stat.last_received_chat_message;
	BC_ASSERT_PTR_NOT_NULL(pauline_msg);
	LinphoneChatRoom *pauline_cr = linphone_core_get_chat_room(pauline->lc, marie->identity);
	BC_ASSERT_PTR_NOT_NULL(pauline_cr);
	pauline_msg = linphone_chat_room_get_last_message_in_history(pauline_cr);
	BC_ASSERT_PTR_NOT_NULL(pauline_msg);
	if (pauline_msg) {
		LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(pauline_msg);
		linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
		linphone_chat_message_cbs_set_file_transfer_recv(cbs, file_transfer_received);
		linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);
	}

	if (with_file_transfer) {
		BC_ASSERT_TRUE(
		    wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageReceivedWithFile, 1, 5000));
		/* wait for a long time in case the DNS SRV resolution takes times - it should be immediate though */
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
		                              &pauline->stat.number_of_LinphoneFileTransferDownloadSuccessful, 1, 55000));
		BC_ASSERT_TRUE(
		    wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageFileTransferDone, 1, 5000));
	}

	if (pauline_msg) {
		const bctbx_list_t *contents = linphone_chat_message_get_contents(pauline_msg);
		BC_ASSERT_PTR_NOT_NULL(contents);
		BC_ASSERT_EQUAL(bctbx_list_size(contents), nb_of_contents, size_t, "%zu");
		content = (LinphoneContent *)bctbx_list_get_data(contents);
		BC_ASSERT_STRING_EQUAL(linphone_content_get_type(content), "text");
		BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(content), "plain");
		BC_ASSERT_STRING_EQUAL(linphone_content_get_utf8_text(content), text);

		if (nb_of_contents > 1) {
			content = (LinphoneContent *)bctbx_list_get_data(bctbx_list_next(contents));
			if (content) {
				BC_ASSERT_STRING_EQUAL(linphone_content_get_name(content), "nowebcamVGA.jpg");
				BC_ASSERT_STRING_EQUAL(linphone_content_get_type(content), "application");
				BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(content), "octet-stream");
			}
		}

		LinphoneChatMessageState state = linphone_chat_message_get_state(pauline_msg);
		BC_ASSERT_EQUAL(state, LinphoneChatMessageStateDelivered, int, "%d");
	}

	BC_ASSERT_TRUE(
	    wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageDeliveredToUser, 1, 5000));

	linphone_chat_room_mark_as_read(pauline_cr); /* This sends the display notification */
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageDisplayed, 1, 5000));
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDisplayed, 1, 5000));

	if (pauline_msg) {
		linphone_chat_message_unref(pauline_msg);
	}
	linphone_chat_message_unref(msg);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void text_message_auto_resent_after_failure(void) {
	text_message_auto_resent_after_failure_base(FALSE);
}

static void transfer_message_auto_resent_after_failure(void) {
	text_message_auto_resent_after_failure_base(TRUE);
}

void transfer_message_base4(LinphoneCoreManager *marie,
                            LinphoneCoreManager *pauline,
                            bool_t upload_error,
                            bool_t download_error,
                            bool_t use_file_body_handler_in_upload,
                            bool_t use_file_body_handler_in_download,
                            bool_t download_from_history,
                            int auto_download,
                            bool_t two_files,
                            bool_t legacy,
                            const char *url,
                            bool_t expect_auth_failure_up,
                            bool_t expect_auth_failure_down,
                            const char *file1_path,
                            char *expected_filename) {
	if (!linphone_factory_is_database_storage_available(linphone_factory_get())) {
		ms_warning("Test skipped, database storage is not available");
		return;
	}

	char *send_filepath = bc_tester_res(file1_path);
	char *send_filepath2 = bc_tester_res("sounds/ahbahouaismaisbon.wav");

	LinphoneChatRoom *chat_room;
	LinphoneChatMessage *msg;
	int file_transfer_size;
	bctbx_list_t *msg_list = NULL;

	/* Globally configure an http file transfer server. */
	linphone_core_set_file_transfer_server(pauline->lc, url);

	/* create a chatroom on pauline's side */
	chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);

	if (two_files) {
		linphone_chat_room_allow_multipart(chat_room);
		linphone_chat_room_allow_cpim(chat_room);
	}

	/* create a file transfer msg */
	if (use_file_body_handler_in_upload) {
		msg = create_file_transfer_message_from_file(chat_room, file1_path, expected_filename);
	} else {
		if (legacy) {
			msg = create_message_from_sintel_trailer_legacy(chat_room, file1_path, expected_filename);
		} else {
			msg = _create_message_from_sintel_trailer(chat_room, FALSE, file1_path, expected_filename);
		}
	}
	const bctbx_list_t *contents = linphone_chat_message_get_contents(msg);
	BC_ASSERT_PTR_NOT_NULL(contents);
	BC_ASSERT_EQUAL(1, (int)bctbx_list_size(contents), int, "%d");
	LinphoneContent *content = (LinphoneContent *)bctbx_list_get_data(contents);
	BC_ASSERT_PTR_NOT_NULL(content);
	file_transfer_size = (int)linphone_content_get_file_size(content);
	BC_ASSERT_NOT_EQUAL(0, file_transfer_size, int, "%d");
	BC_ASSERT_PTR_NULL(linphone_content_get_related_chat_message_id(content));

	if (two_files) {
		FILE *file_to_send = NULL;
		LinphoneContent *content;
		size_t file_size;
		file_to_send = fopen(send_filepath2, "rb");
		fseek(file_to_send, 0, SEEK_END);
		file_size = ftell(file_to_send);
		fseek(file_to_send, 0, SEEK_SET);

		content = linphone_core_create_content(linphone_chat_room_get_core(chat_room));
		belle_sip_object_set_name(BELLE_SIP_OBJECT(content), "ahbahouaismaisbon content");
		linphone_content_set_type(content, "audio");
		linphone_content_set_subtype(content, "wav");
		linphone_content_set_size(content, file_size); /*total size to be transferred*/
		linphone_content_set_name(content, "ahbahouaismaisbon.wav");
		linphone_content_set_user_data(content, file_to_send);
		BC_ASSERT_PTR_NULL(linphone_content_get_related_chat_message_id(content));

		linphone_chat_message_add_file_content(msg, content);
		BC_ASSERT_PTR_NOT_NULL(linphone_content_get_user_data(content));
		const bctbx_list_t *contents = linphone_chat_message_get_contents(msg);
		BC_ASSERT_EQUAL((int)bctbx_list_size(contents), 2, int, "%d");

		linphone_content_unref(content);
	}

	linphone_chat_message_send(msg);

	if (expect_auth_failure_up) {
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageFileTransferError, 1));
		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageDelivered, 0, int, "%d");
		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageFileTransferError, 1, int, "%d");
		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageFileTransferDone, 0, int, "%d");
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneFileTransferDownloadSuccessful, 0, int, "%d");
		goto end;
	}

	if (upload_error) {
		int chat_room_size = 0;
		bctbx_list_t *history;

		/*wait for file to be 25% uploaded and simulate a network error*/
		BC_ASSERT_TRUE(
		    wait_for_until(pauline->lc, marie->lc, &pauline->stat.progress_of_LinphoneFileTransfer, 25, 60000));
		/* Check that the message is already in the chat room history during file upload */
		chat_room_size = linphone_chat_room_get_history_size(chat_room);
		BC_ASSERT_EQUAL(chat_room_size, 1, int, "%d");
		if (chat_room_size == 1) {
			history = linphone_chat_room_get_history(chat_room, 0);
			LinphoneChatMessage *sent_msg = (LinphoneChatMessage *)bctbx_list_get_data(history);
			BC_ASSERT_EQUAL((int)linphone_chat_message_get_state(sent_msg),
			                (int)LinphoneChatMessageStateFileTransferInProgress, int, "%d");
			bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_chat_message_unref);
		}
		sal_set_send_error(linphone_core_get_sal(pauline->lc), -1);

		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessagePendingDelivery, 1));

		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessagePendingDelivery, 1, int, "%d");
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneFileTransferDownloadSuccessful, 0, int, "%d");

		sal_set_send_error(linphone_core_get_sal(pauline->lc), 0);

		linphone_core_refresh_registers(
		    pauline->lc); /*to make sure registration is back in registered and so it can be later unregistered*/
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneRegistrationOk,
		                        pauline->stat.number_of_LinphoneRegistrationOk + 1));

		// The message is automatically resent as the first attempt failed
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageNotDelivered, 1));
		/* Check that the message is in the chat room history even if the file upload failed */
		chat_room_size = linphone_chat_room_get_history_size(chat_room);
		BC_ASSERT_EQUAL(chat_room_size, 1, int, "%d");
		if (chat_room_size == 1) {
			history = linphone_chat_room_get_history(chat_room, 0);
			LinphoneChatMessage *sent_msg = (LinphoneChatMessage *)bctbx_list_get_data(history);
			BC_ASSERT_EQUAL((int)linphone_chat_message_get_state(sent_msg), (int)LinphoneChatMessageStateNotDelivered,
			                int, "%d");
			bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_chat_message_unref);
		}
	} else {
		BC_ASSERT_TRUE(
		    wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceivedWithFile, 1, 60000));
		if (two_files) {
			BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
			                              &pauline->stat.number_of_LinphoneMessageFileTransferDone, 2, 1000));
		}

		if (marie->stat.last_received_chat_message) {
			LinphoneChatRoom *marie_room = linphone_core_get_chat_room(marie->lc, pauline->identity);

			// We shoudln't get displayed IMDN until file has been downloaded & chat message has been markes as read
			if (linphone_factory_is_imdn_available(linphone_factory_get())) {
				BC_ASSERT_FALSE(
				    wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDisplayed, 1, 5000));
			}

			if (auto_download == -1 || (auto_download > 0 && auto_download < file_transfer_size)) {
				linphone_chat_room_mark_as_read(marie_room);
				if (linphone_factory_is_imdn_available(linphone_factory_get())) {
					// Chat message has been markes as read but not downloaded yet, so no IMDN should be received yet
					BC_ASSERT_FALSE(wait_for_until(pauline->lc, marie->lc,
					                               &pauline->stat.number_of_LinphoneMessageDisplayed, 1, 3000));
				}

				LinphoneChatMessage *recv_msg;
				if (download_from_history) {
					msg_list = linphone_chat_room_get_history(marie_room, 1);
					BC_ASSERT_PTR_NOT_NULL(msg_list);
					if (!msg_list) goto end;
					recv_msg = (LinphoneChatMessage *)msg_list->data;
				} else {
					recv_msg = marie->stat.last_received_chat_message;
				}
				BC_ASSERT_PTR_NOT_NULL(linphone_chat_message_get_external_body_url(recv_msg));

				const bctbx_list_t *contents = linphone_chat_message_get_contents(recv_msg);
				size_t nb_of_contents = bctbx_list_size(contents);
				BC_ASSERT_EQUAL(nb_of_contents, two_files ? 2 : 1, size_t, "%zu");

				LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(recv_msg);
				linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
				linphone_chat_message_cbs_set_file_transfer_recv(cbs, file_transfer_received);
				linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);
				linphone_chat_message_cbs_set_file_transfer_terminated(
				    cbs, liblinphone_tester_chat_message_file_transfer_terminated);
				if (use_file_body_handler_in_download) {
					char *receive_filepath = random_filepath("receive_file", "dump");
					linphone_chat_message_set_file_transfer_filepath(recv_msg, receive_filepath);
					bctbx_free(receive_filepath);
				}
				linphone_chat_message_download_file(recv_msg);
				BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc,
				                              &marie->stat.number_of_LinphoneRemainingNumberOfFileTransferChanged, 1,
				                              liblinphone_tester_sip_timeout));
				BC_ASSERT_EQUAL(linphone_core_get_remaining_download_file_count(marie->lc), 1, unsigned int, "%u");

				if (expect_auth_failure_down) {
					BC_ASSERT_TRUE(
					    wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneMessageFileTransferError, 1));
					BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneFileTransferDownloadSuccessful, 0, int, "%d");
					goto end;
				}

				BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneMessageFileTransferInProgress, 1, int, "%d");

				if (download_error) {
					/* Would like to wait for file to be 50% downloaded - no longer possible because the file is short
					 and downloaded in one iterate() cycle. As a workaround, we trigger the receive error immediately */
					// BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.progress_of_LinphoneFileTransfer,
					// 50));
					/* simulate network error */
					belle_http_provider_set_recv_error(linphone_core_get_http_provider(marie->lc), -1);
					BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc,
					                              &marie->stat.number_of_LinphoneMessageFileTransferError, 1, 10000));
					belle_http_provider_set_recv_error(linphone_core_get_http_provider(marie->lc), 1);
					if (linphone_factory_is_imdn_available(linphone_factory_get())) {
						BC_ASSERT_FALSE(wait_for_until(pauline->lc, marie->lc,
						                               &pauline->stat.number_of_LinphoneMessageDisplayed, 1, 3000));
					}
					BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc,
					                              &marie->stat.number_of_LinphoneRemainingNumberOfFileTransferChanged,
					                              2, liblinphone_tester_sip_timeout));
					BC_ASSERT_EQUAL(linphone_core_get_remaining_download_file_count(marie->lc), 0, unsigned int, "%u");
				} else {
					/* wait for a long time in case the DNS SRV resolution takes times - it should be immediate though
					 */
					if (BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
					                                  &marie->stat.number_of_LinphoneFileTransferDownloadSuccessful, 1,
					                                  55000))) {
						compare_files(send_filepath, linphone_chat_message_get_file_transfer_filepath(recv_msg));
						remove(linphone_chat_message_get_file_transfer_filepath(recv_msg));
					}

					BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc,
					                              &marie->stat.number_of_LinphoneRemainingNumberOfFileTransferChanged,
					                              2, liblinphone_tester_sip_timeout));
					BC_ASSERT_EQUAL(linphone_core_get_remaining_download_file_count(marie->lc), 0, unsigned int, "%u");
					BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc,
					                              &marie->stat.number_of_LinphoneMessageFileTransferTerminated, 1,
					                              liblinphone_tester_sip_timeout));

					if (two_files) {
						linphone_chat_message_download_file(recv_msg);
						BC_ASSERT_TRUE(wait_for_until(
						    marie->lc, pauline->lc, &marie->stat.number_of_LinphoneRemainingNumberOfFileTransferChanged,
						    3, liblinphone_tester_sip_timeout));
						BC_ASSERT_EQUAL(linphone_core_get_remaining_download_file_count(marie->lc), 1, unsigned int,
						                "%u");

						BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneMessageFileTransferInProgress, 2, int, "%d");

						BC_ASSERT_TRUE(wait_for_until(
						    marie->lc, pauline->lc, &marie->stat.number_of_LinphoneRemainingNumberOfFileTransferChanged,
						    4, liblinphone_tester_sip_timeout));
						BC_ASSERT_EQUAL(linphone_core_get_remaining_download_file_count(marie->lc), 0, unsigned int,
						                "%u");
						BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc,
						                              &marie->stat.number_of_LinphoneMessageFileTransferTerminated, 2,
						                              liblinphone_tester_sip_timeout));

						/* wait for a long time in case the DNS SRV resolution takes times - it should be immediate
						 * though */
						if (BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
						                                  &marie->stat.number_of_LinphoneFileTransferDownloadSuccessful,
						                                  2, 55000))) {
							compare_files(send_filepath2, linphone_chat_message_get_file_transfer_filepath(recv_msg));
							remove(linphone_chat_message_get_file_transfer_filepath(recv_msg));
						}
					}

					if (linphone_factory_is_imdn_available(linphone_factory_get())) {
						BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
						                              &pauline->stat.number_of_LinphoneMessageDisplayed, 1, 5000));
					}
				}
			} else {
				contents = linphone_chat_message_get_contents(msg);
				BC_ASSERT_PTR_NOT_NULL(contents);
				BC_ASSERT_EQUAL(1, (int)bctbx_list_size(contents), int, "%d");
				content = (LinphoneContent *)bctbx_list_get_data(contents);
				BC_ASSERT_PTR_NOT_NULL(content);
				BC_ASSERT_STRING_EQUAL(linphone_content_get_name(content), expected_filename);
				compare_files(send_filepath, linphone_content_get_file_path(content));
				BC_ASSERT_STRING_NOT_EQUAL(linphone_content_get_subtype(content), "vnd.gsma.rcs-ft-http+xml");
				BC_ASSERT_STRING_EQUAL(linphone_content_get_related_chat_message_id(content),
				                       linphone_chat_message_get_message_id(msg));

				if (linphone_factory_is_imdn_available(linphone_factory_get())) {
					BC_ASSERT_FALSE(wait_for_until(pauline->lc, marie->lc,
					                               &pauline->stat.number_of_LinphoneMessageDisplayed, 1, 3000));
				}
				linphone_chat_room_mark_as_read(marie_room);
				if (linphone_factory_is_imdn_available(linphone_factory_get())) {
					BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
					                              &pauline->stat.number_of_LinphoneMessageDisplayed, 1, 5000));
				}
			}
		}
		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageInProgress, 1, int, "%d");
		if (two_files) BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageFileTransferInProgress, 2, int, "%d");
		else BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageFileTransferInProgress, 1, int, "%d");
		if (linphone_im_notif_policy_get_recv_imdn_displayed(linphone_core_get_im_notif_policy(pauline->lc)) &&
		    linphone_im_notif_policy_get_send_imdn_delivered(linphone_core_get_im_notif_policy(marie->lc))) {
			// In case imdn arrives before 200ok, state Delivered is never be notified
			BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageDeliveredToUser, 1, int, "%d");
		} else {
			BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageDelivered, 1, int, "%d");
		}
	}
end:
	linphone_chat_message_unref(msg);
	bctbx_list_free_with_data(msg_list, (bctbx_list_free_func)linphone_chat_message_unref);
	bctbx_free(send_filepath);
	bctbx_free(send_filepath2);
	bctbx_free(expected_filename);
}

void transfer_message_base3(LinphoneCoreManager *marie,
                            LinphoneCoreManager *pauline,
                            bool_t upload_error,
                            bool_t download_error,
                            bool_t use_file_body_handler_in_upload,
                            bool_t use_file_body_handler_in_download,
                            bool_t download_from_history,
                            int auto_download,
                            bool_t two_files,
                            bool_t legacy,
                            const char *url,
                            bool_t expect_auth_failure_up,
                            bool_t expect_auth_failure_down) {
	transfer_message_base4(marie, pauline, upload_error, download_error, use_file_body_handler_in_upload,
	                       use_file_body_handler_in_download, download_from_history, auto_download, two_files, legacy,
	                       url, expect_auth_failure_up, expect_auth_failure_down, "sounds/special-&_characters.wav",
	                       random_filename("special-&_characters", "wav"));
}

// Add tls information for given user into the linphone core
// cert and keys are path to the file, set them as buffer as it is the most likely method to be used
static void add_tls_client_certificate(
    LinphoneCore *lc, const char *username, const char *realm, const char *cert, const char *key) {
	// We shall already have an auth info for this username/realm, add the tls cert in it
	LinphoneAuthInfo *auth_info = linphone_auth_info_clone(linphone_core_find_auth_info(lc, realm, username, realm));
	// otherwise create it
	if (auth_info == NULL) {
		auth_info = linphone_auth_info_new(username, NULL, NULL, NULL, realm, realm);
	}
	if (cert != NULL) {
		char *cert_path = bc_tester_res(cert);
		char *cert_buffer = NULL;
		liblinphone_tester_load_text_file_in_buffer(cert_path, &cert_buffer);
		linphone_auth_info_set_tls_cert(auth_info, cert_buffer);
		bctbx_free(cert_path);
		bctbx_free(cert_buffer);
	}
	if (key != NULL) {
		char *key_path = bc_tester_res(key);
		char *key_buffer = NULL;
		liblinphone_tester_load_text_file_in_buffer(key_path, &key_buffer);
		linphone_auth_info_set_tls_key(auth_info, key_buffer);
		bctbx_free(key_path);
		bctbx_free(key_buffer);
	}
	linphone_core_add_auth_info(lc, auth_info);
	linphone_auth_info_unref(auth_info);
}

static void transfer_message_tls_client_auth(void) {
	if (transport_supported(LinphoneTransportTls)) {
		LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
		LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

		// set a TLS client certificate
		// Note: this certificates allow to authenticate on sip.example.org but are for user_1 and user_2 not marie_XXX
		// or pauline_XXX This is not a problem as the file transfer server won't check the individual access but only
		// certificate validity but they must be indexed internally with the correct username and domain
		LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie->lc));
		LinphoneAddress *paulineAddr = linphone_address_new(linphone_core_get_identity(pauline->lc));
		add_tls_client_certificate(marie->lc, linphone_address_get_username(marieAddr),
		                           linphone_address_get_domain(marieAddr), "certificates/client/user1_cert.pem",
		                           "certificates/client/user1_key.pem");
		add_tls_client_certificate(pauline->lc, linphone_address_get_username(paulineAddr),
		                           linphone_address_get_domain(paulineAddr), "certificates/client/user2_cert.pem",
		                           "certificates/client/user2_key.pem");
		linphone_address_unref(marieAddr);
		linphone_address_unref(paulineAddr);

		// enable imdn (otherwise transfer_message_base3 is unhappy)
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "deliver_imdn", 1);
		linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "deliver_imdn", 1);
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));

		transfer_message_base3(marie, pauline, FALSE, FALSE, FALSE, FALSE, FALSE, -1, FALSE, FALSE,
		                       file_transfer_url_tls_client_auth, FALSE, FALSE);

		// Give some time for IMDN's 200 OK to be received so it doesn't leak
		wait_for_until(pauline->lc, marie->lc, NULL, 0, 1000);
		linphone_core_manager_destroy(pauline);
		linphone_core_manager_destroy(marie);
	}
}

static void
transfer_message_digest_auth_arg(const char *server_url, bool_t auth_info_failure_up, bool_t auth_info_failure_down) {
	if (transport_supported(LinphoneTransportTls)) {
		LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
		LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

		// enable imdn (otherwise transfer_message_base3 is unhappy)
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "deliver_imdn", 1);
		linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "deliver_imdn", 1);
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));

		// either up or down are set to fail, not both as up would stop the transfer anyway
		LinphoneAuthInfo *good_auth_info = NULL;
		LinphoneAuthInfo *wrong_auth_info = NULL;
		if (auth_info_failure_up) {
			// set wrong credentials for pauline (message sender)
			good_auth_info = linphone_auth_info_clone(linphone_core_find_auth_info(
			    pauline->lc, NULL, linphone_address_get_username(pauline->identity), NULL));
			wrong_auth_info = linphone_auth_info_clone(good_auth_info);
			linphone_auth_info_set_password(wrong_auth_info, "passecretdutout");
			linphone_core_clear_all_auth_info(pauline->lc);
			linphone_core_add_auth_info(pauline->lc, wrong_auth_info);
		}

		if (auth_info_failure_down) {
			// set wrong credentials for marie (message receiver)
			good_auth_info = linphone_auth_info_clone(
			    linphone_core_find_auth_info(marie->lc, NULL, linphone_address_get_username(marie->identity), NULL));
			wrong_auth_info = linphone_auth_info_clone(good_auth_info);
			linphone_auth_info_set_password(wrong_auth_info, "passecretdutout");
			linphone_core_clear_all_auth_info(marie->lc);
			linphone_core_add_auth_info(marie->lc, wrong_auth_info);
		}

		transfer_message_base3(marie, pauline, FALSE, FALSE, FALSE, FALSE, FALSE, -1, FALSE, FALSE, server_url,
		                       auth_info_failure_up, auth_info_failure_down);

		if (auth_info_failure_up) {
			// to make sure unregister will work
			linphone_core_clear_all_auth_info(pauline->lc);
			linphone_core_add_auth_info(pauline->lc, good_auth_info);
			linphone_auth_info_unref(good_auth_info);
			linphone_auth_info_unref(wrong_auth_info);
		}

		if (auth_info_failure_down) {
			// to make sure unregister will work
			linphone_core_clear_all_auth_info(marie->lc);
			linphone_core_add_auth_info(marie->lc, good_auth_info);
			linphone_auth_info_unref(good_auth_info);
			linphone_auth_info_unref(wrong_auth_info);
		}

		// Give some time for IMDN's 200 OK to be received so it doesn't leak
		wait_for_until(pauline->lc, marie->lc, NULL, 0, 1000);
		linphone_core_manager_destroy(pauline);
		linphone_core_manager_destroy(marie);
	}
}

static void transfer_message_digest_auth(void) {
	transfer_message_digest_auth_arg(file_transfer_url_digest_auth, FALSE, FALSE);
}

static void transfer_message_digest_auth_fail_up(void) {
	transfer_message_digest_auth_arg(file_transfer_url_digest_auth, TRUE, FALSE);
}

static void transfer_message_digest_auth_fail_down(void) {
	transfer_message_digest_auth_arg(file_transfer_url_digest_auth, FALSE, TRUE);
}

static void transfer_message_digest_auth_any_domain(void) {
	transfer_message_digest_auth_arg(file_transfer_url_digest_auth_any_domain, FALSE, FALSE);
}

static void transfer_message_digest_auth_fail_any_domain_up(void) {
	transfer_message_digest_auth_arg(file_transfer_url_digest_auth_any_domain, TRUE, FALSE);
}

static void transfer_message_digest_auth_fail_any_domain_down(void) {
	transfer_message_digest_auth_arg(file_transfer_url_digest_auth_any_domain, FALSE, TRUE);
}

static void transfer_message_small_files_pass(void) {
	if (transport_supported(LinphoneTransportTls)) {
		LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
		LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

		// enable imdn (otherwise transfer_message_base3 is unhappy)
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "deliver_imdn", 1);
		linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "deliver_imdn", 1);
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));

		transfer_message_base4(marie, pauline, FALSE, FALSE, TRUE, FALSE, FALSE, -1, FALSE, FALSE,
		                       file_transfer_url_small_files, FALSE, FALSE, "sounds/ahbahouaismaisbon.wav",
		                       random_filename("ahbahouaismaisbon", "wav")); // small file transfer, it should work

		// Give some time for IMDN's 200 OK to be received so it doesn't leak
		wait_for_until(pauline->lc, marie->lc, NULL, 0, 1000);
		linphone_core_manager_destroy(pauline);
		linphone_core_manager_destroy(marie);
	}
}

static void transfer_message_small_files_fail(void) {
	if (transport_supported(LinphoneTransportTls)) {
		LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
		LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

		// enable imdn (otherwise transfer_message_base3 is unhappy)
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "deliver_imdn", 1);
		linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "deliver_imdn", 1);
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));

		transfer_message_base4(
		    marie, pauline, FALSE, FALSE, TRUE, FALSE, FALSE, -1, FALSE, FALSE, file_transfer_url_small_files, TRUE,
		    FALSE, "sounds/sintel_trailer_opus_h264.mkv",
		    random_filename("sintel_trailer_opus_h264", "mkv")); // large file transfer, the upload should fail

		// Give some time for IMDN's 200 OK to be received so it doesn't leak
		wait_for_until(pauline->lc, marie->lc, NULL, 0, 1000);
		linphone_core_manager_destroy(pauline);
		linphone_core_manager_destroy(marie);
	}
}

void transfer_message_base2(LinphoneCoreManager *marie,
                            LinphoneCoreManager *pauline,
                            bool_t upload_error,
                            bool_t download_error,
                            bool_t use_file_body_handler_in_upload,
                            bool_t use_file_body_handler_in_download,
                            bool_t download_from_history,
                            int auto_download,
                            bool_t two_files,
                            bool_t legacy) {
	transfer_message_base3(marie, pauline, upload_error, download_error, use_file_body_handler_in_upload,
	                       use_file_body_handler_in_download, download_from_history, auto_download, two_files, legacy,
	                       file_transfer_url, FALSE, FALSE);
}

void transfer_message_base(bool_t upload_error,
                           bool_t download_error,
                           bool_t use_file_body_handler_in_upload,
                           bool_t use_file_body_handler_in_download,
                           bool_t download_from_history,
                           bool_t enable_imdn,
                           int auto_download,
                           bool_t two_files,
                           bool_t legacy) {
	if (transport_supported(LinphoneTransportTls)) {
		LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
		LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

		if (enable_imdn) {
			linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "deliver_imdn", 1);
			linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "deliver_imdn", 1);
			linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
			linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
		}
		linphone_core_set_max_size_for_auto_download_incoming_files(marie->lc, auto_download);

		transfer_message_base2(marie, pauline, upload_error, download_error, use_file_body_handler_in_upload,
		                       use_file_body_handler_in_download, download_from_history, auto_download, two_files,
		                       legacy);
		// Give some time for IMDN's 200 OK to be received so it doesn't leak
		wait_for_until(pauline->lc, marie->lc, NULL, 0, 1000);
		linphone_core_manager_destroy(pauline);
		linphone_core_manager_destroy(marie);
	}
}

static void transfer_message(void) {
	transfer_message_base(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, -1, FALSE, FALSE);
}

static void transfer_message_2(void) {
	transfer_message_base(FALSE, FALSE, TRUE, FALSE, FALSE, TRUE, -1, FALSE, FALSE);
}

static void transfer_message_3(void) {
	transfer_message_base(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, -1, FALSE, FALSE);
}

static void transfer_message_4(void) {
	transfer_message_base(FALSE, FALSE, TRUE, TRUE, FALSE, TRUE, -1, FALSE, FALSE);
}

static void message_with_voice_recording_base(bool_t create_message_from_recorder,
                                              bool_t auto_download_only_voice_recordings) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	linphone_core_set_file_transfer_server(pauline->lc, file_transfer_url);

	LinphoneChatRoom *room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	BC_ASSERT_TRUE(linphone_chat_room_is_empty(room));

	// Force auto download
	if (auto_download_only_voice_recordings) {
		linphone_core_set_auto_download_voice_recordings_enabled(marie->lc, TRUE);
	} else {
		linphone_core_set_auto_download_voice_recordings_enabled(marie->lc, FALSE);
		linphone_core_set_max_size_for_auto_download_incoming_files(marie->lc, 0);
	}

	LinphoneRecorderParams *params = linphone_core_create_recorder_params(pauline->lc);
	linphone_recorder_params_set_file_format(params, LinphoneMediaFileFormatWav);
	LinphoneRecorder *recorder = linphone_core_create_recorder(pauline->lc, params);
	linphone_recorder_params_unref(params);

	char *filename = bctbx_strdup_printf("%s/voice_record.wav", bc_tester_get_writable_dir_prefix());
	linphone_recorder_open(recorder, filename);
	wait_for_until(pauline->lc, NULL, NULL, 0, 5000);
	linphone_recorder_close(recorder);
	int duration = linphone_recorder_get_duration(recorder);

	LinphoneChatMessage *msg;
	if (create_message_from_recorder) {
		msg = linphone_chat_room_create_voice_recording_message(room, recorder);
	} else {
		msg = linphone_chat_room_create_empty_message(room);
		LinphoneContent *content = linphone_recorder_create_content(recorder);
		linphone_chat_message_add_content(msg, content);
		linphone_content_unref(content);
	}

	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDelivered, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
	LinphoneChatMessage *marie_msg = marie->stat.last_received_chat_message;
	BC_ASSERT_PTR_NOT_NULL(marie_msg);

	if (marie_msg != NULL) {
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageDelivered, 1));
		LinphoneContent *content = (LinphoneContent *)(linphone_chat_message_get_contents(marie_msg)->data);
		BC_ASSERT_EQUAL(linphone_content_get_file_duration(content), duration, int, "%d");
		BC_ASSERT_STRING_EQUAL(linphone_content_get_type(content), "audio");
		BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(content), "wav");
		BC_ASSERT_TRUE(linphone_content_is_voice_recording(content));
	}

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_chat_room(marie->lc, pauline->identity));
	linphone_chat_message_unref(msg);

	// Give some time for IMDN's 200 OK to be received so it doesn't leak
	bctbx_free(filename);
	linphone_recorder_unref(recorder);
	wait_for_until(pauline->lc, marie->lc, NULL, 0, 1000);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void message_with_voice_recording(void) {
	message_with_voice_recording_base(TRUE, FALSE);
}

static void message_with_voice_recording_2(void) {
	message_with_voice_recording_base(FALSE, FALSE);
}

static void message_with_voice_recording_3(void) {
	message_with_voice_recording_base(FALSE, TRUE);
}

static void transfer_message_legacy(void) {
	transfer_message_base(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, -1, FALSE, TRUE);
}

static void transfer_message_2_files(void) {
	transfer_message_base(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, -1, TRUE, FALSE);
}

static void transfer_message_auto_download(void) {
	transfer_message_base(FALSE, FALSE, TRUE, TRUE, FALSE, TRUE, 0, FALSE, FALSE);
}

static void transfer_message_auto_download_2(void) {
	transfer_message_base(FALSE, FALSE, TRUE, TRUE, FALSE, TRUE, 100000000, FALSE, FALSE);
}

static void transfer_message_auto_download_3(void) {
	transfer_message_base(FALSE, FALSE, TRUE, TRUE, FALSE, TRUE, 1, FALSE, FALSE);
}

static void transfer_message_auto_download_failure(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	char *filename = random_filename("sintel_trailer_opus_h264", "mkv");
	const char *filepath = "sounds/sintel_trailer_opus_h264.mkv";
	bctbx_list_t *participants = NULL;
	LinphoneChatRoom *chat_room;
	LinphoneConferenceParams *params;
	LinphoneChatParams *chat_params;
	LinphoneChatMessage *recv_msg, *sent_msg;

	linphone_core_set_file_transfer_server(marie->lc, file_transfer_url);
	linphone_core_set_max_size_for_auto_download_incoming_files(pauline->lc, 0);

	linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "deliver_imdn", 1);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "deliver_imdn", 1);
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));

	participants = bctbx_list_append(participants, pauline->identity);

	/* create a chatroom on Marie's side */
	params = linphone_core_create_conference_params_2(marie->lc, NULL);
	linphone_conference_params_enable_chat(params, TRUE);
	chat_params = linphone_conference_params_get_chat_params(params);
	linphone_conference_params_enable_group(params, FALSE);
	linphone_chat_params_set_backend(chat_params, LinphoneChatRoomBackendBasic);
	chat_room = linphone_core_create_chat_room_7(marie->lc, params, participants);
	bctbx_list_free(participants);
	linphone_conference_params_unref(params);
	BC_ASSERT_PTR_NOT_NULL(chat_room);

	/* create a file transfer msg */
	sent_msg = create_file_transfer_message_from_file(chat_room, filepath, filename);
	ms_free(filename);
	LinphoneChatMessageCbs *cbs = linphone_factory_create_chat_message_cbs(linphone_factory_get());
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_add_callbacks(sent_msg, cbs);
	linphone_chat_message_send(sent_msg);
	linphone_chat_message_cbs_unref(cbs);

	belle_http_provider_set_recv_error(linphone_core_get_http_provider(pauline->lc), -1);
	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneMessageReceived, 1, 10000));
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneFileTransferDownloadSuccessful, 0, int, "%i");
	recv_msg = pauline->stat.last_received_chat_message;
	if (BC_ASSERT_PTR_NOT_NULL(recv_msg)) {
	}
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageDelivered, 0, int, "%i");
	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc,
	                              &pauline->stat.number_of_LinphoneMessageFileTransferInProgress, 1, 10000));
	// as file transfer is going to fail, the message shall return to Delivered state.
	BC_ASSERT_TRUE(wait_for_until(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneMessageDelivered, 1, 10000));
	BC_ASSERT_TRUE(
	    wait_for_until(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneMessageDeliveredToUser, 1, 10000));
	BC_ASSERT_EQUAL(linphone_chat_message_get_state(recv_msg), LinphoneChatMessageStateDelivered, int, "%i");
	linphone_chat_room_unref(chat_room);
	linphone_chat_message_unref(sent_msg);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void transfer_message_auto_download_existing_file(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "deliver_imdn", 1);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "deliver_imdn", 1);
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
	linphone_core_set_max_size_for_auto_download_incoming_files(marie->lc, 0);

	LinphoneChatRoom *chat_room;
	LinphoneChatMessage *msg;
	char *filename = random_filename("sintel_trailer_opus_h264", "mkv");
	const char *filepath = "sounds/sintel_trailer_opus_h264.mkv";
	char *first_received_filename = NULL;

	/* Globally configure an http file transfer server. */
	linphone_core_set_file_transfer_server(pauline->lc, file_transfer_url);

	/* create a chatroom on pauline's side */
	chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);

	/* create a file transfer msg */
	msg = create_file_transfer_message_from_file(chat_room, filepath, filename);
	linphone_chat_message_send(msg);

	BC_ASSERT_TRUE(
	    wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceivedWithFile, 1, 60000));
	BC_ASSERT_PTR_NULL(first_received_filename);

	LinphoneChatMessage *marie_msg = marie->stat.last_received_chat_message;
	BC_ASSERT_PTR_NOT_NULL(marie_msg);
	if (marie_msg) {
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageDelivered, 1));
		const bctbx_list_t *contents = linphone_chat_message_get_contents(marie_msg);
		BC_ASSERT_PTR_NOT_NULL(contents);
		BC_ASSERT_EQUAL(1, (int)bctbx_list_size(contents), int, "%d");
		LinphoneContent *content = (LinphoneContent *)bctbx_list_get_data(contents);
		BC_ASSERT_PTR_NOT_NULL(content);
		BC_ASSERT_TRUE(linphone_content_is_file(content));
		first_received_filename = bctbx_strdup(linphone_content_get_file_path(content));
	}

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageInProgress, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageFileTransferInProgress, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageFileTransferDone, 1));
	BC_ASSERT_PTR_NOT_NULL(first_received_filename);

	linphone_chat_message_unref(msg);
	msg = create_file_transfer_message_from_file(chat_room, filepath, filename);
	linphone_chat_message_send(msg);

	BC_ASSERT_TRUE(
	    wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceivedWithFile, 2, 60000));
	marie_msg = marie->stat.last_received_chat_message;
	BC_ASSERT_PTR_NOT_NULL(marie_msg);
	if (marie_msg) {
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageDelivered, 2));
		const bctbx_list_t *contents = linphone_chat_message_get_contents(marie_msg);
		BC_ASSERT_PTR_NOT_NULL(contents);
		BC_ASSERT_EQUAL(1, (int)bctbx_list_size(contents), int, "%d");
		LinphoneContent *content = (LinphoneContent *)bctbx_list_get_data(contents);
		BC_ASSERT_PTR_NOT_NULL(content);
		BC_ASSERT_TRUE(linphone_content_is_file(content));
		BC_ASSERT_STRING_NOT_EQUAL(first_received_filename, linphone_content_get_file_path(content));
	}

	linphone_chat_message_unref(msg);

	// Give some time for IMDN's 200 OK to be received so it doesn't leak
	wait_for_until(pauline->lc, marie->lc, NULL, 0, 1000);
	bctbx_free(first_received_filename);
	bctbx_free(filename);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void transfer_message_auto_download_two_files_same_name_same_time(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "deliver_imdn", 1);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "deliver_imdn", 1);
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
	linphone_core_set_max_size_for_auto_download_incoming_files(marie->lc, 0);

	if (!linphone_factory_is_database_storage_available(linphone_factory_get())) {
		ms_warning("Test skipped, database storage is not available");
		return;
	}

	LinphoneChatRoom *pauline_chat_room;
	LinphoneChatRoom *marie_chat_room;
	LinphoneChatMessage *msg;
	LinphoneChatMessage *msg2;
	char *filename = random_filename("sintel_trailer_opus_h264", "mkv");
	const char *filepath = "sounds/sintel_trailer_opus_h264.mkv";

	/* Globally configure an http file transfer server. */
	linphone_core_set_file_transfer_server(pauline->lc, file_transfer_url);

	/* create a chatroom on pauline's side */
	pauline_chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);

	/* marie goes offline */
	linphone_core_set_network_reachable_internal(marie->lc, FALSE);

	/* create a file transfer msg */
	msg = create_file_transfer_message_from_file(pauline_chat_room, filepath, filename);
	linphone_chat_message_send(msg);

	/* create a second file transfer msg */
	msg2 = create_file_transfer_message_from_file(pauline_chat_room, filepath, filename);
	linphone_chat_message_send(msg2);

	/* wait for both messages to be sent */
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageSent, 2, 60000));

	/* marie goes back online */
	linphone_core_set_network_reachable_internal(marie->lc, TRUE);
	BC_ASSERT_TRUE(
	    wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceivedWithFile, 2, 60000));

	linphone_chat_message_unref(msg);
	linphone_chat_message_unref(msg2);

	marie_chat_room = linphone_core_get_chat_room(marie->lc, pauline->identity);
	BC_ASSERT_PTR_NOT_NULL(marie_chat_room);
	if (marie_chat_room) {
		bctbx_list_t *history = linphone_chat_room_get_history(marie_chat_room, 0);
		BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(history), 2, int, "%d");
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageDelivered,
		                              (int)bctbx_list_size(history), 5000));

		bctbx_list_t *it = history;

		msg = (LinphoneChatMessage *)bctbx_list_get_data(it);
		const bctbx_list_t *contents = linphone_chat_message_get_contents(msg);
		BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(contents), 1, int, "%d");
		LinphoneContent *content = (LinphoneContent *)bctbx_list_get_data(contents);
		BC_ASSERT_TRUE(linphone_content_is_file(content));
		const char *filename = linphone_content_get_name(content);
		BC_ASSERT_STRING_EQUAL(filename, filename);
		const char *file_path = linphone_content_get_file_path(content);

		it = bctbx_list_next(it);
		msg = (LinphoneChatMessage *)bctbx_list_get_data(it);
		contents = linphone_chat_message_get_contents(msg);
		BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(contents), 1, int, "%d");
		content = (LinphoneContent *)bctbx_list_get_data(contents);
		BC_ASSERT_TRUE(linphone_content_is_file(content));
		filename = linphone_content_get_name(content);
		BC_ASSERT_STRING_EQUAL(filename, filename);
		const char *file_path_2 = linphone_content_get_file_path(content);

		BC_ASSERT_STRING_NOT_EQUAL(file_path, file_path_2);

		bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_chat_message_unref);
	}

	// Give some time for IMDN's 200 OK to be received so it doesn't leak
	wait_for_until(pauline->lc, marie->lc, NULL, 0, 1000);
	bctbx_free(filename);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void transfer_message_from_history(void) {
	transfer_message_base(FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, -1, FALSE, FALSE);
}

static void transfer_message_with_upload_io_error(void) {
	transfer_message_base(TRUE, FALSE, FALSE, FALSE, FALSE, TRUE, -1, FALSE, FALSE);
}

static void transfer_message_with_download_io_error(void) {
	transfer_message_base(FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, -1, FALSE, FALSE);
}

static void transfer_message_upload_cancelled(void) {
	if (transport_supported(LinphoneTransportTls)) {
		LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
		LinphoneChatRoom *chat_room;
		LinphoneChatMessage *msg;
		LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

		/* Globally configure an http file transfer server. */
		linphone_core_set_file_transfer_server(pauline->lc, file_transfer_url);

		/* create a chatroom on pauline's side */
		chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);

		msg = create_message_from_sintel_trailer(chat_room);
		linphone_chat_message_send(msg);

		/*wait for file to be 25% uploaded and cancel the transfer */
		BC_ASSERT_TRUE(
		    wait_for_until(pauline->lc, marie->lc, &pauline->stat.progress_of_LinphoneFileTransfer, 25, 60000));
		linphone_chat_message_cancel_file_transfer(msg);

		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageNotDelivered, 1));

		BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageNotDelivered, 1, int, "%d");
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneFileTransferDownloadSuccessful, 0, int, "%d");

		// Resend message
		linphone_chat_message_send(msg);
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
		                              &pauline->stat.number_of_LinphoneFileTransferDownloadSuccessful, 1, 10000));
		BC_ASSERT_TRUE(
		    wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceivedWithFile, 1, 10000));

		// When C pointer is unreffed first, callbacks will be removed,
		// potentially during file upload causing issue in FileTransferChatMessageModifier::onSendBody
		// while the CPP shared ptr is still held by the chat room...
		linphone_chat_message_unref(msg);
		linphone_core_manager_destroy(pauline);
		linphone_core_manager_destroy(marie);
	}
}

static void transfer_message_upload_finished_during_stop(void) {
	if (transport_supported(LinphoneTransportTls)) {
		LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
		LinphoneChatRoom *chat_room;
		LinphoneChatMessage *msg;
		LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

		/* Globally configure an http file transfer server. */
		linphone_core_set_file_transfer_server(pauline->lc, file_transfer_url);

		/* create a chatroom on pauline's side */
		chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);

		msg = create_message_from_sintel_trailer(chat_room);
		linphone_chat_message_send(msg);

		/*wait for file to be 25% uploaded and cancel the transfer */
		BC_ASSERT_TRUE(
		    wait_for_until(pauline->lc, marie->lc, &pauline->stat.progress_of_LinphoneFileTransfer, 25, 30000));

		linphone_core_stop_async(pauline->lc);

		BC_ASSERT_TRUE(
		    wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1, 60000));
		// When C pointer is unreffed first, callbacks will be removed,
		// potentially during file upload causing issue in FileTransferChatMessageModifier::onSendBody
		// while the CPP shared ptr is still held by the chat room...
		linphone_chat_message_unref(msg);
		linphone_core_manager_destroy(pauline);
		linphone_core_manager_destroy(marie);
	}
}

static void transfer_message_download_cancelled(void) {
	LinphoneChatRoom *chat_room;
	LinphoneChatMessage *msg;
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	/* Globally configure an http file transfer server. */
	linphone_core_set_file_transfer_server(pauline->lc, file_transfer_url);

	/* create a chatroom on pauline's side */
	chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	msg = create_message_from_sintel_trailer(chat_room);
	linphone_chat_message_send(msg);

	/* wait for marie to receive pauline's msg */
	BC_ASSERT_TRUE(
	    wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceivedWithFile, 1, 60000));

	LinphoneChatMessage *marie_msg = marie->stat.last_received_chat_message;
	if (marie_msg) { /* get last msg and use it to download file */
		LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(marie_msg);
		linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);
		// State changed callback is already set by linphone_chat_message_start_file_download
		linphone_chat_message_cbs_set_msg_state_changed(cbs, NULL);
		linphone_chat_message_start_file_download(marie_msg, liblinphone_tester_chat_message_state_change, marie->lc);
		/* wait for file to be 2% downloaded */
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.progress_of_LinphoneFileTransfer, 2));
		/* and cancel the transfer */
		linphone_chat_message_cancel_file_transfer(marie_msg);
	}

	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageInProgress, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageFileTransferInProgress, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageDelivered, 1, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneFileTransferDownloadSuccessful, 0, int, "%d");
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneMessageFileTransferError, 1, int, "%d");

	linphone_chat_message_unref(msg);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void transfer_message_auto_download_aborted(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	char *dl_path = linphone_core_get_download_path(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(dl_path);

	char *path = bctbx_strdup_printf("%s/sintel_trailer_opus_h264.mkv", dl_path);
	remove(path);

	/* Globally configure an http file transfer server. */
	linphone_core_set_file_transfer_server(pauline->lc, file_transfer_url);

	/* Enable auto download on marie's Core */
	linphone_core_set_max_size_for_auto_download_incoming_files(marie->lc, 0);

	/* create a chatroom on pauline's side */
	LinphoneChatRoom *chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	LinphoneChatMessage *msg = create_message_from_sintel_trailer(chat_room);
	linphone_chat_message_send(msg);

	belle_http_provider_set_recv_error(linphone_core_get_http_provider(marie->lc), -1);
	/* wait for marie to receive pauline's msg */
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageSent, 1, 5000));

	BC_ASSERT_TRUE(
	    wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceivedWithFile, 1, 5000));
	LinphoneChatMessage *marie_msg = marie->stat.last_received_chat_message;
	BC_ASSERT_PTR_NOT_NULL(marie_msg);

	// We auto download using random file name, so we expect above file to not exist yet
	BC_ASSERT_EQUAL(bctbx_file_exist(path), -1, int, "%d");

	linphone_core_manager_stop(marie);

	BC_ASSERT_EQUAL(bctbx_file_exist(path), -1, int, "%d");
	if (path) bctbx_free(path);
	if (dl_path) bctbx_free(dl_path);

	linphone_core_manager_restart(marie, TRUE);

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneFileTransferDownloadSuccessful, 0, int, "%d");
	BC_ASSERT_NOT_EQUAL(marie->stat.number_of_LinphoneMessageNotDelivered, 1, int, "%d");

	LinphoneChatRoom *marie_cr = linphone_core_get_chat_room(marie->lc, pauline->identity);
	marie_msg = linphone_chat_room_get_last_message_in_history(marie_cr);
	BC_ASSERT_PTR_NOT_NULL(marie_msg);
	if (marie_msg) {
		const bctbx_list_t *contents = linphone_chat_message_get_contents(marie_msg);
		BC_ASSERT_PTR_NOT_NULL(contents);
		BC_ASSERT_EQUAL((int)bctbx_list_size(contents), 1, int, "%d");
		LinphoneContent *content = (LinphoneContent *)bctbx_list_get_data(contents);
		BC_ASSERT_STRING_EQUAL(linphone_content_get_type(content), "application");
		BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(content), "vnd.gsma.rcs-ft-http+xml");

		LinphoneChatMessageState state = linphone_chat_message_get_state(marie_msg);
		BC_ASSERT_EQUAL(state, LinphoneChatMessageStateDelivered, int, "%d");

		// Auto download isn't resumed automatically, and since the manager restarted the stats are reset
		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneMessageFileTransferInProgress, 0, int, "%d");

		char *send_filepath = bc_tester_res("sounds/sintel_trailer_opus_h264.mkv");

		LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(marie_msg);
		linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
		linphone_chat_message_cbs_set_file_transfer_recv(cbs, file_transfer_received);
		linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);
		linphone_chat_message_download_file(marie_msg);

		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneMessageFileTransferInProgress, 1, int, "%d");
		if (BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
		                                  &marie->stat.number_of_LinphoneFileTransferDownloadSuccessful, 1, 55000))) {
			// file_transfer_received function store file name into file_transfer_filepath
			const char *receive_filepath = linphone_chat_message_get_file_transfer_filepath(marie_msg);
			compare_files(send_filepath, receive_filepath);
		}

		linphone_chat_message_unref(marie_msg);
		bctbx_free(send_filepath);
	}
	linphone_chat_message_unref(msg);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void transfer_message_core_stopped_async(bool_t remote_available) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	/* Globally configure an http file transfer server. */
	linphone_core_set_file_transfer_server(pauline->lc, file_transfer_url);

	/* create a chatroom on pauline's side */
	LinphoneChatRoom *chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	LinphoneChatMessage *msg = create_message_from_sintel_trailer(chat_room);

	if (!remote_available) {
		linphone_core_set_network_reachable_internal(marie->lc, FALSE);
	}

	linphone_chat_message_send(msg);
	linphone_core_stop_async(pauline->lc);

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDelivered, 1, 10000));
	linphone_chat_message_unref(msg);

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneGlobalOff, 1, 5000));
	linphone_core_manager_destroy_after_stop_async(pauline);

	if (!remote_available) {
		linphone_core_set_network_reachable_internal(marie->lc, TRUE);
	}
	BC_ASSERT_TRUE(wait_for_until(NULL, marie->lc, &marie->stat.number_of_LinphoneMessageReceivedWithFile, 1, 1000));
	linphone_core_manager_destroy(marie);
}

static void transfer_message_core_stopped_async_1(void) {
	transfer_message_core_stopped_async(TRUE);
}

static void transfer_message_core_stopped_async_2(void) {
	transfer_message_core_stopped_async(FALSE);
}

static void file_transfer_2_messages_simultaneously(void) {
	if (transport_supported(LinphoneTransportTls)) {
		LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
		LinphoneChatRoom *pauline_room;
		LinphoneChatMessage *msg;
		LinphoneChatMessage *msg2;
		char *send_filepath = bc_tester_res("sounds/sintel_trailer_opus_h264.mkv");
		LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

		/* Globally configure an http file transfer server. */
		linphone_core_set_file_transfer_server(pauline->lc, file_transfer_url);

		/* create a chatroom on pauline's side */
		pauline_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
		char *filename = random_filename("sintel_trailer_opus_h264", "mkv");
		msg = _create_message_from_sintel_trailer(pauline_room, FALSE, "sounds/sintel_trailer_opus_h264.mkv", filename);
		msg2 = _create_message_from_sintel_trailer(pauline_room, TRUE, "sounds/sintel_trailer_opus_h264.mkv", filename);

		// Check that Marie has no active chat rooms with Pauline.
		// This assert might fail if a previous test didn't clean all its contexts or wait for a message to be sent
		const bctbx_list_t *chat_rooms = linphone_account_get_chat_rooms(linphone_core_get_default_account(marie->lc));
		BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(chat_rooms), 0, unsigned int, "%u");
		if (bctbx_list_size(chat_rooms) == 0) {
			linphone_chat_message_send(msg);
			linphone_chat_message_send(msg2);
			if (BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
			                                  &marie->stat.number_of_LinphoneMessageReceivedWithFile, 2, 60000))) {
				LinphoneChatMessage *recvMsg;
				LinphoneChatMessage *recvMsg2;
				bctbx_list_t *history;
				chat_rooms = linphone_account_get_chat_rooms(linphone_core_get_default_account(marie->lc));
				LinphoneChatRoom *cr;

				BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(chat_rooms), 1, unsigned int, "%u");
				if (bctbx_list_size(chat_rooms) != 1) {
					char *buf = ms_strdup_printf("Found %d rooms instead of 1: ", (int)bctbx_list_size(chat_rooms));
					const bctbx_list_t *it = chat_rooms;
					while (it) {
						const LinphoneAddress *peer = linphone_chat_room_get_peer_address(it->data);
						buf = ms_strcat_printf(buf, "%s, ", linphone_address_get_username(peer));
						it = it->next;
					}
					ms_error("%s", buf);
					ms_free(buf);
				}

				cr = chat_rooms ? (LinphoneChatRoom *)chat_rooms->data : NULL;
				if (BC_ASSERT_PTR_NOT_NULL(cr)) {
					history = linphone_chat_room_get_history(cr, -1);
					BC_ASSERT_TRUE(bctbx_list_size(history) == 2);
					recvMsg = (LinphoneChatMessage *)history->data;
					recvMsg2 = (LinphoneChatMessage *)history->next->data;

					LinphoneChatMessageCbs *cbs = linphone_factory_create_chat_message_cbs(linphone_factory_get());
					linphone_chat_message_cbs_set_msg_state_changed(cbs,
					                                                liblinphone_tester_chat_message_msg_state_changed);
					linphone_chat_message_cbs_set_file_transfer_recv(cbs, file_transfer_received);
					linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs,
					                                                                file_transfer_progress_indication);
					linphone_chat_message_add_callbacks(recvMsg, cbs);
					linphone_chat_message_cbs_unref(cbs);
					linphone_chat_message_download_file(recvMsg);

					cbs = linphone_factory_create_chat_message_cbs(linphone_factory_get());
					linphone_chat_message_cbs_set_msg_state_changed(cbs,
					                                                liblinphone_tester_chat_message_msg_state_changed);
					linphone_chat_message_cbs_set_file_transfer_recv(cbs, file_transfer_received);
					linphone_chat_message_cbs_set_file_transfer_progress_indication(
					    cbs, file_transfer_progress_indication_2);
					linphone_chat_message_add_callbacks(recvMsg2, cbs);
					linphone_chat_message_cbs_unref(cbs);
					linphone_chat_message_download_file(recvMsg2);

					BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
					                              &marie->stat.number_of_LinphoneFileTransferDownloadSuccessful, 2,
					                              50000));

					BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageFileTransferInProgress, 2, int, "%d");
					BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageInProgress, 2, int, "%d");
					BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageDelivered, 2, int, "%d");
					compare_files(send_filepath, linphone_chat_message_get_file_transfer_filepath(recvMsg));
					remove(linphone_chat_message_get_file_transfer_filepath(recvMsg));
					remove(linphone_chat_message_get_file_transfer_filepath(recvMsg2));

					bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_chat_message_unref);
				}
			}
		}
		linphone_chat_message_unref(msg);
		linphone_chat_message_unref(msg2);
		linphone_core_manager_destroy(pauline);
		bctbx_free(send_filepath);
		bctbx_free(filename);
		linphone_core_manager_destroy(marie);
	}
}

static void file_transfer_external_body_url(bool_t use_file_body_handler_in_download, bool_t use_invalid_url) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneChatRoom *chat_room = linphone_core_get_chat_room(marie->lc, pauline->identity);
	LinphoneChatMessage *msg = linphone_chat_room_create_message_from_utf8(chat_room, NULL);
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
	char *receive_filepath = random_filepath("receive_file", "dump");

	if (use_invalid_url) {
		linphone_chat_message_set_external_body_url(
		    msg, "https://transfer.example.org:444/download/0aa00aaa00a0a_a0000d00aaa0a0aaaa00.jpg");
	} else {
		linphone_chat_message_set_external_body_url(msg, "http://sip.example.org/vcards.vcf");
	}
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg);
	linphone_chat_message_unref(msg);

	BC_ASSERT_TRUE(
	    wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageReceivedWithFile, 1, 60000));

	LinphoneChatMessage *recv_msg = pauline->stat.last_received_chat_message;
	if (recv_msg) {
		recv_msg = pauline->stat.last_received_chat_message;
		cbs = linphone_chat_message_get_callbacks(recv_msg);
		linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
		linphone_chat_message_cbs_set_file_transfer_recv(cbs, file_transfer_received);
		linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);
		if (use_file_body_handler_in_download) {
			/* Remove any previously downloaded file */
			remove(receive_filepath);
			linphone_chat_message_set_file_transfer_filepath(recv_msg, receive_filepath);
		}
		linphone_chat_message_download_file(recv_msg);

		/* wait for a long time in case the DNS SRV resolution takes times - it should be immediate though */
		if (use_invalid_url) {
			BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
			                              &pauline->stat.number_of_LinphoneMessageFileTransferError, 1, 55000));
			BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageFileTransferDone, 0, int, "%d");
			BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageDisplayed, 0, int, "%d");
		} else {
			BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
			                              &pauline->stat.number_of_LinphoneFileTransferDownloadSuccessful, 1, 55000));
		}
		remove(linphone_chat_message_get_file_transfer_filepath(recv_msg));
	}

	bctbx_free(receive_filepath);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void file_transfer_using_external_body_url(void) {
	file_transfer_external_body_url(FALSE, FALSE);
}

static void file_transfer_using_external_body_url_2(void) {
	file_transfer_external_body_url(TRUE, FALSE);
}

static void file_transfer_using_external_body_url_404(void) {
	file_transfer_external_body_url(FALSE, TRUE);
}

static void text_message_denied(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneChatRoom *chat_room = linphone_core_get_chat_room(marie->lc, pauline->identity);
	LinphoneChatMessage *msg = linphone_chat_room_create_message_from_utf8(chat_room, "Bli bli bli \n blu");
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);

	/*pauline doesn't want to be disturbed*/
	linphone_core_disable_chat(pauline->lc, LinphoneReasonDoNotDisturb);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageNotDelivered, 1));
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageReceived, 0, int, "%d");
	linphone_chat_message_unref(msg);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static const char *info_content = "<somexml>blabla</somexml>";

void info_message_base(bool_t with_content) {
	LinphoneInfoMessage *info;
	const LinphoneContent *content;
	const char *hvalue;

	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	if (BC_ASSERT_TRUE(call(pauline, marie))) {

		info = linphone_core_create_info_message(marie->lc);
		linphone_info_message_add_header(info, "Weather", "still bad");
		if (with_content) {
			LinphoneContent *content = linphone_core_create_content(marie->lc);
			linphone_content_set_type(content, "application");
			linphone_content_set_subtype(content, "somexml");
			linphone_content_set_buffer(content, (const uint8_t *)info_content, strlen(info_content));
			linphone_info_message_set_content(info, content);
			linphone_content_unref(content);
		}
		linphone_call_send_info_message(linphone_core_get_current_call(marie->lc), info);
		linphone_info_message_unref(info);

		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_InfoReceived, 1));

		BC_ASSERT_PTR_NOT_NULL(pauline->stat.last_received_info_message);
		hvalue = linphone_info_message_get_header(pauline->stat.last_received_info_message, "Weather");
		content = linphone_info_message_get_content(pauline->stat.last_received_info_message);

		BC_ASSERT_PTR_NOT_NULL(hvalue);
		if (hvalue) BC_ASSERT_STRING_EQUAL(hvalue, "still bad");

		if (with_content) {
			BC_ASSERT_PTR_NOT_NULL(content);
			if (content) {
				BC_ASSERT_PTR_NOT_NULL(linphone_content_get_buffer(content));
				BC_ASSERT_PTR_NOT_NULL(linphone_content_get_type(content));
				BC_ASSERT_PTR_NOT_NULL(linphone_content_get_subtype(content));
				if (linphone_content_get_type(content))
					BC_ASSERT_STRING_EQUAL(linphone_content_get_type(content), "application");
				if (linphone_content_get_subtype(content))
					BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(content), "somexml");
				if (linphone_content_get_buffer(content))
					BC_ASSERT_STRING_EQUAL(linphone_content_get_utf8_text(content), info_content);
				BC_ASSERT_EQUAL((int)linphone_content_get_size(content), (int)strlen(info_content), int, "%d");
			}
		}
		end_call(marie, pauline);
	}
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void info_message(void) {
	info_message_base(FALSE);
}

static void info_message_with_body(void) {
	info_message_base(TRUE);
}

#ifdef HAVE_ADVANCED_IM
static void is_composing_notification(void) {
	LinphoneChatRoom *pauline_chat_room;
	LinphoneChatRoom *marie_chat_room;
	int dummy = 0;
	const bctbx_list_t *composing_addresses = NULL;

	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "deliver_imdn", 1);

	pauline_chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	marie_chat_room = linphone_core_get_chat_room(marie->lc, pauline->identity);
	linphone_core_get_chat_room(marie->lc, pauline->identity); /*make marie create the chatroom with pauline, which is
	                                                              necessary for receiving the is-composing*/
	linphone_chat_room_compose(pauline_chat_room);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneIsComposingActiveReceived, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageSent, 0));
	composing_addresses = linphone_chat_room_get_composing_addresses(marie_chat_room);
	BC_ASSERT_GREATER((int)bctbx_list_size(composing_addresses), 0, int, "%i");
	if (bctbx_list_size(composing_addresses) > 0) {
		LinphoneAddress *addr = (LinphoneAddress *)bctbx_list_get_data(composing_addresses);
		char *address_string = linphone_address_as_string(addr);
		char *pauline_address = linphone_address_as_string(pauline->identity);
		BC_ASSERT_STRING_EQUAL(address_string, pauline_address);
		bctbx_free(address_string);
		bctbx_free(pauline_address);
	}
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
	LinphoneChatMessage *is_composing_msg = marie->stat.last_received_chat_message;
	if (BC_ASSERT_PTR_NOT_NULL(is_composing_msg)) {
		const char *expires = linphone_chat_message_get_custom_header(is_composing_msg, "Expires");
		if (BC_ASSERT_PTR_NOT_NULL(expires)) BC_ASSERT_STRING_EQUAL(expires, "0");

		const char *priority = linphone_chat_message_get_custom_header(is_composing_msg, "Priority");
		if (BC_ASSERT_PTR_NOT_NULL(priority)) BC_ASSERT_STRING_EQUAL(priority, "non-urgent");
	}
	wait_for_until(pauline->lc, marie->lc, &dummy, 1, 1500); /*just to sleep while iterating*/
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneIsComposingIdleReceived, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 2));
	is_composing_msg = marie->stat.last_received_chat_message;
	if (BC_ASSERT_PTR_NOT_NULL(is_composing_msg)) {
		const char *expires = linphone_chat_message_get_custom_header(is_composing_msg, "Expires");
		if (BC_ASSERT_PTR_NOT_NULL(expires)) BC_ASSERT_STRING_EQUAL(expires, "0");

		const char *priority = linphone_chat_message_get_custom_header(is_composing_msg, "Priority");
		if (BC_ASSERT_PTR_NOT_NULL(priority)) BC_ASSERT_STRING_EQUAL(priority, "non-urgent");
	}
	linphone_chat_room_send_message(pauline_chat_room, "Composing a msg");
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneIsComposingIdleReceived, 2));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageSent, 1));
	composing_addresses = linphone_chat_room_get_composing_addresses(marie_chat_room);
	BC_ASSERT_EQUAL((int)bctbx_list_size(composing_addresses), 0, int, "%i");
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 3));

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void imdn_notifications(void) {
	if (!linphone_factory_is_database_storage_available(linphone_factory_get())) {
		ms_warning("Test skipped, database storage is not available");
		return;
	}

	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "deliver_imdn", 1);
	LinphoneChatRoom *pauline_chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	LinphoneChatRoom *marie_chat_room;
	LinphoneChatMessage *sent_cm;
	LinphoneChatMessage *received_cm;
	LinphoneChatMessageCbs *cbs;
	bctbx_list_t *history;

	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
	sent_cm = linphone_chat_room_create_message_from_utf8(pauline_chat_room, "Tell me if you get my message");
	cbs = linphone_chat_message_get_callbacks(sent_cm);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(sent_cm);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageSent, 1));
	marie_chat_room = linphone_core_get_chat_room(marie->lc, pauline->identity);
	history = linphone_chat_room_get_history(marie_chat_room, 1);
	BC_ASSERT_EQUAL((int)bctbx_list_size(history), 1, int, "%d");
	if (bctbx_list_size(history) > 0) {
		received_cm = (LinphoneChatMessage *)bctbx_list_nth_data(history, 0);
		BC_ASSERT_PTR_NOT_NULL(received_cm);
		if (received_cm != NULL) {
			BC_ASSERT_TRUE(
			    wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDeliveredToUser, 1));
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageReceived, 1));
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageSent, 0));
			LinphoneChatMessage *imdn_message = pauline->stat.last_received_chat_message;
			if (BC_ASSERT_PTR_NOT_NULL(imdn_message)) {
				const char *priority = linphone_chat_message_get_custom_header(imdn_message, "Priority");
				if (BC_ASSERT_PTR_NOT_NULL(priority)) BC_ASSERT_STRING_EQUAL(priority, "non-urgent");
			}

			// linphone_chat_room_mark_as_read(marie_chat_room); /* This sends the display notification */
			BC_ASSERT_TRUE(linphone_chat_room_get_unread_messages_count(marie_chat_room) == 1);
			linphone_chat_message_mark_as_read(received_cm); /* This sends the display notification */
			BC_ASSERT_TRUE(linphone_chat_room_get_unread_messages_count(marie_chat_room) == 0);

			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDisplayed, 1));
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageReceived, 2));
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageSent, 0));
			imdn_message = pauline->stat.last_received_chat_message;
			if (BC_ASSERT_PTR_NOT_NULL(imdn_message)) {
				const char *priority = linphone_chat_message_get_custom_header(imdn_message, "Priority");
				if (BC_ASSERT_PTR_NOT_NULL(priority)) BC_ASSERT_STRING_EQUAL(priority, "non-urgent");
			}
			bctbx_list_free_with_data(history, (bctbx_list_free_func)linphone_chat_message_unref);
		}
	}
	linphone_chat_message_unref(sent_cm);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void im_notification_policy(void) {
	if (!linphone_factory_is_database_storage_available(linphone_factory_get())) {
		ms_warning("Test skipped, database storage is not available");
		return;
	}

	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneImNotifPolicy *marie_policy = linphone_core_get_im_notif_policy(marie->lc);
	LinphoneImNotifPolicy *pauline_policy = linphone_core_get_im_notif_policy(pauline->lc);
	LinphoneChatRoom *pauline_chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	LinphoneChatRoom *marie_chat_room;
	LinphoneChatMessage *msg1;
	LinphoneChatMessage *msg2;
	LinphoneChatMessage *msg3;
	LinphoneChatMessage *msg4;
	LinphoneChatMessageCbs *cbs;
	int dummy = 0;

	linphone_im_notif_policy_enable_all(marie_policy);
	linphone_im_notif_policy_clear(pauline_policy);
	marie_chat_room =
	    linphone_core_get_chat_room(marie->lc, pauline->identity); /* Make marie create the chatroom with pauline, which
	                                                                  is necessary for receiving the is-composing */

	/* Test is_composing sending */
	linphone_chat_room_compose(pauline_chat_room);
	wait_for_until(pauline->lc, marie->lc, &dummy, 1, 1500); /* Just to sleep while iterating */
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneIsComposingActiveReceived, 0, int, "%d");
	linphone_im_notif_policy_set_send_is_composing(pauline_policy, TRUE);
	linphone_chat_room_compose(pauline_chat_room);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneIsComposingActiveReceived, 1));

	/* Test is_composing receiving */
	linphone_chat_room_compose(marie_chat_room);
	wait_for_until(pauline->lc, marie->lc, &dummy, 1, 1500); /* Just to sleep while iterating */
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneIsComposingActiveReceived, 0, int, "%d");
	linphone_im_notif_policy_set_recv_is_composing(pauline_policy, TRUE);
	linphone_chat_room_compose(marie_chat_room);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneIsComposingActiveReceived, 1));

	/* Test imdn delivered */
	msg1 = linphone_chat_room_create_message_from_utf8(pauline_chat_room, "Happy new year!");
	cbs = linphone_chat_message_get_callbacks(msg1);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg1);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
	wait_for_until(pauline->lc, marie->lc, &dummy, 1, 1500); /* Just to sleep while iterating */
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneMessageDeliveredToUser, 0, int, "%d");
	linphone_im_notif_policy_set_recv_imdn_delivered(pauline_policy, TRUE);
	msg2 = linphone_chat_room_create_message_from_utf8(pauline_chat_room, "I said: Happy new year!");
	cbs = linphone_chat_message_get_callbacks(msg2);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg2);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 2));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDeliveredToUser, 1));
	msg3 = linphone_chat_room_create_message_from_utf8(marie_chat_room, "Thank you! Happy easter to you!");
	cbs = linphone_chat_message_get_callbacks(msg3);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg3);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageReceived, 1));
	wait_for_until(pauline->lc, marie->lc, &dummy, 1, 1500); /* Just to sleep while iterating */
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneMessageDeliveredToUser, 0, int, "%d");
	linphone_im_notif_policy_set_send_imdn_delivered(pauline_policy, TRUE);
	msg4 = linphone_chat_room_create_message_from_utf8(marie_chat_room, "Yeah, yeah, I heard that...");
	cbs = linphone_chat_message_get_callbacks(msg4);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg4);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageReceived, 2));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageDeliveredToUser, 1));

	/* Test imdn displayed */
	linphone_im_notif_policy_set_send_imdn_displayed(pauline_policy, TRUE);
	linphone_chat_room_mark_as_read(pauline_chat_room);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageDisplayed, 2));
	linphone_im_notif_policy_set_recv_imdn_displayed(pauline_policy, TRUE);
	linphone_chat_room_mark_as_read(marie_chat_room);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDisplayed, 2));

	linphone_chat_message_unref(msg1);
	linphone_chat_message_unref(msg2);
	linphone_chat_message_unref(msg3);
	linphone_chat_message_unref(msg4);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void aggregated_imdns(void) {
	if (!linphone_factory_is_database_storage_available(linphone_factory_get())) {
		ms_warning("Test skipped, database storage is not available");
		return;
	}

	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "deliver_imdn", 1);
	LinphoneChatRoom *pauline_chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	LinphoneChatRoom *marie_chat_room;
	LinphoneChatMessage *sent_cm;
	bctbx_list_t *messages = NULL;

	/*set marie in airplaine mode: */
	linphone_core_set_network_reachable(marie->lc, FALSE);

	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));

	sent_cm = linphone_chat_room_create_message_from_utf8(pauline_chat_room, "Coucou");
	linphone_chat_message_cbs_set_msg_state_changed(linphone_chat_message_get_callbacks(sent_cm),
	                                                liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(sent_cm);
	messages = bctbx_list_append(messages, sent_cm);

	sent_cm = linphone_chat_room_create_message_from_utf8(pauline_chat_room, "ça va ?");
	linphone_chat_message_cbs_set_msg_state_changed(linphone_chat_message_get_callbacks(sent_cm),
	                                                liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(sent_cm);
	messages = bctbx_list_append(messages, sent_cm);

	sent_cm = linphone_chat_room_create_message_from_utf8(pauline_chat_room, "tu fais quoi ce soir ?");
	linphone_chat_message_cbs_set_msg_state_changed(linphone_chat_message_get_callbacks(sent_cm),
	                                                liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(sent_cm);
	messages = bctbx_list_append(messages, sent_cm);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageSent, 3));

	/* Marie shall not receive them */
	BC_ASSERT_FALSE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1, 3000));

	/* Marie is now online */
	linphone_core_set_network_reachable(marie->lc, TRUE);

	/* Marie should receive them all */
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 3));

	/* Pauline should be notified of good delivery */
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDeliveredToUser, 3));

	marie_chat_room = linphone_core_get_chat_room(marie->lc, pauline->identity);

	ms_message("Marking all %s's messages as read", linphone_core_get_identity(marie->lc));
	linphone_chat_room_mark_as_read(marie_chat_room); /* This sends the display notification */

	/* Pauline should be notified of messages being displayed */
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDisplayed, 3));

	bctbx_list_free_with_data(messages, (bctbx_list_free_func)linphone_chat_message_unref);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void aggregated_imdns_in_group_chat_base(const LinphoneTesterLimeAlgo curveId) {
	LinphoneCoreManager *marie = linphone_core_manager_create("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_create("pauline_rc");
	LinphoneCoreManager *chloe = linphone_core_manager_create("chloe_rc");
	bctbx_list_t *coresManagerList = NULL;
	bctbx_list_t *participantsAddresses = NULL;
	coresManagerList = bctbx_list_append(coresManagerList, marie);
	coresManagerList = bctbx_list_append(coresManagerList, pauline);
	coresManagerList = bctbx_list_append(coresManagerList, chloe);
	bool_t encrypted = FALSE;
	if (curveId != UNSET) {
		encrypted = TRUE;
		set_lime_server_and_curve_list(curveId, coresManagerList);
	}
	setup_mgr_for_conference(marie, NULL);
	setup_mgr_for_conference(pauline, NULL);
	setup_mgr_for_conference(chloe, NULL);
	bctbx_list_t *coresList = init_core_for_conference(coresManagerList);
	start_core_for_conference(coresManagerList);
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(pauline->lc)));
	participantsAddresses =
	    bctbx_list_append(participantsAddresses, linphone_address_new(linphone_core_get_identity(chloe->lc)));

	linphone_core_set_imdn_to_everybody_threshold(marie->lc, 1);
	linphone_core_set_chat_messages_aggregation_enabled(marie->lc, TRUE);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "chat_messages_aggregation_delay", 10);
	linphone_core_set_imdn_to_everybody_threshold(pauline->lc, 1);
	linphone_core_set_chat_messages_aggregation_enabled(pauline->lc, TRUE);
	linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "chat_messages_aggregation_delay", 10);
	linphone_core_set_imdn_to_everybody_threshold(chloe->lc, 1);
	linphone_core_set_chat_messages_aggregation_enabled(chloe->lc, TRUE);
	linphone_config_set_int(linphone_core_get_config(chloe->lc), "sip", "chat_messages_aggregation_delay", 10);

	// Enable IMDN
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
	linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(chloe->lc));

	stats initialMarieStats = marie->stat;
	stats initialPaulineStats = pauline->stat;
	stats initialChloeStats = chloe->stat;

	// Marie creates a new group chat room
	const char *initialSubject = "Colleagues";
	LinphoneChatRoom *marieCr =
	    create_chat_room_client_side(coresList, marie, &initialMarieStats, participantsAddresses, initialSubject,
	                                 encrypted, LinphoneChatRoomEphemeralModeDeviceManaged);
	const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

	// Check that the chat room is correctly created on Pauline's side and that the participants are added
	LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline, &initialPaulineStats,
	                                                                   confAddr, initialSubject, 2, FALSE);

	// Check that the chat room is correctly created on Chloe's side and that the participants are added
	LinphoneChatRoom *chloeCr =
	    check_creation_chat_room_client_side(coresList, chloe, &initialChloeStats, confAddr, initialSubject, 2, FALSE);

	int nbMessages = 10;
	bctbx_list_t *messages = NULL;
	for (int idx = 0; idx < nbMessages; idx++) {
		char messageText[100];
		sprintf(messageText, "Hello everybody - attempt %0d", idx);
		LinphoneChatMessage *marieMessage = _send_message(marieCr, messageText);
		BC_ASSERT_PTR_NOT_NULL(marieMessage);
		if (marieMessage) {
			messages = bctbx_list_append(messages, marieMessage);
		}
	}

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneAggregatedMessagesReceived, nbMessages,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneAggregatedMessagesReceived, nbMessages,
	                             liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDeliveredToUser, nbMessages,
	                             liblinphone_tester_sip_timeout));

	// Only Marie receives IMDNs
	LinphoneChatMessageState expected_msg_state = LinphoneChatMessageStateDeliveredToUser;
	if (marieCr) {
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), nbMessages, int, "%d");
		bctbx_list_t *marie_history = linphone_chat_room_get_history(marieCr, nbMessages + 1);
		for (bctbx_list_t *item = marie_history; item; item = bctbx_list_next(item)) {
			LinphoneChatMessage *msg = (LinphoneChatMessage *)bctbx_list_get_data(item);
			BC_ASSERT_EQUAL(linphone_chat_message_get_state(msg), expected_msg_state, int, "%d");
		}
		bctbx_list_free_with_data(marie_history, (bctbx_list_free_func)linphone_chat_message_unref);
	}

	linphone_chat_room_mark_as_read(paulineCr);
	linphone_chat_room_mark_as_read(chloeCr);

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneMessageDisplayed, nbMessages,
	                             liblinphone_tester_sip_timeout));

	expected_msg_state = LinphoneChatMessageStateDisplayed;
	if (marieCr) {
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(marieCr), nbMessages, int, "%d");
		bctbx_list_t *marie_history = linphone_chat_room_get_history(marieCr, nbMessages + 1);
		for (bctbx_list_t *item = marie_history; item; item = bctbx_list_next(item)) {
			LinphoneChatMessage *msg = (LinphoneChatMessage *)bctbx_list_get_data(item);
			BC_ASSERT_EQUAL(linphone_chat_message_get_state(msg), expected_msg_state, int, "%d");
		}
		bctbx_list_free_with_data(marie_history, (bctbx_list_free_func)linphone_chat_message_unref);
	}

	if (messages) {
		bctbx_list_free_with_data(messages, (bctbx_list_free_func)linphone_chat_message_unref);
		messages = NULL;
	}

	initialMarieStats = marie->stat;
	initialChloeStats = chloe->stat;
	initialPaulineStats = pauline->stat;
	bctbx_list_t *participants = bctbx_list_append(NULL, pauline->identity);
	ms_message("%s deletes one to one chatroom with %s", linphone_core_get_identity(marie->lc),
	           linphone_core_get_identity(pauline->lc));
	LinphoneChatRoom *mariePaulineOneToOneCr =
	    linphone_core_search_chat_room(marie->lc, NULL, marie->identity, NULL, participants);
	bctbx_list_free(participants);
	BC_ASSERT_PTR_NOT_NULL(mariePaulineOneToOneCr);
	if (mariePaulineOneToOneCr) {
		const LinphoneChatRoomParams *chat_room_params = linphone_chat_room_get_current_params(mariePaulineOneToOneCr);
		BC_ASSERT_PTR_NOT_NULL(chat_room_params);
		if (chat_room_params) {
			BC_ASSERT_FALSE(linphone_chat_room_params_group_enabled(chat_room_params));
		}
		linphone_chat_room_leave(mariePaulineOneToOneCr);
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateTerminationPending,
		                             initialPaulineStats.number_of_LinphoneChatRoomStateTerminationPending + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneChatRoomStateTerminated,
		                             initialPaulineStats.number_of_LinphoneChatRoomStateTerminated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateTerminationPending,
		                             initialMarieStats.number_of_LinphoneChatRoomStateTerminationPending + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneChatRoomStateTerminated,
		                             initialMarieStats.number_of_LinphoneChatRoomStateTerminated + 1,
		                             liblinphone_tester_sip_timeout));
	}

	linphone_core_refresh_registers(pauline->lc);
	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneRegistrationOk,
	                             initialPaulineStats.number_of_LinphoneRegistrationOk + 1,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneSubscriptionOutgoingProgress,
	                              initialPaulineStats.number_of_LinphoneSubscriptionOutgoingProgress + 1,
	                              liblinphone_tester_sip_timeout));
	BC_ASSERT_FALSE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneSubscriptionError,
	                              initialPaulineStats.number_of_LinphoneSubscriptionError + 1,
	                              liblinphone_tester_sip_timeout));

	for (int idx = 0; idx < nbMessages; idx++) {
		char messageText[100];
		sprintf(messageText, "Excited to be here - attempt %0d", idx);
		LinphoneChatMessage *paulineMessage = _send_message(paulineCr, messageText);
		BC_ASSERT_PTR_NOT_NULL(paulineMessage);
		if (paulineMessage) {
			messages = bctbx_list_append(messages, paulineMessage);
		}
	}

	BC_ASSERT_TRUE(wait_for_list(coresList, &marie->stat.number_of_LinphoneAggregatedMessagesReceived,
	                             initialPaulineStats.number_of_LinphoneMessageReceived + nbMessages,
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(coresList, &chloe->stat.number_of_LinphoneAggregatedMessagesReceived,
	                             initialChloeStats.number_of_LinphoneMessageReceived + nbMessages,
	                             liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDeliveredToUser,
	                             initialPaulineStats.number_of_LinphoneMessageDeliveredToUser + nbMessages,
	                             liblinphone_tester_sip_timeout));

	// Only Pauline receives IMDNs
	expected_msg_state = LinphoneChatMessageStateDeliveredToUser;
	if (paulineCr) {
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 2 * nbMessages, int, "%d");
		bctbx_list_t *pauline_history = linphone_chat_room_get_history(paulineCr, nbMessages);
		for (bctbx_list_t *item = pauline_history; item; item = bctbx_list_next(item)) {
			LinphoneChatMessage *msg = (LinphoneChatMessage *)bctbx_list_get_data(item);
			BC_ASSERT_EQUAL(linphone_chat_message_get_state(msg), expected_msg_state, int, "%d");
		}
		bctbx_list_free_with_data(pauline_history, (bctbx_list_free_func)linphone_chat_message_unref);
	}

	linphone_chat_room_mark_as_read(marieCr);
	linphone_chat_room_mark_as_read(chloeCr);

	BC_ASSERT_TRUE(wait_for_list(coresList, &pauline->stat.number_of_LinphoneMessageDisplayed,
	                             initialPaulineStats.number_of_LinphoneMessageDisplayed + nbMessages,
	                             liblinphone_tester_sip_timeout));

	expected_msg_state = LinphoneChatMessageStateDisplayed;
	if (paulineCr) {
		BC_ASSERT_EQUAL(linphone_chat_room_get_history_size(paulineCr), 2 * nbMessages, int, "%d");
		bctbx_list_t *pauline_history = linphone_chat_room_get_history(paulineCr, nbMessages);
		for (bctbx_list_t *item = pauline_history; item; item = bctbx_list_next(item)) {
			LinphoneChatMessage *msg = (LinphoneChatMessage *)bctbx_list_get_data(item);
			BC_ASSERT_EQUAL(linphone_chat_message_get_state(msg), expected_msg_state, int, "%d");
		}
		bctbx_list_free_with_data(pauline_history, (bctbx_list_free_func)linphone_chat_message_unref);
	}

	if (messages) {
		bctbx_list_free_with_data(messages, (bctbx_list_free_func)linphone_chat_message_unref);
		messages = NULL;
	}

	for (bctbx_list_t *coreIt = coresList; coreIt != NULL; coreIt = bctbx_list_next(coreIt)) {
		LinphoneCore *core = (LinphoneCore *)bctbx_list_get_data(coreIt);
		LinphoneCoreManager *mgr = get_manager(core);
		const bctbx_list_t *chatrooms = linphone_core_get_chat_rooms(core);
		for (const bctbx_list_t *chatroomIt = chatrooms; chatroomIt != NULL; chatroomIt = bctbx_list_next(chatroomIt)) {
			LinphoneChatRoom *chatroom = (LinphoneChatRoom *)bctbx_list_get_data(chatroomIt);
			linphone_core_manager_delete_chat_room(mgr, chatroom, coresList);
		}
	}

	bctbx_list_free(coresList);
	bctbx_list_free(coresManagerList);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(chloe);
}

static void aggregated_imdns_in_group_chat(void) {
	aggregated_imdns_in_group_chat_base(UNSET);
}
#endif

int check_no_strange_time(BCTBX_UNUSED(void *data), int argc, char **argv, char **cNames) {
	BC_ASSERT_EQUAL(argc, 1, int, "%d");
	BC_ASSERT_STRING_EQUAL(cNames[0], "COUNT(*)"); // count of non updated messages should be 0
	BC_ASSERT_STRING_EQUAL(argv[0], "0");          // count of non updated messages should be 0
	return 0;
}

void history_message_count_helper(LinphoneChatRoom *chatroom, int x, int y, unsigned int expected) {
	bctbx_list_t *messages = linphone_chat_room_get_history_range(chatroom, x, y);
	BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(messages), expected, unsigned int, "%u");
	bctbx_list_free_with_data(messages, (void (*)(void *))linphone_chat_message_unref);
}

void crash_during_file_transfer(void) {
	if (!linphone_factory_is_database_storage_available(linphone_factory_get())) {
		ms_warning("Test skipped, database storage is not available");
		return;
	}

	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneChatRoom *chat_room;
	LinphoneChatMessage *msg;
	int chat_room_size = 0;
	bctbx_list_t *msg_list = NULL;

	/* Globally configure an http file transfer server. */
	linphone_core_set_file_transfer_server(pauline->lc, file_transfer_url);

	/* Create a chatroom and a file transfer message on pauline's side */
	chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	msg = create_file_transfer_message_from_sintel_trailer(chat_room);
	linphone_chat_message_send(msg);

	/* Wait for 25% of the file to be uploaded and crash by stopping the iteration, saving the chat database and
	 * destroying the core */
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &pauline->stat.progress_of_LinphoneFileTransfer, 25, 60000));
	linphone_chat_message_unref(msg);
	linphone_core_manager_stop(pauline);

	/* Create a new core and check that the message stored in the saved database is in the not delivered state */
	linphone_core_manager_restart(pauline, TRUE);
	linphone_core_set_file_transfer_server(pauline->lc, file_transfer_url);

	// BC_ASSERT_TRUE(wait_for(pauline->lc, pauline->lc, &pauline->stat.number_of_LinphoneRegistrationOk, 1));

	chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	chat_room_size = linphone_chat_room_get_history_size(chat_room);
	BC_ASSERT_EQUAL(chat_room_size, 1, int, "%d");
	if (chat_room_size == 1) {
		msg_list = linphone_chat_room_get_history(chat_room, 0);
		LinphoneChatMessage *sent_msg = (LinphoneChatMessage *)bctbx_list_get_data(msg_list);
		BC_ASSERT_EQUAL((int)linphone_chat_message_get_state(sent_msg), (int)LinphoneChatMessageStateNotDelivered, int,
		                "%d");
		// resend
		linphone_chat_message_send(sent_msg);
		if (BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
		                                  &marie->stat.number_of_LinphoneMessageReceivedWithFile, 1, 60000))) {
			linphone_core_manager_stop(marie);
			/* Create a new core and check that the message stored in the saved database is in the not delivered state
			 */
			linphone_core_manager_restart(marie, TRUE);
			LinphoneChatRoom *marie_room = linphone_core_get_chat_room(marie->lc, pauline->identity);
			bctbx_list_t *msg_list_2 = linphone_chat_room_get_history(marie_room, 1);
			if (BC_ASSERT_PTR_NOT_NULL(msg_list_2)) {
				LinphoneChatMessage *recv_msg = (LinphoneChatMessage *)msg_list_2->data;
				LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(recv_msg);
				linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
				linphone_chat_message_cbs_set_file_transfer_recv(cbs, file_transfer_received);
				linphone_chat_message_cbs_set_file_transfer_progress_indication(cbs, file_transfer_progress_indication);
				linphone_chat_message_download_file(recv_msg);
				/* wait for a long time in case the DNS SRV resolution takes times - it should be immediate though */
				if (BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
				                                  &marie->stat.number_of_LinphoneFileTransferDownloadSuccessful, 1,
				                                  55000))) {
					BC_ASSERT_PTR_NULL(linphone_chat_message_get_external_body_url(recv_msg));
				}
				bctbx_list_free_with_data(msg_list_2, (bctbx_list_free_func)linphone_chat_message_unref);
			}
		}
	}

	bctbx_list_free_with_data(msg_list, (bctbx_list_free_func)linphone_chat_message_unref);

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void text_status_after_destroying_chat_room(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneChatRoom *chatroom =
	    linphone_core_get_chat_room_from_uri(marie->lc, "<sip:flexisip-tester@sip.linphone.org>");
	LinphoneChatMessage *msg = linphone_chat_room_create_message_from_utf8(chatroom, "hello");
	linphone_chat_message_send(msg);
	linphone_core_delete_chat_room(marie->lc, chatroom);
	// since message is orphan, we do not expect to be notified of state change
	BC_ASSERT_FALSE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneMessageNotDelivered, 1, 1000));
	linphone_chat_message_unref(msg);
	linphone_core_manager_destroy(marie);
}

static void file_transfer_not_sent_if_invalid_url(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneChatRoom *chatroom =
	    linphone_core_get_chat_room_from_uri(marie->lc, "<sip:flexisip-tester@sip.linphone.org>");
	LinphoneChatMessage *msg = create_message_from_sintel_trailer(chatroom);
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_core_set_file_transfer_server(marie->lc, "INVALID URL");
	linphone_chat_message_send(msg);
	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneMessageNotDelivered, 1, 1000));
	linphone_chat_message_unref(msg);
	linphone_core_manager_destroy(marie);
}

void file_transfer_io_error_base(bool_t host_not_found) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneChatRoom *chatroom =
	    linphone_core_get_chat_room_from_uri(marie->lc, "<sip:flexisip-tester@sip.linphone.org>");
	LinphoneChatMessage *msg = create_message_from_sintel_trailer(chatroom);
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	const char *server_url = host_not_found ? "https://not-existing-url.com" : "http://transfer.example.org/toto.php";
	linphone_core_set_file_transfer_server(marie->lc, server_url);
	linphone_chat_message_send(msg);
	BC_ASSERT_TRUE(
	    wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneMessageFileTransferInProgress, 1, 1000));
	if (host_not_found) {
		BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneMessagePendingDelivery, 1, 3000));
	} else {
		BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneMessageNotDelivered, 1, 3000));
	}
	linphone_chat_message_unref(msg);
	linphone_core_manager_destroy(marie);
}

static void file_transfer_not_sent_if_host_not_found(void) {
	file_transfer_io_error_base(TRUE);
}

static void file_transfer_not_sent_if_url_moved_permanently(void) {
	file_transfer_io_error_base(FALSE);
}

static void file_transfer_success_after_destroying_chatroom(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneChatRoom *chatroom =
	    linphone_core_get_chat_room_from_uri(marie->lc, "<sip:flexisip-tester@sip.linphone.org>");
	LinphoneChatMessage *msg = create_message_from_sintel_trailer(chatroom);
	linphone_chat_message_cbs_set_msg_state_changed(linphone_chat_message_get_callbacks(msg),
	                                                liblinphone_tester_chat_message_msg_state_changed);
	linphone_core_set_file_transfer_server(marie->lc, file_transfer_url);
	linphone_chat_message_send(msg);
	BC_ASSERT_TRUE(
	    wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneMessageFileTransferInProgress, 1, 1000));
	linphone_core_delete_chat_room(marie->lc, chatroom);
	// As of today (2019/02/07), chat message can no longer be sent without chatroom
	BC_ASSERT_FALSE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneMessageDisplayed, 1, 1000));
	linphone_chat_message_unref(msg);
	linphone_core_manager_destroy(marie);
}

static void real_time_text(bool_t audio_stream_enabled,
                           bool_t srtp_enabled,
                           bool_t mess_with_marie_payload_number,
                           bool_t mess_with_pauline_payload_number,
                           bool_t ice_enabled,
                           bool_t sql_storage,
                           bool_t do_not_store_rtt_messages_in_sql_storage,
                           bool_t existing_chat_room) {
	if (sql_storage && !linphone_factory_is_database_storage_available(linphone_factory_get())) {
		ms_warning("Test skipped, database storage is not available");
		return;
	}

	LinphoneChatRoom *pauline_chat_room;
	LinphoneChatRoom *marie_chat_room;
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCallParams *marie_params = NULL;
	LinphoneCall *pauline_call, *marie_call;
	char *marie_db = bc_tester_file("marie.db");
	char *pauline_db = bc_tester_file("pauline.db");

	if (existing_chat_room) {
		text_message_base(marie, pauline);
	}

	if (sql_storage) {
		linphone_core_set_chat_database_path(marie->lc, marie_db);
		linphone_core_set_chat_database_path(pauline->lc, pauline_db);

		if (do_not_store_rtt_messages_in_sql_storage) {
			linphone_config_set_int(linphone_core_get_config(marie->lc), "misc", "store_rtt_messages", 0);
			linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "store_rtt_messages", 0);
		}
	}

	if (mess_with_marie_payload_number) {
		bctbx_list_t *payloads = linphone_core_get_text_payload_types(pauline->lc);
		for (bctbx_list_t *elem = payloads; elem != NULL; elem = elem->next) {
			PayloadType *pt = (PayloadType *)elem->data;
			if (pt->mime_type && strcasecmp(pt->mime_type, payload_type_t140.mime_type) == 0) {
				payload_type_set_number(pt, 99);
				break;
			}
		}
		bctbx_list_free_with_data(payloads, (void (*)(void *))linphone_payload_type_unref);
	} else if (mess_with_pauline_payload_number) {
		bctbx_list_t *payloads = linphone_core_get_text_payload_types(pauline->lc);
		for (bctbx_list_t *elem = payloads; elem != NULL; elem = elem->next) {
			PayloadType *pt = (PayloadType *)elem->data;
			if (pt->mime_type && strcasecmp(pt->mime_type, payload_type_t140.mime_type) == 0) {
				payload_type_set_number(pt, 99);
				break;
			}
		}
		bctbx_list_free_with_data(payloads, (void (*)(void *))linphone_payload_type_unref);
	}

	if (ice_enabled) {
		linphone_core_set_firewall_policy(marie->lc, LinphonePolicyUseIce);
		linphone_core_set_firewall_policy(pauline->lc, LinphonePolicyUseIce);
	}

	if (srtp_enabled) {
		if (!ms_srtp_supported()) {
			ms_warning("test skipped, missing srtp support");
			goto srtp_end;
		}
		BC_ASSERT_TRUE(linphone_core_media_encryption_supported(marie->lc, LinphoneMediaEncryptionSRTP));
		linphone_core_set_media_encryption(marie->lc, LinphoneMediaEncryptionSRTP);
		linphone_core_set_media_encryption(pauline->lc, LinphoneMediaEncryptionSRTP);
		linphone_core_set_media_encryption_mandatory(marie->lc, TRUE);
		linphone_core_set_media_encryption_mandatory(pauline->lc, TRUE);
	}

	linphone_core_set_realtime_text_keepalive_interval(pauline->lc, 500);

	marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_realtime_text(marie_params, TRUE);
	if (!audio_stream_enabled) {
		linphone_call_params_enable_audio(marie_params, FALSE);
		linphone_core_set_nortp_timeout(marie->lc, 5);
		linphone_core_set_nortp_timeout(pauline->lc, 5);
	}

	BC_ASSERT_TRUE(call_with_caller_params(marie, pauline, marie_params));
	pauline_call = linphone_core_get_current_call(pauline->lc);
	marie_call = linphone_core_get_current_call(marie->lc);
	if (pauline_call) {
		BC_ASSERT_TRUE(linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(pauline_call)));
		BC_ASSERT_EQUAL(
		    linphone_call_params_get_realtime_text_keepalive_interval(linphone_call_get_current_params(pauline_call)),
		    500, unsigned int, "%u");

		if (audio_stream_enabled) {
			BC_ASSERT_TRUE(linphone_call_params_audio_enabled(linphone_call_get_current_params(pauline_call)));
		}

		pauline_chat_room = linphone_call_get_chat_room(pauline_call);
		marie_chat_room = linphone_call_get_chat_room(marie_call);
		BC_ASSERT_PTR_NOT_NULL(pauline_chat_room);
		BC_ASSERT_PTR_NOT_NULL(marie_chat_room);
		if (pauline_chat_room && marie_chat_room) {
			const char *message = "Be l3l";
			size_t i;
			LinphoneChatMessage *rtt_message = linphone_chat_room_create_message_from_utf8(pauline_chat_room, NULL);

			for (i = 0; i < strlen(message); i++) {
				BC_ASSERT_FALSE(linphone_chat_message_put_char(rtt_message, message[i]));
				BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
				                              &marie->stat.number_of_LinphoneIsComposingActiveReceived, (int)i + 1,
				                              3000));
				BC_ASSERT_EQUAL(linphone_chat_room_get_char(marie_chat_room), message[i], char, "%c");
			}
			linphone_chat_message_send(rtt_message);
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
			linphone_chat_message_unref(rtt_message);

			if (sql_storage) {
				bctbx_list_t *marie_messages = linphone_chat_room_get_history(marie_chat_room, 0);
				bctbx_list_t *pauline_messages = linphone_chat_room_get_history(pauline_chat_room, 0);
				LinphoneChatMessage *marie_msg = NULL;
				LinphoneChatMessage *pauline_msg = NULL;
				if (do_not_store_rtt_messages_in_sql_storage) {
					BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(marie_messages), 0, unsigned int, "%u");
					BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(pauline_messages), 0, unsigned int, "%u");
				} else {
					BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(marie_messages), 1, unsigned int, "%u");
					BC_ASSERT_EQUAL((unsigned int)bctbx_list_size(pauline_messages), 1, unsigned int, "%u");
					if (!marie_messages || !pauline_messages) {
						goto end;
					}
					marie_msg = (LinphoneChatMessage *)marie_messages->data;
					pauline_msg = (LinphoneChatMessage *)pauline_messages->data;
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie_msg), message);
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(pauline_msg), message);
					bctbx_list_free_with_data(marie_messages, (void (*)(void *))linphone_chat_message_unref);
					bctbx_list_free_with_data(pauline_messages, (void (*)(void *))linphone_chat_message_unref);
				}
			}
		}

		if (!audio_stream_enabled) {
			int dummy = 0;
			wait_for_until(pauline->lc, marie->lc, &dummy, 1,
			               7000); /* Wait to see if call is dropped after the nortp_timeout */
			BC_ASSERT_FALSE(marie->stat.number_of_LinphoneCallEnd > 0);
			BC_ASSERT_FALSE(pauline->stat.number_of_LinphoneCallEnd > 0);
		}

		if (ice_enabled) {
			BC_ASSERT_TRUE(check_ice(pauline, marie, LinphoneIceStateHostConnection));
		}

	end:
		end_call(marie, pauline);
	}
	linphone_call_params_unref(marie_params);
srtp_end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	remove(marie_db);
	bctbx_free(marie_db);
	remove(pauline_db);
	bctbx_free(pauline_db);
}

static void real_time_text_message(void) {
	real_time_text(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);
}

static void real_time_text_message_with_existing_basic_chat_room(void) {
	real_time_text(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE);
}

static void real_time_text_sql_storage(void) {
	real_time_text(TRUE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE);
}

static void real_time_text_sql_storage_rtt_disabled(void) {
	real_time_text(TRUE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE);
}

static void real_time_text_conversation(void) {
	LinphoneChatRoom *pauline_chat_room, *marie_chat_room;
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCallParams *marie_params = linphone_core_create_call_params(marie->lc, NULL);
	LinphoneCall *pauline_call, *marie_call;
	linphone_call_params_enable_realtime_text(marie_params, TRUE);

	if (BC_ASSERT_TRUE(call_with_caller_params(marie, pauline, marie_params))) {
		pauline_call = linphone_core_get_current_call(pauline->lc);
		marie_call = linphone_core_get_current_call(marie->lc);
		BC_ASSERT_TRUE(linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(pauline_call)));

		pauline_chat_room = linphone_call_get_chat_room(pauline_call);
		BC_ASSERT_PTR_NOT_NULL(pauline_chat_room);
		marie_chat_room = linphone_call_get_chat_room(marie_call);
		BC_ASSERT_PTR_NOT_NULL(marie_chat_room);
		if (pauline_chat_room && marie_chat_room) {
			const char *message1_1 = "Lorem";
			const char *message1_2 = "Ipsum";
			const char *message2_1 = "Be lle Com";
			const char *message2_2 = "eB ell moC";
			size_t i;
			LinphoneChatMessage *pauline_rtt_message =
			    linphone_chat_room_create_message_from_utf8(pauline_chat_room, NULL);
			LinphoneChatMessage *marie_rtt_message = linphone_chat_room_create_message_from_utf8(marie_chat_room, NULL);

			for (i = 0; i < strlen(message1_1); i++) {
				linphone_chat_message_put_char(pauline_rtt_message, message1_1[i]);
				BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
				                              &marie->stat.number_of_LinphoneIsComposingActiveReceived, (int)i + 1,
				                              5000));
				BC_ASSERT_EQUAL(linphone_chat_room_get_char(marie_chat_room), message1_1[i], char, "%c");

				linphone_chat_message_put_char(marie_rtt_message, message1_2[i]);
				BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
				                              &pauline->stat.number_of_LinphoneIsComposingActiveReceived, (int)i + 1,
				                              5000));
				BC_ASSERT_EQUAL(linphone_chat_room_get_char(pauline_chat_room), message1_2[i], char, "%c");
			}

			/*Commit the message, triggers a NEW LINE in T.140 */
			linphone_chat_message_send(pauline_rtt_message);
			linphone_chat_message_send(marie_rtt_message);

			// Read new line character
			BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
			                              &marie->stat.number_of_LinphoneIsComposingActiveReceived,
			                              (int)strlen(message1_1) + 1, 5000));
			BC_ASSERT_EQUAL(linphone_chat_room_get_char(marie_chat_room), (char)0x2028, char, "%c");
			BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
			                              &pauline->stat.number_of_LinphoneIsComposingActiveReceived,
			                              (int)strlen(message1_2) + 1, 5000));
			BC_ASSERT_EQUAL(linphone_chat_room_get_char(pauline_chat_room), (char)0x2028, char, "%c");

			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
			{
				LinphoneChatMessage *msg = marie->stat.last_received_chat_message;
				BC_ASSERT_PTR_NOT_NULL(msg);
				if (msg) {
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), message1_1);
				}
			}
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageReceived, 1));
			{
				LinphoneChatMessage *msg = pauline->stat.last_received_chat_message;
				BC_ASSERT_PTR_NOT_NULL(msg);
				if (msg) {
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), message1_2);
				}
			}

			linphone_chat_message_unref(pauline_rtt_message);
			linphone_chat_message_unref(marie_rtt_message);
			reset_counters(&pauline->stat);
			reset_counters(&marie->stat);
			pauline_rtt_message = linphone_chat_room_create_message_from_utf8(pauline_chat_room, NULL);
			marie_rtt_message = linphone_chat_room_create_message_from_utf8(marie_chat_room, NULL);

			for (i = 0; i < strlen(message2_1); i++) {
				linphone_chat_message_put_char(pauline_rtt_message, message2_1[i]);
				BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
				                              &marie->stat.number_of_LinphoneIsComposingActiveReceived, (int)i + 1,
				                              5000));
				BC_ASSERT_EQUAL(linphone_chat_room_get_char(marie_chat_room), message2_1[i], char, "%c");

				linphone_chat_message_put_char(marie_rtt_message, message2_2[i]);
				BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
				                              &pauline->stat.number_of_LinphoneIsComposingActiveReceived, (int)i + 1,
				                              5000));
				BC_ASSERT_EQUAL(linphone_chat_room_get_char(pauline_chat_room), message2_2[i], char, "%c");
			}

			/*Commit the message, triggers a NEW LINE in T.140 */
			linphone_chat_message_send(pauline_rtt_message);
			linphone_chat_message_send(marie_rtt_message);

			// Read new line character
			BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
			                              &marie->stat.number_of_LinphoneIsComposingActiveReceived,
			                              (int)strlen(message2_1) + 1, 5000));
			BC_ASSERT_EQUAL(linphone_chat_room_get_char(marie_chat_room), (char)0x2028, char, "%c");
			BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
			                              &pauline->stat.number_of_LinphoneIsComposingActiveReceived,
			                              (int)strlen(message2_2) + 1, 5000));
			BC_ASSERT_EQUAL(linphone_chat_room_get_char(pauline_chat_room), (char)0x2028, char, "%c");

			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
			{
				LinphoneChatMessage *msg = marie->stat.last_received_chat_message;
				BC_ASSERT_PTR_NOT_NULL(msg);
				if (msg) {
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), message2_1);
				}
			}
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageReceived, 1));
			{
				LinphoneChatMessage *msg = pauline->stat.last_received_chat_message;
				BC_ASSERT_PTR_NOT_NULL(msg);
				if (msg) {
					BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), message2_2);
				}
			}
			linphone_chat_message_unref(pauline_rtt_message);
			linphone_chat_message_unref(marie_rtt_message);
		}
		end_call(marie, pauline);
	}
	linphone_call_params_unref(marie_params);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void real_time_text_without_audio(void) {
	real_time_text(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);
}

static void real_time_text_srtp(void) {
	real_time_text(TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);
}

static void real_time_text_ice(void) {
	real_time_text(TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE);
}

static void real_time_text_message_compat(bool_t end_with_crlf, bool_t end_with_lf) {
	LinphoneChatRoom *pauline_chat_room;
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCallParams *marie_params = NULL;
	LinphoneCall *pauline_call, *marie_call;
	LinphoneChatRoom *marie_chat_room;

	marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_realtime_text(marie_params, TRUE);

	BC_ASSERT_TRUE(call_with_caller_params(marie, pauline, marie_params));
	pauline_call = linphone_core_get_current_call(pauline->lc);
	marie_call = linphone_core_get_current_call(marie->lc);
	if (pauline_call) {
		BC_ASSERT_TRUE(linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(pauline_call)));

		pauline_chat_room = linphone_call_get_chat_room(pauline_call);
		marie_chat_room = linphone_call_get_chat_room(marie_call);
		BC_ASSERT_PTR_NOT_NULL(pauline_chat_room);
		BC_ASSERT_PTR_NOT_NULL(marie_chat_room);
		if (pauline_chat_room && marie_chat_room) {
			const char *message = "Be l3l";
			size_t i;
			LinphoneChatMessage *rtt_message = linphone_chat_room_create_message_from_utf8(pauline_chat_room, NULL);
			uint32_t crlf = 0x0D0A;
			uint32_t lf = 0x0A;

			for (i = 0; i < strlen(message); i++) {
				linphone_chat_message_put_char(rtt_message, message[i]);
				BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
				                              &marie->stat.number_of_LinphoneIsComposingActiveReceived, (int)i + 1,
				                              5000));
				BC_ASSERT_EQUAL(linphone_chat_room_get_char(marie_chat_room), message[i], char, "%c");
			}

			if (end_with_crlf) {
				linphone_chat_message_put_char(rtt_message, crlf);
			} else if (end_with_lf) {
				linphone_chat_message_put_char(rtt_message, lf);
			}
			BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
			                              &marie->stat.number_of_LinphoneIsComposingActiveReceived,
			                              (int)strlen(message), 5000));
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
			linphone_chat_message_unref(rtt_message);
		}
		end_call(marie, pauline);
	}
	linphone_call_params_unref(marie_params);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void real_time_text_message_compat_crlf(void) {
	real_time_text_message_compat(TRUE, FALSE);
}

static void real_time_text_message_compat_lf(void) {
	real_time_text_message_compat(FALSE, TRUE);
}

static void real_time_text_message_accented_chars(void) {
	LinphoneChatRoom *pauline_chat_room;
	LinphoneChatRoom *marie_chat_room;
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCallParams *marie_params = NULL;
	LinphoneCall *pauline_call, *marie_call;

	marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_realtime_text(marie_params, TRUE);

	BC_ASSERT_TRUE(call_with_caller_params(marie, pauline, marie_params));
	pauline_call = linphone_core_get_current_call(pauline->lc);
	marie_call = linphone_core_get_current_call(marie->lc);
	if (pauline_call) {
		BC_ASSERT_TRUE(linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(pauline_call)));

		pauline_chat_room = linphone_call_get_chat_room(pauline_call);
		marie_chat_room = linphone_call_get_chat_room(marie_call);
		BC_ASSERT_PTR_NOT_NULL(pauline_chat_room);
		BC_ASSERT_PTR_NOT_NULL(marie_chat_room);
		if (pauline_chat_room && marie_chat_room) {
			LinphoneChatMessage *rtt_message = linphone_chat_room_create_message_from_utf8(pauline_chat_room, NULL);
			uint32_t message[] = {0xe3 /*ã*/, 0xe6 /*æ*/, 0xe7 /*ç*/, 0xe9 /*é*/,   0xee /*î*/,
			                      0xf8 /*ø*/, 0xf9 /*ù*/, 0xff /*ÿ*/, 0x2a7d /*⩽*/, 0x1f600 /*😀*/};
			const int message_len = sizeof(message) / sizeof(uint32_t);
			int i;
			for (i = 0; i < message_len; i++) {
				linphone_chat_message_put_char(rtt_message, message[i]);
				BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
				                              &marie->stat.number_of_LinphoneIsComposingActiveReceived, i + 1, 5000));
				BC_ASSERT_EQUAL(linphone_chat_room_get_char(marie_chat_room), message[i], unsigned long, "%lu");
			}

			linphone_chat_message_send(rtt_message);
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
			BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message);
			if (marie->stat.last_received_chat_message) {
				const char *text = linphone_chat_message_get_text(marie->stat.last_received_chat_message);
				BC_ASSERT_PTR_NOT_NULL(text);
				if (text) BC_ASSERT_STRING_EQUAL(text, "ãæçéîøùÿ⩽😀");
			}
			linphone_chat_message_unref(rtt_message);
		}
		end_call(marie, pauline);
	}
	linphone_call_params_unref(marie_params);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void real_time_text_and_early_media(void) {
	LinphoneChatRoom *pauline_chat_room;
	LinphoneChatRoom *marie_chat_room;
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCallParams *marie_params = NULL, *pauline_params = NULL;
	LinphoneCall *pauline_call, *marie_call;

	marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_realtime_text(marie_params, TRUE);
	linphone_call_params_enable_early_media_sending(marie_params, TRUE);

	linphone_core_invite_address_with_params(marie->lc, pauline->identity, marie_params);
	linphone_call_params_unref(marie_params);

	if (!BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1))) {
		goto end;
	}

	pauline_call = linphone_core_get_current_call(pauline->lc);
	marie_call = linphone_core_get_current_call(marie->lc);
	pauline_params = linphone_core_create_call_params(pauline->lc, pauline_call);
	linphone_call_params_enable_realtime_text(pauline_params, TRUE);
	linphone_call_accept_early_media_with_params(pauline_call, pauline_params);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallIncomingEarlyMedia, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallOutgoingEarlyMedia, 1));

	BC_ASSERT_TRUE(linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(pauline_call)));

	pauline_chat_room = linphone_call_get_chat_room(pauline_call);
	marie_chat_room = linphone_call_get_chat_room(marie_call);
	BC_ASSERT_PTR_NOT_NULL(pauline_chat_room);
	BC_ASSERT_PTR_NOT_NULL(marie_chat_room);
	if (pauline_chat_room && marie_chat_room) {
		LinphoneChatMessage *rtt_message = linphone_chat_room_create_message_from_utf8(pauline_chat_room, NULL);
		int i;
		uint32_t message[8];
		int message_len = 8;
		int chars_received;

		message[0] = 0xE3; // ã
		message[1] = 0xE6; // æ
		message[2] = 0xE7; // ç
		message[3] = 0xE9; // é
		message[4] = 0xEE; // î
		message[5] = 0xF8; // ø
		message[6] = 0xF9; // ù
		message[7] = 0xFF; // ÿ
		for (i = 0; i < message_len; i++) {
			linphone_chat_message_put_char(rtt_message, message[i]);
			BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
			                              &marie->stat.number_of_LinphoneIsComposingActiveReceived, i + 1, 5000));
			BC_ASSERT_EQUAL(linphone_chat_room_get_char(marie_chat_room), message[i], unsigned long, "%lu");
		}

		linphone_chat_message_send(rtt_message);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
		BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message);
		if (marie->stat.last_received_chat_message) {
			const char *text = linphone_chat_message_get_text(marie->stat.last_received_chat_message);
			BC_ASSERT_PTR_NOT_NULL(text);
			if (text) BC_ASSERT_STRING_EQUAL(text, "ãæçéîøùÿ");
		}
		linphone_chat_message_unref(rtt_message);

		/* Disable audio when accepting the call, so that we force a restart of the streams. */
		linphone_call_params_enable_audio(pauline_params, FALSE);
		linphone_call_accept_with_params(pauline_call, pauline_params);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
		linphone_call_params_unref(pauline_params);
		chars_received = marie->stat.number_of_LinphoneIsComposingActiveReceived;

		// Read new line character
		BC_ASSERT_EQUAL(linphone_chat_room_get_char(marie_chat_room), (char)0x2028, char, "%c");

		/* Send RTT again once the call is established. */
		rtt_message = linphone_chat_room_create_message_from_utf8(pauline_chat_room, NULL);
		for (i = 0; i < message_len; i++) {
			linphone_chat_message_put_char(rtt_message, message[i]);
			BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
			                              &marie->stat.number_of_LinphoneIsComposingActiveReceived,
			                              chars_received + i + 1, 5000));
			BC_ASSERT_EQUAL(linphone_chat_room_get_char(marie_chat_room), message[i], unsigned long, "%lu");
		}

		linphone_chat_message_send(rtt_message);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
		BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message);
		if (marie->stat.last_received_chat_message) {
			const char *text = linphone_chat_message_get_text(marie->stat.last_received_chat_message);
			BC_ASSERT_PTR_NOT_NULL(text);
			if (text) BC_ASSERT_STRING_EQUAL(text, "ãæçéîøùÿ");
		}
		linphone_chat_message_unref(rtt_message);
	}
	end_call(marie, pauline);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void only_real_time_text_accepted(void) {
	LinphoneChatRoom *pauline_chat_room;
	LinphoneChatRoom *marie_chat_room;
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCallParams *marie_params = NULL, *pauline_params = NULL;
	LinphoneCall *pauline_call, *marie_call;
	LinphoneChatMessage *rtt_message;
	int chars_received;
	int i;
	uint32_t message[8] = {0xE3, 0xE6, 0xE7, 0xE9, 0xEE, 0xF8, 0xF9, 0xFF};
	int message_len = 8;

	marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_realtime_text(marie_params, TRUE);

	linphone_core_invite_address_with_params(marie->lc, pauline->identity, marie_params);
	linphone_call_params_unref(marie_params);

	if (!BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallIncomingReceived, 1))) {
		goto end;
	}

	pauline_call = linphone_core_get_current_call(pauline->lc);
	marie_call = linphone_core_get_current_call(marie->lc);
	pauline_params = linphone_core_create_call_params(pauline->lc, pauline_call);
	linphone_call_params_enable_audio(pauline_params, FALSE);
	linphone_call_params_enable_video(pauline_params, FALSE);
	linphone_call_params_enable_realtime_text(pauline_params, TRUE);

	linphone_call_accept_with_params(pauline_call, pauline_params);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));

	pauline_chat_room = linphone_call_get_chat_room(pauline_call);
	marie_chat_room = linphone_call_get_chat_room(marie_call);
	BC_ASSERT_PTR_NOT_NULL(pauline_chat_room);
	BC_ASSERT_PTR_NOT_NULL(marie_chat_room);
	if (pauline_chat_room && marie_chat_room) {

		linphone_call_params_unref(pauline_params);
		chars_received = marie->stat.number_of_LinphoneIsComposingActiveReceived;

		/* Send RTT again once the call is established. */
		rtt_message = linphone_chat_room_create_message_from_utf8(pauline_chat_room, NULL);
		for (i = 0; i < message_len; i++) {
			linphone_chat_message_put_char(rtt_message, message[i]);
			BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
			                              &marie->stat.number_of_LinphoneIsComposingActiveReceived,
			                              chars_received + i + 1, 5000));
			BC_ASSERT_EQUAL(linphone_chat_room_get_char(marie_chat_room), message[i], unsigned long, "%lu");
		}

		linphone_chat_message_send(rtt_message);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
		BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message);
		if (marie->stat.last_received_chat_message) {
			const char *text = linphone_chat_message_get_text(marie->stat.last_received_chat_message);
			BC_ASSERT_PTR_NOT_NULL(text);
			if (text) BC_ASSERT_STRING_EQUAL(text, "ãæçéîøùÿ");
		}
		linphone_chat_message_unref(rtt_message);
	}
	end_call(marie, pauline);

end:
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void real_time_text_message_different_text_codecs_payload_numbers_sender_side(void) {
	real_time_text(FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
}

static void real_time_text_message_different_text_codecs_payload_numbers_receiver_side(void) {
	real_time_text(FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE);
}

static void real_time_text_copy_paste(void) {
	LinphoneChatRoom *pauline_chat_room;
	LinphoneChatRoom *marie_chat_room;
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCallParams *marie_params = NULL;
	LinphoneCall *pauline_call, *marie_call;

	marie_params = linphone_core_create_call_params(marie->lc, NULL);
	linphone_call_params_enable_realtime_text(marie_params, TRUE);

	BC_ASSERT_TRUE(call_with_caller_params(marie, pauline, marie_params));
	pauline_call = linphone_core_get_current_call(pauline->lc);
	marie_call = linphone_core_get_current_call(marie->lc);
	if (pauline_call) {
		BC_ASSERT_TRUE(linphone_call_params_realtime_text_enabled(linphone_call_get_current_params(pauline_call)));

		pauline_chat_room = linphone_call_get_chat_room(pauline_call);
		marie_chat_room = linphone_call_get_chat_room(marie_call);
		BC_ASSERT_PTR_NOT_NULL(pauline_chat_room);
		BC_ASSERT_PTR_NOT_NULL(marie_chat_room);
		if (pauline_chat_room && marie_chat_room) {
			const char *message = "Be l3l";
			size_t i;
			LinphoneChatMessage *rtt_message = linphone_chat_room_create_message_from_utf8(pauline_chat_room, NULL);

			for (i = 1; i <= strlen(message); i++) {
				linphone_chat_message_put_char(rtt_message, message[i - 1]);
				if (i % 4 == 0) {
					int j;
					BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
					                              &marie->stat.number_of_LinphoneIsComposingActiveReceived, (int)i,
					                              5000));
					for (j = 4; j > 0; j--) {
						BC_ASSERT_EQUAL(linphone_chat_room_get_char(marie_chat_room), message[i - j], char, "%c");
					}
				}
			}
			linphone_chat_message_send(rtt_message);
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
			linphone_chat_message_unref(rtt_message);
		}

		end_call(marie, pauline);
	}
	linphone_call_params_unref(marie_params);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void file_transfer_with_http_proxy(void) {
	if (transport_supported(LinphoneTransportTls)) {
		LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
		LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "deliver_imdn", 1);
		linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "deliver_imdn", 1);
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie->lc));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline->lc));
		linphone_core_set_http_proxy_host(marie->lc, "http-proxy.example.org");
		transfer_message_base2(marie, pauline, FALSE, FALSE, FALSE, FALSE, FALSE, -1, FALSE, FALSE);
		linphone_core_manager_destroy(pauline);
		linphone_core_manager_destroy(marie);
	}
}

void chat_message_custom_headers(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneChatRoom *chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	LinphoneChatMessage *msg = linphone_chat_room_create_message_from_utf8(chat_room, "Lorem Ipsum");
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);

	linphone_chat_message_add_custom_header(msg, "Test1", "Value1");
	linphone_chat_message_add_custom_header(msg, "Test2", "Value2");
	linphone_chat_message_remove_custom_header(msg, "Test1");

	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDelivered, 1));

	if (marie->stat.last_received_chat_message) {
		const char *header = linphone_chat_message_get_custom_header(marie->stat.last_received_chat_message, "Test2");
		BC_ASSERT_STRING_EQUAL(header, "Value2");
		header = linphone_chat_message_get_custom_header(marie->stat.last_received_chat_message, "Test1");
		BC_ASSERT_PTR_NULL(header);
		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message), "Lorem Ipsum");
	}

	linphone_chat_message_unref(msg);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void _text_message_with_custom_content_type(bool_t is_supported) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneChatRoom *chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	LinphoneChatMessage *msg;
	LinphoneChatMessageCbs *cbs;
	bctbx_vfs_t *vfs = bctbx_vfs_get_default();
	char *send_filepath;
	bctbx_vfs_file_t *file_to_send;
	size_t file_size;
	char *buf;

	linphone_core_add_content_type_support(pauline->lc, "image/svg+xml");
	if (is_supported) {
		linphone_core_add_content_type_support(marie->lc, "image/svg+xml");
	}

	send_filepath = bc_tester_res("images/linphone.svg");
	file_to_send = bctbx_file_open(vfs, send_filepath, "r");
	file_size = (size_t)bctbx_file_size(file_to_send);
	buf = bctbx_malloc(file_size + 1);
	bctbx_file_read(file_to_send, buf, file_size, 0);
	buf[file_size] = '\0';
	bctbx_file_close(file_to_send);
	bctbx_free(send_filepath);
	msg = linphone_chat_room_create_message(chat_room, buf);
	linphone_chat_message_set_content_type(msg, "image/svg+xml");

	cbs = linphone_chat_message_get_callbacks(msg);
	linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
	linphone_chat_message_send(msg);

	if (is_supported) {
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDelivered, 1));
	} else {
		BC_ASSERT_FALSE(
		    wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1, 5000));
		BC_ASSERT_FALSE(
		    wait_for_until(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageDelivered, 1, 5000));
	}

	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(msg), "image/svg+xml");
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(msg), buf);

	if (is_supported) {
		if (marie->stat.last_received_chat_message) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_content_type(marie->stat.last_received_chat_message),
			                       "image/svg+xml");
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message), buf);
		}
	} else {
		BC_ASSERT_PTR_NULL(marie->stat.last_received_chat_message);
		LinphoneChatRoom *marie_room = linphone_core_get_chat_room(marie->lc, pauline->identity);
		BC_ASSERT_PTR_NOT_NULL(marie_room);
		if (marie_room) {
			BC_ASSERT_PTR_NULL(linphone_chat_room_get_last_message_in_history(marie_room));
		}
	}

	bctbx_free(buf);
	linphone_chat_message_unref(msg);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void text_message_with_custom_content_type(void) {
	_text_message_with_custom_content_type(TRUE);
}

void text_message_with_unsupported_content_type(void) {
	_text_message_with_custom_content_type(FALSE);
}

static int im_encryption_engine_process_incoming_message_cb(BCTBX_UNUSED(LinphoneImEncryptionEngine *engine),
                                                            BCTBX_UNUSED(LinphoneChatRoom *room),
                                                            LinphoneChatMessage *msg) {
	ms_debug("IM encryption process incoming message with content type %s",
	         linphone_chat_message_get_content_type(msg));
	if (linphone_chat_message_get_content_type(msg)) {
		if (strcmp(linphone_chat_message_get_content_type(msg), "cipher/b64") == 0) {
			size_t b64Size = 0;
			unsigned char *output;
			const char *data = linphone_chat_message_get_text(msg);
			ms_debug("IM encryption process incoming message crypted message is %s", data);
			bctbx_base64_decode(NULL, &b64Size, (unsigned char *)data, strlen(data));
			output = (unsigned char *)ms_malloc(b64Size + 1 + strlen(" is secured by base64")),
			bctbx_base64_decode(output, &b64Size, (unsigned char *)data, strlen(data));
			sprintf((char *)&output[b64Size], "%s",
			        " is secured by base64"); // to check if encryption engine is really called
			linphone_chat_message_set_text(msg, (char *)output);
			ms_debug("IM encryption process incoming message decrypted message is %s", output);
			ms_free(output);
			linphone_chat_message_set_content_type(msg, "text/plain");

			return 0;
		} else if (strcmp(linphone_chat_message_get_content_type(msg), "text/plain") == 0) {
			return -1; // Not encrypted, nothing to do
		} else {
			return 488; // Not acceptable
		}
	}
	return 500;
}

static LinphoneChatMessage *pending_message = NULL;          /*limited to one message at a time */
static LinphoneChatMessage *incoming_pending_message = NULL; /*limited to one message at a time */

static bool_t im_encryption_engine_process_incoming_message_async_impl(LinphoneChatMessage **msg) {
	if (*msg) {
		im_encryption_engine_process_incoming_message_cb(NULL, NULL, *msg);
		linphone_chat_room_receive_chat_message(linphone_chat_message_get_chat_room(*msg), *msg);
		linphone_chat_message_unref(*msg);
		incoming_pending_message = NULL;
		*msg = NULL;
	}
	return TRUE;
}

static int im_encryption_engine_process_outgoing_message_cb(BCTBX_UNUSED(LinphoneImEncryptionEngine *engine),
                                                            BCTBX_UNUSED(LinphoneChatRoom *room),
                                                            LinphoneChatMessage *msg) {
	if (strcmp(linphone_chat_message_get_content_type(msg), "text/plain") == 0) {
		size_t b64Size = 0;
		unsigned char *output;
		bctbx_base64_encode(NULL, &b64Size, (unsigned char *)linphone_chat_message_get_text(msg),
		                    strlen(linphone_chat_message_get_text(msg)));
		output = (unsigned char *)ms_malloc0(b64Size + 1),
		bctbx_base64_encode(output, &b64Size, (unsigned char *)linphone_chat_message_get_text(msg),
		                    strlen(linphone_chat_message_get_text(msg)));
		output[b64Size] = '\0';
		linphone_chat_message_set_text(msg, (const char *)output);
		ms_free(output);
		linphone_chat_message_set_content_type(msg, "cipher/b64");
		return 0;
	}
	return -1;
}

static bool_t im_encryption_engine_process_outgoing_message_async_impl(LinphoneChatMessage **msg) {
	if (*msg) {
		im_encryption_engine_process_outgoing_message_cb(NULL, NULL, *msg);
		linphone_chat_message_send(*msg);
		linphone_chat_message_unref(*msg);
		*msg = NULL;
	}
	return TRUE;
}

static int im_encryption_engine_process_outgoing_message_async(BCTBX_UNUSED(LinphoneImEncryptionEngine *engine),
                                                               BCTBX_UNUSED(LinphoneChatRoom *room),
                                                               LinphoneChatMessage *msg) {
	if (strcmp(linphone_chat_message_get_content_type(msg), "cipher/b64") == 0) return 0; // already ciphered;
	pending_message = msg;
	linphone_chat_message_ref(pending_message);
	linphone_core_add_iterate_hook(linphone_chat_room_get_core(room),
	                               (LinphoneCoreIterateHook)im_encryption_engine_process_outgoing_message_async_impl,
	                               &pending_message);
	return 1; /*temporaly code to defer message sending*/
}

static int im_encryption_engine_process_incoming_message_async(BCTBX_UNUSED(LinphoneImEncryptionEngine *engine),
                                                               LinphoneChatRoom *room,
                                                               LinphoneChatMessage *msg) {
	incoming_pending_message = msg;
	linphone_chat_message_ref(incoming_pending_message);
	linphone_core_add_iterate_hook(linphone_chat_room_get_core(room),
	                               (LinphoneCoreIterateHook)im_encryption_engine_process_incoming_message_async_impl,
	                               &incoming_pending_message);
	return 1; /*temporaly code to defer message receiving*/
}

void im_encryption_engine_b64_base(bool_t async) {
	LinphoneChatMessage *chat_msg = NULL;
	LinphoneChatRoom *chat_room = NULL;
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneImEncryptionEngine *marie_imee = linphone_im_encryption_engine_new();
	LinphoneImEncryptionEngineCbs *marie_cbs = linphone_im_encryption_engine_get_callbacks(marie_imee);
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneImEncryptionEngine *pauline_imee = linphone_im_encryption_engine_new();
	LinphoneImEncryptionEngineCbs *pauline_cbs = linphone_im_encryption_engine_get_callbacks(pauline_imee);

	linphone_im_encryption_engine_cbs_set_process_outgoing_message(marie_cbs,
	                                                               im_encryption_engine_process_outgoing_message_cb);
	linphone_im_encryption_engine_cbs_set_process_incoming_message(pauline_cbs,
	                                                               im_encryption_engine_process_incoming_message_cb);
	if (async) {
		linphone_im_encryption_engine_cbs_set_process_outgoing_message(
		    pauline_cbs, im_encryption_engine_process_outgoing_message_async);
		linphone_im_encryption_engine_cbs_set_process_incoming_message(
		    marie_cbs, im_encryption_engine_process_incoming_message_async);
	} else {
		linphone_im_encryption_engine_cbs_set_process_outgoing_message(
		    pauline_cbs, im_encryption_engine_process_outgoing_message_cb);
		linphone_im_encryption_engine_cbs_set_process_incoming_message(
		    marie_cbs, im_encryption_engine_process_incoming_message_cb);
	}

	linphone_core_set_im_encryption_engine(marie->lc, marie_imee);
	linphone_core_set_im_encryption_engine(pauline->lc, pauline_imee);

	chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);
	chat_msg = linphone_chat_room_create_message_from_utf8(chat_room, "Bla bla bla bla");
	linphone_chat_message_send(chat_msg);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
	BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(chat_msg), "Bla bla bla bla");
	BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message);
	if (marie->stat.last_received_chat_message) {
		// firt check if message raw message is base64

		BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text(marie->stat.last_received_chat_message),
		                       "Bla bla bla bla is secured by base64");
	}
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_chat_room(marie->lc, pauline->identity));

	linphone_chat_message_unref(chat_msg);
	linphone_im_encryption_engine_unref(marie_imee);
	linphone_im_encryption_engine_unref(pauline_imee);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void im_encryption_engine_b64(void) {
	im_encryption_engine_b64_base(FALSE);
}

void im_encryption_engine_b64_async(void) {
	im_encryption_engine_b64_base(TRUE);
}

void unread_message_count_base(bool_t mute_chat_room) {
	if (!linphone_factory_is_database_storage_available(linphone_factory_get())) {
		ms_warning("Test skipped, database storage is not available");
		return;
	}

	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	text_message_base(marie, pauline);

	BC_ASSERT_PTR_NOT_NULL(marie->stat.last_received_chat_message);
	if (marie->stat.last_received_chat_message != NULL) {
		LinphoneChatRoom *marie_room = linphone_chat_message_get_chat_room(marie->stat.last_received_chat_message);
		BC_ASSERT_FALSE(linphone_chat_room_get_muted(marie_room));
		if (mute_chat_room) {
			linphone_chat_room_set_muted(marie_room, TRUE);
			BC_ASSERT_TRUE(linphone_chat_room_get_muted(marie_room));
		}

		BC_ASSERT_FALSE(linphone_chat_room_is_empty(marie_room));
		if (mute_chat_room) {
			BC_ASSERT_EQUAL(
			    linphone_account_get_unread_chat_message_count(linphone_core_get_default_account(marie->lc)), 0, int,
			    "%d");
		} else {
			BC_ASSERT_EQUAL(
			    linphone_account_get_unread_chat_message_count(linphone_core_get_default_account(marie->lc)), 1, int,
			    "%d");
		}
		BC_ASSERT_EQUAL(linphone_chat_room_get_unread_messages_count(marie_room), 1, int, "%d");
		BC_ASSERT_EQUAL(linphone_core_get_unread_chat_message_count(marie->lc), 1, int, "%d");
		if (mute_chat_room) {
			BC_ASSERT_EQUAL(linphone_core_get_unread_chat_message_count_from_active_locals(marie->lc), 0, int, "%d");
			BC_ASSERT_EQUAL(linphone_core_get_unread_chat_message_count_from_local(
			                    marie->lc, linphone_chat_room_get_local_address(marie_room)),
			                0, int, "%d");
		} else {
			BC_ASSERT_EQUAL(linphone_core_get_unread_chat_message_count_from_active_locals(marie->lc), 1, int, "%d");
			BC_ASSERT_EQUAL(linphone_core_get_unread_chat_message_count_from_local(
			                    marie->lc, linphone_chat_room_get_local_address(marie_room)),
			                1, int, "%d");
		}

		linphone_chat_room_mark_as_read(marie_room);

		BC_ASSERT_FALSE(linphone_chat_room_is_empty(marie_room));
		BC_ASSERT_EQUAL(linphone_account_get_unread_chat_message_count(linphone_core_get_default_account(marie->lc)), 0,
		                int, "%d");
		BC_ASSERT_EQUAL(linphone_chat_room_get_unread_messages_count(marie_room), 0, int, "%d");
		BC_ASSERT_EQUAL(linphone_core_get_unread_chat_message_count(marie->lc), 0, int, "%d");
		BC_ASSERT_EQUAL(linphone_core_get_unread_chat_message_count_from_active_locals(marie->lc), 0, int, "%d");
		BC_ASSERT_EQUAL(linphone_core_get_unread_chat_message_count_from_local(
		                    marie->lc, linphone_chat_room_get_local_address(marie_room)),
		                0, int, "%d");
	}

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void unread_message_count(void) {
	unread_message_count_base(FALSE);
}

void unread_message_count_when_muted(void) {
	unread_message_count_base(TRUE);
}

static void message_received_callback(LinphoneCore *lc, LinphoneChatRoom *room, LinphoneChatMessage *msg) {
	BC_ASSERT_PTR_NOT_NULL(room);
	BC_ASSERT_FALSE(linphone_chat_room_is_empty(room));
	BC_ASSERT_TRUE(linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
	BC_ASSERT_EQUAL(1, linphone_chat_room_get_unread_messages_count(room), int, "%d");
	BC_ASSERT_EQUAL(1, linphone_core_get_unread_chat_message_count(lc), int, "%d");
	BC_ASSERT_EQUAL(1, linphone_core_get_unread_chat_message_count_from_active_locals(lc), int, "%d");
	BC_ASSERT_EQUAL(
	    1, linphone_core_get_unread_chat_message_count_from_local(lc, linphone_chat_room_get_local_address(room)), int,
	    "%d");
	BC_ASSERT_PTR_NOT_NULL(msg);
	if (room != NULL) {
		linphone_chat_room_mark_as_read(room);
	}
	BC_ASSERT_FALSE(linphone_chat_room_is_empty(room));
	BC_ASSERT_EQUAL(0, linphone_chat_room_get_unread_messages_count(room), int, "%d");
	BC_ASSERT_EQUAL(0, linphone_core_get_unread_chat_message_count(lc), int, "%d");
	BC_ASSERT_EQUAL(0, linphone_core_get_unread_chat_message_count_from_active_locals(lc), int, "%d");
	BC_ASSERT_EQUAL(
	    0, linphone_core_get_unread_chat_message_count_from_local(lc, linphone_chat_room_get_local_address(room)), int,
	    "%d");
}

void unread_message_count_callback(void) {
	if (!linphone_factory_is_database_storage_available(linphone_factory_get())) {
		ms_warning("Test skipped, database storage is not available");
		return;
	}

	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	int dummy = 0;

	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_message_received(cbs, message_received_callback);
	linphone_core_add_callbacks(marie->lc, cbs);

	text_message_base(marie, pauline);

	wait_for_until(pauline->lc, marie->lc, &dummy, 1, 5000);

	linphone_core_cbs_unref(cbs);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void migration_from_messages_db(void) {
	if (!linphone_factory_is_database_storage_available(linphone_factory_get())) {
		ms_warning("Test skipped, database storage is not available");
		return;
	}

	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	char *src_db = bc_tester_res("db/messages.db");
	char *tmp_db = random_filepath("tmp", "db");
	BC_ASSERT_EQUAL(liblinphone_tester_copy_file(src_db, tmp_db), 0, int, "%d");

	// The messages.db has 10000 dummy messages with the very first DB scheme.
	// This will test the migration procedure
	linphone_core_set_chat_database_path(marie->lc, tmp_db);

	const bctbx_list_t *chatrooms = linphone_core_get_chat_rooms(marie->lc);
	BC_ASSERT(bctbx_list_size(chatrooms) > 0);

	linphone_core_manager_destroy(marie);
	remove(tmp_db);
	bctbx_free(src_db);
	bctbx_free(tmp_db);
}

static void received_messages_with_aggregation_enabled(void) {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");

	/* enable chat messages aggregation */
	linphone_config_set_bool(linphone_core_get_config(pauline->lc), "sip", "chat_messages_aggregation", TRUE);
	linphone_config_set_bool(linphone_core_get_config(marie->lc), "sip", "chat_messages_aggregation", TRUE);
	linphone_config_set_int(linphone_core_get_config(pauline->lc), "sip", "chat_messages_aggregation_delay", 2000);
	linphone_config_set_int(linphone_core_get_config(marie->lc), "sip", "chat_messages_aggregation_delay", 2000);

	/* create a chatroom on pauline's side */
	LinphoneChatRoom *chat_room = linphone_core_get_chat_room(pauline->lc, marie->identity);

	LinphoneChatMessage *chat_msg = linphone_chat_room_create_message_from_utf8(chat_room, "Bla bla bla bla");
	linphone_chat_message_send(chat_msg);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageSent, 1));

	// check message is received using only new callback when chat room is being created
	BC_ASSERT_TRUE(
	    wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneAggregatedMessagesReceived, 1, 5000));
	BC_ASSERT_FALSE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1, 3000));

	linphone_chat_message_unref(chat_msg);
	reset_counters(&pauline->stat);
	reset_counters(&marie->stat);

	chat_msg = linphone_chat_room_create_message_from_utf8(chat_room, "Blu blu blu blu");
	linphone_chat_message_send(chat_msg);
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageSent, 1));

	// check message is received using only new callback for existing chat room
	BC_ASSERT_TRUE(
	    wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneAggregatedMessagesReceived, 1, 5000));
	BC_ASSERT_FALSE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1, 3000));

	linphone_chat_message_unref(chat_msg);
	reset_counters(&pauline->stat);
	reset_counters(&marie->stat);

	for (int i = 0; i < 10; i++) {
		LinphoneChatMessage *chat_msg = linphone_chat_room_create_message_from_utf8(chat_room, "Bla bli bla blu");
		linphone_chat_message_send(chat_msg);
		BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneMessageSent, i + 1));
		linphone_chat_message_unref(chat_msg);
	}

	// check messages are received using only new callback when more than one message is being received in less than
	// aggregation delay seconds
	BC_ASSERT_TRUE(
	    wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneAggregatedMessagesReceived, 10, 5000));
	BC_ASSERT_FALSE(wait_for_until(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 10, 3000));

	// Give some time for IMDN's 200 OK to be received so it doesn't leak
	wait_for_until(pauline->lc, marie->lc, NULL, 0, 1000);

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

#ifdef HAVE_BAUDOT

#define BAUDOT_DETECTED_CB_BODY(mode)                                                                                  \
	LinphoneCore *core = linphone_call_get_core(call);                                                                 \
	LinphoneCoreManager *manager = (LinphoneCoreManager *)linphone_core_get_user_data(core);                           \
	manager->stat.number_of_LinphoneBaudotDetected++;                                                                  \
	linphone_call_set_baudot_mode(call, mode);                                                                         \
	switch (standard) {                                                                                                \
		case LinphoneBaudotStandardUs:                                                                                 \
			manager->stat.number_of_LinphoneBaudotUsDetected++;                                                        \
			break;                                                                                                     \
		case LinphoneBaudotStandardEurope:                                                                             \
			manager->stat.number_of_LinphoneBaudotEuropeDetected++;                                                    \
			break;                                                                                                     \
	}

static void baudot_detected_cb_tty(LinphoneCall *call, LinphoneBaudotStandard standard) {
	BAUDOT_DETECTED_CB_BODY(LinphoneBaudotModeTty)
}

static void baudot_detected_cb_voice_carryover(LinphoneCall *call, LinphoneBaudotStandard standard) {
	BAUDOT_DETECTED_CB_BODY(LinphoneBaudotModeVoiceCarryOver)
}

static void baudot_text_message(LinphoneBaudotMode initial_sender_baudot_mode,
                                LinphoneBaudotStandard initial_sender_baudot_standard,
                                LinphoneBaudotMode initial_receiver_baudot_mode,
                                LinphoneBaudotStandard initial_receiver_baudot_standard,
                                LinphoneBaudotMode mode_to_switch_to_on_detection) {
	LinphoneChatRoom *pauline_chat_room;
	LinphoneChatRoom *marie_chat_room;
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new("pauline_tcp_rc");
	LinphoneCallParams *marie_params = NULL;
	LinphoneCall *pauline_call, *marie_call;
	char *marie_db = bc_tester_file("marie.db");
	char *pauline_db = bc_tester_file("pauline.db");

	linphone_core_enable_baudot(marie->lc, TRUE);
	linphone_core_enable_baudot(pauline->lc, TRUE);

	marie_params = linphone_core_create_call_params(marie->lc, NULL);

	BC_ASSERT_TRUE(call_with_caller_params(marie, pauline, marie_params));
	pauline_call = linphone_core_get_current_call(pauline->lc);
	marie_call = linphone_core_get_current_call(marie->lc);
	if (pauline_call && marie_call) {
		LinphoneCallCbs *marie_call_cbs = linphone_factory_create_call_cbs(linphone_factory_get());
		linphone_call_cbs_set_baudot_detected(marie_call_cbs,
		                                      (mode_to_switch_to_on_detection == LinphoneBaudotModeVoiceCarryOver)
		                                          ? baudot_detected_cb_voice_carryover
		                                          : baudot_detected_cb_tty);
		linphone_call_add_callbacks(marie_call, marie_call_cbs);
		linphone_call_cbs_unref(marie_call_cbs);
		linphone_call_set_baudot_mode(pauline_call, initial_sender_baudot_mode);
		linphone_call_set_baudot_mode(marie_call, initial_receiver_baudot_mode);
		linphone_call_set_baudot_sending_standard(pauline_call, initial_sender_baudot_standard);
		linphone_call_set_baudot_sending_standard(marie_call, initial_receiver_baudot_standard);

		pauline_chat_room = linphone_call_get_chat_room(pauline_call);
		marie_chat_room = linphone_call_get_chat_room(marie_call);
		BC_ASSERT_PTR_NOT_NULL(pauline_chat_room);
		BC_ASSERT_PTR_NOT_NULL(marie_chat_room);
		if (pauline_chat_room && marie_chat_room) {
			const char *message = "Be l3l";
			size_t i;
			LinphoneChatMessage *chat_message = linphone_chat_room_create_message_from_utf8(pauline_chat_room, NULL);

			for (i = 0; i < strlen(message); i++) {
				BC_ASSERT_FALSE(linphone_chat_message_put_char(chat_message, message[i]));
				BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc,
				                              &marie->stat.number_of_LinphoneIsComposingActiveReceived, (int)i + 1,
				                              3000));
				BC_ASSERT_EQUAL(linphone_chat_room_get_char(marie_chat_room), (char)toupper(message[i]), char, "%c");
			}
			linphone_chat_message_send(chat_message);
			BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneMessageReceived, 1));
			linphone_chat_message_unref(chat_message);
		}

		end_call(marie, pauline);
	}

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneBaudotDetected, 1, int, "%d");
	switch (initial_sender_baudot_standard) {
		case LinphoneBaudotStandardUs:
			BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneBaudotUsDetected, 1, int, "%d");
			BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneBaudotEuropeDetected, 0, int, "%d");
			break;
		case LinphoneBaudotStandardEurope:
			BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneBaudotUsDetected, 0, int, "%d");
			BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneBaudotEuropeDetected, 1, int, "%d");
			break;
	}

	linphone_call_params_unref(marie_params);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
	remove(marie_db);
	bctbx_free(marie_db);
	remove(pauline_db);
	bctbx_free(pauline_db);
}

static void baudot_text_message_us(void) {
	baudot_text_message(LinphoneBaudotModeTty, LinphoneBaudotStandardUs, LinphoneBaudotModeTty,
	                    LinphoneBaudotStandardUs, LinphoneBaudotModeTty);
}

static void baudot_text_message_europe(void) {
	baudot_text_message(LinphoneBaudotModeTty, LinphoneBaudotStandardEurope, LinphoneBaudotModeTty,
	                    LinphoneBaudotStandardEurope, LinphoneBaudotModeTty);
}

static void baudot_text_message_voice_switch_tty_us(void) {
	baudot_text_message(LinphoneBaudotModeTty, LinphoneBaudotStandardUs, LinphoneBaudotModeVoice,
	                    LinphoneBaudotStandardUs, LinphoneBaudotModeTty);
}

static void baudot_text_message_voice_switch_tty_europe(void) {
	baudot_text_message(LinphoneBaudotModeTty, LinphoneBaudotStandardEurope, LinphoneBaudotModeVoice,
	                    LinphoneBaudotStandardEurope, LinphoneBaudotModeTty);
}

static void baudot_text_message_voice_switch_voice_carryover_us(void) {
	baudot_text_message(LinphoneBaudotModeTty, LinphoneBaudotStandardUs, LinphoneBaudotModeVoice,
	                    LinphoneBaudotStandardUs, LinphoneBaudotModeVoiceCarryOver);
}

static void baudot_text_message_voice_switch_voice_carryover_europe(void) {
	baudot_text_message(LinphoneBaudotModeTty, LinphoneBaudotStandardEurope, LinphoneBaudotModeVoice,
	                    LinphoneBaudotStandardEurope, LinphoneBaudotModeVoiceCarryOver);
}

#endif /* HAVE_BAUDOT */

static void crappy_to_header(bool_t existing_chat_room) {
	const char *template = "MESSAGE sip:%s@192.168.0.24:9597 SIP/2.0\r\n"
	                       "Via: SIP/2.0/TCP 10.0.0.1;rport;branch=z9hG4bKSg92gr72NvDUp\r\n"
	                       "Route: <sip:%s@192.168.0.24:9597>;transport=tcp\r\n"
	                       "Max-Forwards: 70\r\n"
	                       "From: <sip:marcel@sip.example.org>;tag=yPe~pl-Rg\r\n"
	                       "To: <sip:%s@192.168.0.24:9597>;transport=tcp\r\n"
	                       "Call-ID: e1431428-4229-4744-9479-e432618ba7f9\r\n"
	                       "CSeq: 95887387 MESSAGE\r\n"
	                       "User-Agent: FreeSWITCH\r\n"
	                       "Allow: INVITE, ACK, BYE, CANCEL, OPTIONS, MESSAGE, INFO, UPDATE, REGISTER, REFER, NOTIFY, "
	                       "PUBLISH, SUBSCRIBE\r\n"
	                       "Supported: path, replaces\r\n"
	                       "Content-Type: text/plain\r\n"
	                       "Content-Length: 2\r\n"
	                       "\r\n"
	                       "Hi\r\n";

	LinphoneCoreManager *laure = linphone_core_manager_new("laure_rc_udp");
	const char *laure_username = linphone_address_get_username(laure->identity);

	if (existing_chat_room) {
		char *localSipUri = bctbx_strdup_printf("sip:%s@192.168.0.24", laure_username);
		LinphoneChatRoom *existing_cr =
		    linphone_core_create_basic_chat_room(laure->lc, localSipUri, "sip:marcel@sip.example.org");
		BC_ASSERT_PTR_NOT_NULL(existing_cr);
		if (existing_cr) {
			const LinphoneAddress *local_address = linphone_chat_room_get_local_address(existing_cr);
			BC_ASSERT_STRING_EQUAL(linphone_address_get_username(local_address), laure_username);
			BC_ASSERT_STRING_EQUAL(linphone_address_get_domain(local_address), "192.168.0.24");

			const LinphoneAddress *peer_address = linphone_chat_room_get_peer_address(existing_cr);
			BC_ASSERT_STRING_EQUAL(linphone_address_get_username(peer_address), "marcel");
			BC_ASSERT_STRING_EQUAL(linphone_address_get_domain(peer_address), "sip.example.org");
		}
		bctbx_free(localSipUri);
	}

	char *message = bctbx_strdup_printf(template, laure_username, laure_username, laure_username);
	LinphoneTransports *tp = linphone_core_get_transports_used(laure->lc);
	BC_ASSERT_TRUE(liblinphone_tester_send_data(message, strlen(message), "127.0.0.1",
	                                            linphone_transports_get_udp_port(tp), SOCK_DGRAM) > 0);
	linphone_transports_unref(tp);
	bctbx_free(message);

	BC_ASSERT_TRUE(wait_for(laure->lc, NULL, &laure->stat.number_of_LinphoneMessageReceived, 1));
	LinphoneChatMessage *received_message = laure->stat.last_received_chat_message;
	BC_ASSERT_PTR_NOT_NULL(received_message);
	if (received_message != NULL) {
		LinphoneChatRoom *chat_room = linphone_chat_message_get_chat_room(received_message);
		BC_ASSERT_PTR_NOT_NULL(chat_room);
		if (chat_room) {
			const LinphoneAddress *local_address = linphone_chat_room_get_local_address(chat_room);
			BC_ASSERT_STRING_EQUAL(linphone_address_get_username(local_address), laure_username);
			BC_ASSERT_STRING_EQUAL(linphone_address_get_domain(local_address), "sip.example.org");

			const LinphoneAddress *peer_address = linphone_chat_room_get_peer_address(chat_room);
			BC_ASSERT_STRING_EQUAL(linphone_address_get_username(peer_address), "marcel");
			BC_ASSERT_STRING_EQUAL(linphone_address_get_domain(peer_address), "sip.example.org");

			LinphoneAccount *account = linphone_chat_room_get_account(chat_room);
			BC_ASSERT_PTR_NOT_NULL(account);
			if (account) {
				const LinphoneAddress *identity_address =
				    linphone_account_params_get_identity_address(linphone_account_get_params(account));
				BC_ASSERT_STRING_EQUAL(linphone_address_get_username(identity_address), laure_username);
				BC_ASSERT_STRING_EQUAL(linphone_address_get_domain(identity_address), "sip.example.org");

				const bctbx_list_t *chat_rooms = linphone_account_get_chat_rooms(account);
				BC_ASSERT_PTR_NOT_NULL(chat_rooms);
				if (chat_rooms) {
					BC_ASSERT_EQUAL((int)bctbx_list_size(chat_rooms), 1, int, "%d");
					LinphoneChatRoom *first_chat_room = (LinphoneChatRoom *)bctbx_list_get_data(chat_rooms);
					LinphoneChatMessage *last_message = linphone_chat_room_get_last_message_in_history(first_chat_room);
					BC_ASSERT_PTR_NOT_NULL(last_message);
					if (last_message) {
						BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_text_content(last_message),
						                       linphone_chat_message_get_text_content(received_message));
						linphone_chat_message_unref(last_message);
					}
				}
			}
		}
	}

	linphone_core_manager_destroy(laure);
}

void text_message_crappy_to_header_existing_chat_room(void) {
	crappy_to_header(TRUE);
}

void text_message_crappy_to_header(void) {
	crappy_to_header(FALSE);
}

static test_t message_tests[] = {
    TEST_NO_TAG("File transfer content", file_transfer_content),
    TEST_NO_TAG("Message with 2 attachments", message_with_two_attachments),
    TEST_NO_TAG("Create two basic chat rooms with same remote", create_two_basic_chat_room_with_same_remote),
    TEST_NO_TAG("Text message", text_message),
    TEST_NO_TAG("Text forward message", text_forward_message),
    TEST_NO_TAG("Text forward message with CPIM enabled with backward compat",
                text_forward_message_cpim_enabled_backward_compat),
    TEST_NO_TAG("Text forward message with CPIM enabled", text_forward_message_cpim_enabled),
    TEST_NO_TAG("Text forward transfer message not downloaded", text_forward_transfer_message_not_downloaded),
    TEST_NO_TAG("Text reply message", text_reply_message),
    TEST_NO_TAG("Text reply message with CPIM enabled with backward compat",
                text_reply_message_cpim_enabled_backward_compat),
    TEST_NO_TAG("Text reply message with CPIM enabled", text_reply_message_cpim_enabled),
    TEST_NO_TAG("Text reaction message", text_reaction_message),
    TEST_NO_TAG("Text reaction message with CPIM enabled with backward compat",
                text_reaction_message_cpim_enabled_backward_compat),
    TEST_NO_TAG("Text reaction message with CPIM enabled", text_reaction_message_cpim_enabled),
    TEST_NO_TAG("Text message UTF8", text_message_with_utf8),
    TEST_NO_TAG("Text message with credentials from auth callback", text_message_with_credential_from_auth_callback),
    TEST_NO_TAG("Text message with privacy", text_message_with_privacy),
    TEST_NO_TAG("Text message compatibility mode", text_message_compatibility_mode),
    TEST_NO_TAG("Text message with ack", text_message_with_ack),
    TEST_NO_TAG("Text message with send error", text_message_with_send_error),
    TEST_NO_TAG("Text message from non default proxy config", text_message_from_non_default_proxy_config),
    TEST_NO_TAG("Text message reply from non default proxy config", text_message_reply_from_non_default_proxy_config),
    TEST_NO_TAG("Text message in call chat room", text_message_in_call_chat_room),
    TEST_NO_TAG("Text message in call chat room from denied text offer",
                text_message_in_call_chat_room_from_denied_text_offer),
    TEST_NO_TAG("Text message in call chat room when room exists", text_message_in_call_chat_room_when_room_exists),
    TEST_NO_TAG("Text message in call chat room from denied text offer when room exists",
                text_message_in_call_chat_room_from_denied_text_offer_when_room_exists),
    TEST_NO_TAG("Text message duplication", text_message_duplication),
    TEST_NO_TAG("Text message auto resent after failure", text_message_auto_resent_after_failure),
    TEST_NO_TAG("Text message with crappy TO header", text_message_crappy_to_header),
    TEST_NO_TAG("Text message with crappy TO header from existing chat room",
                text_message_crappy_to_header_existing_chat_room),
    TEST_NO_TAG("Transfer message", transfer_message),
    TEST_NO_TAG("Transfer message 2", transfer_message_2),
    TEST_NO_TAG("Transfer message 3", transfer_message_3),
    TEST_NO_TAG("Transfer message 4", transfer_message_4),
    TEST_NO_TAG("Message with voice recording", message_with_voice_recording),
    TEST_NO_TAG("Message with voice recording 2", message_with_voice_recording_2),
    TEST_NO_TAG("Message with voice recording 3", message_with_voice_recording_3),
    TEST_NO_TAG("Transfer message legacy", transfer_message_legacy),
    TEST_NO_TAG("Transfer message with 2 files", transfer_message_2_files),
    TEST_NO_TAG("Transfer message auto download", transfer_message_auto_download),
    TEST_NO_TAG("Transfer message auto download 2", transfer_message_auto_download_2),
    TEST_NO_TAG("Transfer message auto download failure", transfer_message_auto_download_failure),
    TEST_NO_TAG("Transfer message auto download enabled but file too large", transfer_message_auto_download_3),
    TEST_NO_TAG("Transfer message auto download existing file", transfer_message_auto_download_existing_file),
    TEST_NO_TAG("Transfer messages same file auto download",
                transfer_message_auto_download_two_files_same_name_same_time),
    TEST_NO_TAG("Transfer message from history", transfer_message_from_history),
    TEST_NO_TAG("Transfer message with http proxy", file_transfer_with_http_proxy),
    TEST_NO_TAG("Transfer message with upload io error", transfer_message_with_upload_io_error),
    TEST_NO_TAG("Transfer message with download io error", transfer_message_with_download_io_error),
    TEST_NO_TAG("Transfer message upload cancelled", transfer_message_upload_cancelled),
    TEST_NO_TAG("Transfer message upload finished during stop", transfer_message_upload_finished_during_stop),
    TEST_NO_TAG("Transfer message download cancelled", transfer_message_download_cancelled),
    TEST_NO_TAG("Transfer message auto download aborted", transfer_message_auto_download_aborted),
    TEST_NO_TAG("Transfer message core stopped async 1", transfer_message_core_stopped_async_1),
    TEST_NO_TAG("Transfer message core stopped async 2", transfer_message_core_stopped_async_2),
    TEST_NO_TAG("Transfer 2 messages simultaneously", file_transfer_2_messages_simultaneously),
    TEST_NO_TAG("Transfer using external body URL", file_transfer_using_external_body_url),
    TEST_NO_TAG("Transfer using external body URL 2", file_transfer_using_external_body_url_2),
    TEST_NO_TAG("Transfer using external body URL 404", file_transfer_using_external_body_url_404),
    TEST_NO_TAG("Transfer message - file transfer server authenticates client using certificate",
                transfer_message_tls_client_auth),
    TEST_NO_TAG("Transfer message - file transfer server authenticates client using digest auth",
                transfer_message_digest_auth),
    TEST_NO_TAG("Transfer message - file transfer server authenticates client using digest auth server accepting "
                "multiple auth domain",
                transfer_message_digest_auth_any_domain),
    TEST_NO_TAG("Transfer message - file transfer server authenticates client using digest auth - upload auth fail",
                transfer_message_digest_auth_fail_up),
    TEST_NO_TAG("Transfer message - file transfer server authenticates client using digest auth - download auth fail",
                transfer_message_digest_auth_fail_down),
    TEST_NO_TAG("Transfer message - file transfer server authenticates client using digest auth server accepting "
                "multiple auth domain - upload auth fail",
                transfer_message_digest_auth_fail_any_domain_up),
    TEST_NO_TAG("Transfer message - file transfer server authenticates client using digest auth server accepting "
                "multiple auth domain - download auth fail",
                transfer_message_digest_auth_fail_any_domain_down),
    TEST_NO_TAG("Transfer message - file size limited pass", transfer_message_small_files_pass),
    TEST_NO_TAG("Transfer message - file size limited fail", transfer_message_small_files_fail),
    TEST_NO_TAG("Transfer message auto resent after failure", transfer_message_auto_resent_after_failure),
    TEST_NO_TAG("Aggregated messages", received_messages_with_aggregation_enabled),
    TEST_NO_TAG("Text message denied", text_message_denied),
#ifdef HAVE_ADVANCED_IM
    TEST_NO_TAG("IsComposing notification", is_composing_notification),
    TEST_NO_TAG("IMDN notifications", imdn_notifications),
    TEST_NO_TAG("IM notification policy", im_notification_policy),
    TEST_NO_TAG("Aggregated IMDNs", aggregated_imdns),
    TEST_NO_TAG("Aggregated IMDNs in group chat", aggregated_imdns_in_group_chat),
#endif
    TEST_NO_TAG("Unread message count", unread_message_count),
    TEST_NO_TAG("Unread message count with muted chat room", unread_message_count_when_muted),
    TEST_NO_TAG("Unread message count in callback", unread_message_count_callback),
    TEST_NO_TAG("Transfer not sent if invalid url", file_transfer_not_sent_if_invalid_url),
    TEST_NO_TAG("Transfer not sent if host not found", file_transfer_not_sent_if_host_not_found),
    TEST_NO_TAG("Transfer not sent if url moved permanently", file_transfer_not_sent_if_url_moved_permanently),
    TEST_NO_TAG("IM Encryption Engine custom headers", chat_message_custom_headers),
    TEST_NO_TAG("Text message with custom content-type", text_message_with_custom_content_type),
    TEST_NO_TAG("Text message with unsupported content-type", text_message_with_unsupported_content_type),
    TEST_NO_TAG("IM Encryption Engine b64", im_encryption_engine_b64),
    TEST_NO_TAG("IM Encryption Engine b64 async", im_encryption_engine_b64_async),
    TEST_NO_TAG("Text message within call dialog", text_message_within_call_dialog),
    TEST_NO_TAG("Info message", info_message),
    TEST_NO_TAG("Info message with body", info_message_with_body),
    TEST_NO_TAG("Crash during file transfer", crash_during_file_transfer),
    TEST_NO_TAG("Text status after destroying chat room", text_status_after_destroying_chat_room),
    TEST_NO_TAG("Transfer success after destroying chatroom", file_transfer_success_after_destroying_chatroom),
    TEST_NO_TAG("Migration from messages db", migration_from_messages_db)};

static test_t rtt_message_tests[] = {
    TEST_ONE_TAG("Real Time Text message", real_time_text_message, "RTT"),
    TEST_ONE_TAG("Real Time Text message with existing basic chat room",
                 real_time_text_message_with_existing_basic_chat_room,
                 "RTT"),
    TEST_ONE_TAG("Real Time Text SQL storage", real_time_text_sql_storage, "RTT"),
    TEST_ONE_TAG(
        "Real Time Text SQL storage with RTT messages not stored", real_time_text_sql_storage_rtt_disabled, "RTT"),
    TEST_ONE_TAG("Real Time Text conversation", real_time_text_conversation, "RTT"),
    TEST_ONE_TAG("Real Time Text without audio", real_time_text_without_audio, "RTT"),
    TEST_ONE_TAG("Only Real Time Text accepted", only_real_time_text_accepted, "RTT"),
    TEST_ONE_TAG("Real Time Text with srtp", real_time_text_srtp, "RTT"),
    TEST_ONE_TAG("Real Time Text with ice", real_time_text_ice, "RTT"),
    TEST_ONE_TAG("Real Time Text message compatibility crlf", real_time_text_message_compat_crlf, "RTT"),
    TEST_ONE_TAG("Real Time Text message compatibility lf", real_time_text_message_compat_lf, "RTT"),
    TEST_ONE_TAG("Real Time Text message with accented characters", real_time_text_message_accented_chars, "RTT"),
    TEST_ONE_TAG("Real Time Text offer answer with different payload numbers (sender side)",
                 real_time_text_message_different_text_codecs_payload_numbers_sender_side,
                 "RTT"),
    TEST_ONE_TAG("Real Time Text offer answer with different payload numbers (receiver side)",
                 real_time_text_message_different_text_codecs_payload_numbers_receiver_side,
                 "RTT"),
    TEST_ONE_TAG("Real Time Text copy paste", real_time_text_copy_paste, "RTT"),
    TEST_ONE_TAG("Real Time Text and early media", real_time_text_and_early_media, "RTT")};

#ifdef HAVE_BAUDOT

static test_t baudot_message_tests[] = {
    TEST_ONE_TAG("Baudot text message US", baudot_text_message_us, "Baudot"),
    TEST_ONE_TAG("Baudot text message Europe", baudot_text_message_europe, "Baudot"),
    TEST_ONE_TAG("Baudot text message voice switch to US TTY", baudot_text_message_voice_switch_tty_us, "Baudot"),
    TEST_ONE_TAG(
        "Baudot text message voice switch to Europe TTY", baudot_text_message_voice_switch_tty_europe, "Baudot"),
    TEST_ONE_TAG("Baudot text message voice switch to US Voice CarryOver",
                 baudot_text_message_voice_switch_voice_carryover_us,
                 "Baudot"),
    TEST_ONE_TAG("Baudot text message voice switch to Europe Voice CarryOver",
                 baudot_text_message_voice_switch_voice_carryover_europe,
                 "Baudot"),
};
#endif /* HAVE_BAUDOT */

static int message_tester_before_suite(void) {
	// liblinphone_tester_keep_uuid = TRUE;

	/*
	 * FIXME: liblinphone does not automatically creates the data directory into which it can write databases, logs etc.
	 * Today it is done by applications (like linphone-desktop) or by the system (ios, android).
	 * This must be solved.
	 * Until this is done, this hack will simply create the directory, for linux only.
	 */
#if defined(__linux__) && !defined(__ANDROID__)
	const char *home = getenv("HOME");
	char *command;
	int err;

	if (!home) home = ".";
	command = bctbx_strdup_printf("mkdir -p %s/.local/share/linphone", home);
	err = system(command);
	if (err != -1 && WIFEXITED(err) && WEXITSTATUS(err) == 0) {
		bctbx_message("%s done successfully.", command);
	} else {
		bctbx_error("%s failed. Some tests may fail.", command);
	}
	bctbx_free(command);
#endif
	return 0;
}

static int message_tester_after_suite(void) {
	// liblinphone_tester_keep_uuid = FALSE;
	return 0;
}

test_suite_t message_test_suite = {"Message",
                                   message_tester_before_suite,
                                   message_tester_after_suite,
                                   liblinphone_tester_before_each,
                                   liblinphone_tester_after_each,
                                   sizeof(message_tests) / sizeof(message_tests[0]),
                                   message_tests,
                                   0};

test_suite_t rtt_message_test_suite = {"RTT Message",
                                       message_tester_before_suite,
                                       message_tester_after_suite,
                                       liblinphone_tester_before_each,
                                       liblinphone_tester_after_each,
                                       sizeof(rtt_message_tests) / sizeof(rtt_message_tests[0]),
                                       rtt_message_tests,
                                       0};

#ifdef HAVE_BAUDOT
test_suite_t baudot_message_test_suite = {"Baudot Message",
                                          message_tester_before_suite,
                                          message_tester_after_suite,
                                          liblinphone_tester_before_each,
                                          liblinphone_tester_after_each,
                                          sizeof(baudot_message_tests) / sizeof(baudot_message_tests[0]),
                                          baudot_message_tests,
                                          0};
#endif /* HAVE_BAUDOT */
