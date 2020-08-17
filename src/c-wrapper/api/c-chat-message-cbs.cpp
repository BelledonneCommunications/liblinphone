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

#include "linphone/api/c-chat-message-cbs.h"

#include "c-wrapper/c-wrapper.h"

// =============================================================================

struct _LinphoneChatMessageCbs {
	belle_sip_object_t base;
	void *userData;
	LinphoneChatMessageCbsMsgStateChangedCb msg_state_changed;
	LinphoneChatMessageCbsFileTransferRecvCb file_transfer_recv;
	LinphoneChatMessageCbsFileTransferSendCb file_transfer_send;
	LinphoneChatMessageCbsFileTransferProgressIndicationCb file_transfer_progress_indication;
	LinphoneChatMessageCbsParticipantImdnStateChangedCb participant_imdn_state_changed;
	LinphoneChatMessageCbsEphemeralMessageTimerStartedCb ephemeral_message_timer_started;
	LinphoneChatMessageCbsEphemeralMessageDeletedCb ephemeral_message_deleted;
	LinphoneChatMessageCbsFileTransferSendChunkCb file_transfer_send_chunk;
};

BELLE_SIP_DECLARE_VPTR_NO_EXPORT(LinphoneChatMessageCbs);

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneChatMessageCbs);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneChatMessageCbs, belle_sip_object_t,
	NULL, // destroy
	NULL, // clone
	NULL, // marshal
	FALSE
);

// =============================================================================

LinphoneChatMessageCbs * linphone_chat_message_cbs_new (void) {
	return belle_sip_object_new(LinphoneChatMessageCbs);
}

LinphoneChatMessageCbs * linphone_chat_message_cbs_ref (LinphoneChatMessageCbs *cbs) {
	belle_sip_object_ref(cbs);
	return cbs;
}

void linphone_chat_message_cbs_unref (LinphoneChatMessageCbs *cbs) {
	belle_sip_object_unref(cbs);
}

void * linphone_chat_message_cbs_get_user_data (const LinphoneChatMessageCbs *cbs) {
	return cbs->userData;
}

void linphone_chat_message_cbs_set_user_data (LinphoneChatMessageCbs *cbs, void *ud) {
	cbs->userData = ud;
}

LinphoneChatMessageCbsMsgStateChangedCb linphone_chat_message_cbs_get_msg_state_changed (
	const LinphoneChatMessageCbs *cbs
) {
	return cbs->msg_state_changed;
}

void linphone_chat_message_cbs_set_msg_state_changed (
	LinphoneChatMessageCbs *cbs,
	LinphoneChatMessageCbsMsgStateChangedCb cb
) {
	cbs->msg_state_changed = cb;
}

LinphoneChatMessageCbsFileTransferRecvCb linphone_chat_message_cbs_get_file_transfer_recv (
	const LinphoneChatMessageCbs *cbs
) {
	return cbs->file_transfer_recv;
}

void linphone_chat_message_cbs_set_file_transfer_recv (
	LinphoneChatMessageCbs *cbs,
	LinphoneChatMessageCbsFileTransferRecvCb cb
) {
	cbs->file_transfer_recv = cb;
}

LinphoneChatMessageCbsFileTransferSendCb linphone_chat_message_cbs_get_file_transfer_send (
	const LinphoneChatMessageCbs *cbs
) {
	return cbs->file_transfer_send;
}

void linphone_chat_message_cbs_set_file_transfer_send (
	LinphoneChatMessageCbs *cbs,
	LinphoneChatMessageCbsFileTransferSendCb cb
) {
	cbs->file_transfer_send = cb;
}

LinphoneChatMessageCbsFileTransferSendChunkCb linphone_chat_message_cbs_get_file_transfer_send_chunk (
	const LinphoneChatMessageCbs *cbs
) {
	return cbs->file_transfer_send_chunk;
}

void linphone_chat_message_cbs_set_file_transfer_send_chunk (
	LinphoneChatMessageCbs *cbs,
	LinphoneChatMessageCbsFileTransferSendChunkCb cb
) {
	cbs->file_transfer_send_chunk = cb;
}

LinphoneChatMessageCbsFileTransferProgressIndicationCb linphone_chat_message_cbs_get_file_transfer_progress_indication (
	const LinphoneChatMessageCbs *cbs
) {
	return cbs->file_transfer_progress_indication;
}

void linphone_chat_message_cbs_set_file_transfer_progress_indication (
	LinphoneChatMessageCbs *cbs,
	LinphoneChatMessageCbsFileTransferProgressIndicationCb cb
) {
	cbs->file_transfer_progress_indication = cb;
}

LinphoneChatMessageCbsParticipantImdnStateChangedCb linphone_chat_message_cbs_get_participant_imdn_state_changed (
	const LinphoneChatMessageCbs *cbs
) {
	return cbs->participant_imdn_state_changed;
}

void linphone_chat_message_cbs_set_participant_imdn_state_changed (
	LinphoneChatMessageCbs *cbs,
	LinphoneChatMessageCbsParticipantImdnStateChangedCb cb
) {
	cbs->participant_imdn_state_changed = cb;
}

LinphoneChatMessageCbsEphemeralMessageTimerStartedCb linphone_chat_message_cbs_get_ephemeral_message_timer_started (
	const LinphoneChatMessageCbs *cbs
) {
	return cbs->ephemeral_message_timer_started;
}

void linphone_chat_message_cbs_set_ephemeral_message_timer_started (
	LinphoneChatMessageCbs *cbs,
	LinphoneChatMessageCbsEphemeralMessageTimerStartedCb cb
) {
	cbs->ephemeral_message_timer_started = cb;
}

LinphoneChatMessageCbsEphemeralMessageDeletedCb linphone_chat_message_cbs_get_ephemeral_message_deleted (
	const LinphoneChatMessageCbs *cbs
) {
	return cbs->ephemeral_message_deleted;
}

void linphone_chat_message_cbs_set_ephemeral_message_deleted (
	LinphoneChatMessageCbs *cbs,
	LinphoneChatMessageCbsEphemeralMessageDeletedCb cb
) {
	cbs->ephemeral_message_deleted = cb;
}
