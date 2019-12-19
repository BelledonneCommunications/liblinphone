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

#include "linphone/api/c-chat-message.h"
#include "linphone/wrapper_utils.h"
#include "linphone/api/c-content.h"
#include "linphone/utils/utils.h"

#include "ortp/b64.h"

#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "chat/chat-message/chat-message-p.h"
#include "chat/chat-room/chat-room-p.h"
#include "chat/chat-room/real-time-text-chat-room-p.h"
#include "chat/notification/imdn.h"
#include "conference/participant-imdn-state.h"
#include "conference/participant.h"
#include "content/content-type.h"
#include "content/content.h"
#include "core/core-p.h"

// =============================================================================

using namespace std;

static void _linphone_chat_message_constructor (LinphoneChatMessage *msg);
static void _linphone_chat_message_destructor (LinphoneChatMessage *msg);

L_DECLARE_C_OBJECT_IMPL_WITH_XTORS(ChatMessage,
	_linphone_chat_message_constructor,
	_linphone_chat_message_destructor,

	LinphoneChatMessageCbs *cbs; // Deprecated, use a list of Cbs instead
	bctbx_list_t *callbacks;
	LinphoneChatMessageCbs *currentCbs;
	LinphoneChatMessageStateChangedCb message_state_changed_cb;
	void *message_state_changed_user_data;

	struct Cache {
		~Cache () {
			if (from)
				linphone_address_unref(from);
			if (to)
				linphone_address_unref(to);
			if (contents)
				bctbx_list_free(contents);
		}

		string contentType;
		string textContentBody;
		string customHeaderValue;

		LinphoneAddress *from = nullptr;
		LinphoneAddress *to = nullptr;

		bctbx_list_t *contents = nullptr;
	} mutable cache;
)

static void _linphone_chat_message_constructor (LinphoneChatMessage *msg) {
	msg->cbs = linphone_chat_message_cbs_new();
	new(&msg->cache) LinphoneChatMessage::Cache();
}

static void _linphone_chat_message_destructor (LinphoneChatMessage *msg) {
	linphone_chat_message_cbs_unref(msg->cbs);
	msg->cbs = nullptr;
	_linphone_chat_message_clear_callbacks(msg);
	msg->cache.~Cache();
}

// =============================================================================
// Reference and user data handling functions.
// =============================================================================

LinphoneChatMessage *linphone_chat_message_ref (LinphoneChatMessage *msg) {
	belle_sip_object_ref(msg);
	return msg;
}

void linphone_chat_message_unref (LinphoneChatMessage *msg) {
	belle_sip_object_unref(msg);
}

void * linphone_chat_message_get_user_data (const LinphoneChatMessage *msg) {
	return L_GET_USER_DATA_FROM_C_OBJECT(msg);
}

void linphone_chat_message_set_user_data (LinphoneChatMessage *msg, void *ud) {
	L_SET_USER_DATA_FROM_C_OBJECT(msg, ud);
}

LinphoneChatMessageCbs *linphone_chat_message_get_callbacks(const LinphoneChatMessage *msg) {
	return msg->cbs;
}

void _linphone_chat_message_clear_callbacks (LinphoneChatMessage *msg) {
	bctbx_list_free_with_data(msg->callbacks, (bctbx_list_free_func)linphone_chat_message_cbs_unref);
	msg->callbacks = nullptr;
}

void linphone_chat_message_add_callbacks(LinphoneChatMessage *msg, LinphoneChatMessageCbs *cbs) {
	msg->callbacks = bctbx_list_append(msg->callbacks, linphone_chat_message_cbs_ref(cbs));
}

void linphone_chat_message_remove_callbacks(LinphoneChatMessage *msg, LinphoneChatMessageCbs *cbs) {
	msg->callbacks = bctbx_list_remove(msg->callbacks, cbs);
	linphone_chat_message_cbs_unref(cbs);
}

LinphoneChatMessageCbs *linphone_chat_message_get_current_callbacks(const LinphoneChatMessage *msg) {
	return msg->currentCbs;
}

void linphone_chat_message_set_current_callbacks(LinphoneChatMessage *msg, LinphoneChatMessageCbs *cbs) {
	msg->currentCbs = cbs;
}

const bctbx_list_t *linphone_chat_message_get_callbacks_list(const LinphoneChatMessage *msg) {
	return msg->callbacks;
}

#define NOTIFY_IF_EXIST(cbName, functionName, ...) \
	bctbx_list_t *callbacksCopy = bctbx_list_copy(linphone_chat_message_get_callbacks_list(msg)); \
	for (bctbx_list_t *it = callbacksCopy; it; it = bctbx_list_next(it)) { \
		linphone_chat_message_set_current_callbacks(msg, reinterpret_cast<LinphoneChatMessageCbs *>(bctbx_list_get_data(it))); \
		LinphoneChatMessageCbs ## cbName ## Cb cb = linphone_chat_message_cbs_get_ ## functionName (linphone_chat_message_get_current_callbacks(msg)); \
		if (cb) \
			cb(__VA_ARGS__); \
	} \
	linphone_chat_message_set_current_callbacks(msg, nullptr); \
	bctbx_list_free(callbacksCopy);

void _linphone_chat_message_notify_msg_state_changed(LinphoneChatMessage *msg, LinphoneChatMessageState state) {
	NOTIFY_IF_EXIST(MsgStateChanged, msg_state_changed, msg, state)
}

void _linphone_chat_message_notify_participant_imdn_state_changed(LinphoneChatMessage* msg, const LinphoneParticipantImdnState *state) {
	NOTIFY_IF_EXIST(ParticipantImdnStateChanged, participant_imdn_state_changed, msg, state)
}

void _linphone_chat_message_notify_file_transfer_recv(LinphoneChatMessage *msg, const LinphoneContent* content, const LinphoneBuffer *buffer) {
	NOTIFY_IF_EXIST(FileTransferRecv, file_transfer_recv, msg, content, buffer)
}

void _linphone_chat_message_notify_file_transfer_send(LinphoneChatMessage *msg,  const LinphoneContent* content, size_t offset, size_t size) {
	NOTIFY_IF_EXIST(FileTransferSend, file_transfer_send, msg, content, offset, size)
}

void _linphone_chat_message_notify_file_transfer_progress_indication(LinphoneChatMessage *msg, const LinphoneContent* content, size_t offset, size_t total) {
	NOTIFY_IF_EXIST(FileTransferProgressIndication, file_transfer_progress_indication, msg, content, offset, total)
}

void _linphone_chat_message_notify_ephemeral_message_timer_started(LinphoneChatMessage* msg) {
	NOTIFY_IF_EXIST(EphemeralMessageTimerStarted, ephemeral_message_timer_started, msg)
}

void _linphone_chat_message_notify_ephemeral_message_deleted(LinphoneChatMessage* msg) {
	NOTIFY_IF_EXIST(EphemeralMessageDeleted, ephemeral_message_deleted, msg)
}

// =============================================================================
// Getter and setters
// =============================================================================

LINPHONE_PUBLIC LinphoneCore *linphone_chat_message_get_core (const LinphoneChatMessage *msg) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(msg)->getCore()->getCCore();
}

LinphoneChatRoom *linphone_chat_message_get_chat_room (const LinphoneChatMessage *msg) {
	return L_GET_C_BACK_PTR(L_GET_CPP_PTR_FROM_C_OBJECT(msg)->getChatRoom());
}

const char *linphone_chat_message_get_external_body_url (const LinphoneChatMessage *msg) {
	return L_STRING_TO_C(L_GET_PRIVATE_FROM_C_OBJECT(msg)->getExternalBodyUrl());
}

void linphone_chat_message_set_external_body_url (LinphoneChatMessage *msg, const char *url) {
	L_GET_PRIVATE_FROM_C_OBJECT(msg)->setExternalBodyUrl(L_C_TO_STRING(url));
}

time_t linphone_chat_message_get_time (const LinphoneChatMessage *msg) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(msg)->getTime();
}

bool_t linphone_chat_message_is_secured (const LinphoneChatMessage *msg) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(msg)->isSecured();
}

bool_t linphone_chat_message_is_outgoing (const LinphoneChatMessage *msg) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(msg)->getDirection() == LinphonePrivate::ChatMessage::Direction::Outgoing;
}

LinphoneChatMessageState linphone_chat_message_get_state (const LinphoneChatMessage *msg) {
	return ((LinphoneChatMessageState)L_GET_CPP_PTR_FROM_C_OBJECT(msg)->getState());
}

const char* linphone_chat_message_get_message_id (const LinphoneChatMessage *msg) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(msg)->getImdnMessageId().c_str();
}

bool_t linphone_chat_message_is_read (const LinphoneChatMessage *msg) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(msg)->isRead();
}

const char *linphone_chat_message_get_appdata (const LinphoneChatMessage *msg) {
	return L_STRING_TO_C(L_GET_PRIVATE_FROM_C_OBJECT(msg)->getAppdata());
}

void linphone_chat_message_set_appdata (LinphoneChatMessage *msg, const char *data) {
	L_GET_PRIVATE_FROM_C_OBJECT(msg)->setAppdata(L_C_TO_STRING(data));
}

const LinphoneAddress *linphone_chat_message_get_from_address (const LinphoneChatMessage *msg) {
	if (msg->cache.from)
		linphone_address_unref(msg->cache.from);
	msg->cache.from = linphone_address_new(L_GET_CPP_PTR_FROM_C_OBJECT(msg)->getFromAddress().asString().c_str());
	return msg->cache.from;
}

const LinphoneAddress *linphone_chat_message_get_to_address (const LinphoneChatMessage *msg) {
	if (msg->cache.to)
		linphone_address_unref(msg->cache.to);
	msg->cache.to = linphone_address_new(L_GET_CPP_PTR_FROM_C_OBJECT(msg)->getToAddress().asString().c_str());
	return msg->cache.to;
}

const char *linphone_chat_message_get_file_transfer_filepath (const LinphoneChatMessage *msg) {
	return L_STRING_TO_C(L_GET_PRIVATE_FROM_C_OBJECT(msg)->getFileTransferFilepath());
}

void linphone_chat_message_set_file_transfer_filepath (LinphoneChatMessage *msg, const char *filepath) {
	L_GET_PRIVATE_FROM_C_OBJECT(msg)->setFileTransferFilepath(L_C_TO_STRING(filepath));
}

bool_t linphone_chat_message_is_forward(LinphoneChatMessage *msg) {
	return !L_GET_CPP_PTR_FROM_C_OBJECT(msg)->getForwardInfo().empty();
}

const char *linphone_chat_message_get_forward_info (const LinphoneChatMessage *msg) {
	return L_STRING_TO_C(L_GET_CPP_PTR_FROM_C_OBJECT(msg)->getForwardInfo());
}

bool_t linphone_chat_message_is_ephemeral (LinphoneChatMessage *msg) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(msg)->isEphemeral();
}

long linphone_chat_message_get_ephemeral_lifetime (LinphoneChatMessage *msg) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(msg)->getEphemeralLifetime();
}

time_t linphone_chat_message_get_ephemeral_expired_time (LinphoneChatMessage *msg) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(msg)->getEphemeralExpiredTime();
}

void linphone_chat_message_add_custom_header(
	LinphoneChatMessage *msg,
	const char *header_name,
	const char *header_value
) {
	L_GET_PRIVATE_FROM_C_OBJECT(msg)->addSalCustomHeader(L_C_TO_STRING(header_name), L_C_TO_STRING(header_value));
}

void linphone_chat_message_remove_custom_header (LinphoneChatMessage *msg, const char *header_name) {
	L_GET_PRIVATE_FROM_C_OBJECT(msg)->removeSalCustomHeader(L_C_TO_STRING(header_name));
}

const char *linphone_chat_message_get_custom_header (const LinphoneChatMessage *msg, const char *header_name) {
	msg->cache.customHeaderValue = L_GET_PRIVATE_FROM_C_OBJECT(msg)->getSalCustomHeaderValue(L_C_TO_STRING(header_name));
	return L_STRING_TO_C(msg->cache.customHeaderValue);
}

const LinphoneErrorInfo *linphone_chat_message_get_error_info (const LinphoneChatMessage *msg) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(msg)->getErrorInfo();
}

bool_t linphone_chat_message_get_to_be_stored (const LinphoneChatMessage *message) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(message)->getToBeStored();
}

void linphone_chat_message_set_to_be_stored (LinphoneChatMessage *message, bool_t to_be_stored) {
	L_GET_CPP_PTR_FROM_C_OBJECT(message)->setToBeStored(!!to_be_stored);
}

// =============================================================================
// Methods
// =============================================================================

void linphone_chat_message_cancel_file_transfer (LinphoneChatMessage *msg) {
	L_GET_CPP_PTR_FROM_C_OBJECT(msg)->cancelFileTransfer();
}

void linphone_chat_message_send (LinphoneChatMessage *msg) {
	L_GET_CPP_PTR_FROM_C_OBJECT(msg)->send();
}

void linphone_chat_message_resend (LinphoneChatMessage *msg) {
	L_GET_CPP_PTR_FROM_C_OBJECT(msg)->send();
}

void linphone_chat_message_resend_2 (LinphoneChatMessage *msg) {
	L_GET_CPP_PTR_FROM_C_OBJECT(msg)->send();
}

LinphoneStatus linphone_chat_message_put_char (LinphoneChatMessage *msg, uint32_t character) {
	return ((LinphoneStatus)L_GET_CPP_PTR_FROM_C_OBJECT(msg)->putCharacter(character));
}

void linphone_chat_message_add_file_content (LinphoneChatMessage *msg, LinphoneContent *c_content) {
	LinphonePrivate::FileContent *fileContent = new LinphonePrivate::FileContent();
	LinphonePrivate::ContentType contentType;
	contentType.setType(L_C_TO_STRING(linphone_content_get_type(c_content)));
	contentType.setSubType(L_C_TO_STRING(linphone_content_get_subtype(c_content)));
	fileContent->setContentType(contentType);
	fileContent->setFileSize(linphone_content_get_size(c_content));
	fileContent->setFileName(linphone_content_get_name(c_content));
	fileContent->setFilePath(linphone_content_get_file_path(c_content));
	if (linphone_content_get_size(c_content) > 0) {
		fileContent->setBody(linphone_content_get_string_buffer(c_content));
	}
	L_GET_CPP_PTR_FROM_C_OBJECT(msg)->addContent(fileContent);
}

void linphone_chat_message_add_text_content (LinphoneChatMessage *msg, const char *text) {
	LinphonePrivate::Content *content = new LinphonePrivate::Content();
	LinphonePrivate::ContentType contentType = LinphonePrivate::ContentType::PlainText;
	content->setContentType(contentType);
	content->setBody(L_C_TO_STRING(text));
	L_GET_CPP_PTR_FROM_C_OBJECT(msg)->addContent(content);
}

void linphone_chat_message_remove_content (LinphoneChatMessage *msg, LinphoneContent *content) {
	L_GET_CPP_PTR_FROM_C_OBJECT(msg)->removeContent(L_GET_CPP_PTR_FROM_C_OBJECT(content));
}

const bctbx_list_t *linphone_chat_message_get_contents (const LinphoneChatMessage *msg) {
	if (msg->cache.contents)
		bctbx_free(msg->cache.contents);
	msg->cache.contents = L_GET_RESOLVED_C_LIST_FROM_CPP_LIST(L_GET_CPP_PTR_FROM_C_OBJECT(msg)->getContents());
	return msg->cache.contents;
}

bool_t linphone_chat_message_has_text_content (const LinphoneChatMessage *msg) {
	return L_GET_PRIVATE_FROM_C_OBJECT(msg)->hasTextContent();
}

const char *linphone_chat_message_get_text_content (const LinphoneChatMessage *msg) {
	const LinphonePrivate::Content *content = L_GET_PRIVATE_FROM_C_OBJECT(msg)->getTextContent();
	if (content->isEmpty())
		return nullptr;
	msg->cache.textContentBody = content->getBodyAsString();
	return L_STRING_TO_C(msg->cache.textContentBody);
}

bool_t linphone_chat_message_is_file_transfer_in_progress (const LinphoneChatMessage *msg) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(msg)->isFileTransferInProgress();
}

bctbx_list_t *linphone_chat_message_get_participants_by_imdn_state (const LinphoneChatMessage *msg, LinphoneChatMessageState state) {
	return L_GET_RESOLVED_C_LIST_FROM_CPP_LIST(
		L_GET_CPP_PTR_FROM_C_OBJECT(msg)->getParticipantsByImdnState(LinphonePrivate::ChatMessage::State(state))
	);
}

bool_t linphone_chat_message_download_content (LinphoneChatMessage *msg, LinphoneContent *c_content) {
	LinphonePrivate::Content *content = L_GET_CPP_PTR_FROM_C_OBJECT(c_content);
	if (!content->isFileTransfer()) {
		lError() << "LinphoneContent isn't an instance of FileTransferContent";
		return false;
	}
	LinphonePrivate::FileTransferContent *fileTransferContent = static_cast<LinphonePrivate::FileTransferContent* >(content);
	return !!L_GET_CPP_PTR_FROM_C_OBJECT(msg)->downloadFile(fileTransferContent);
}

// =============================================================================
// Old listener
// =============================================================================

LinphoneChatMessageStateChangedCb linphone_chat_message_get_message_state_changed_cb (LinphoneChatMessage* msg) {
	return msg->message_state_changed_cb;
}

void linphone_chat_message_set_message_state_changed_cb (LinphoneChatMessage* msg, LinphoneChatMessageStateChangedCb cb) {
	msg->message_state_changed_cb = cb;
}

void linphone_chat_message_set_message_state_changed_cb_user_data (LinphoneChatMessage* msg, void *user_data) {
	msg->message_state_changed_user_data = user_data;
}

void *linphone_chat_message_get_message_state_changed_cb_user_data (LinphoneChatMessage* msg) {
	return msg->message_state_changed_user_data;
}

// =============================================================================
// Structure has changed, hard to keep the behavior
// =============================================================================

const char *linphone_chat_message_get_content_type (const LinphoneChatMessage *msg) {
	msg->cache.contentType = L_GET_PRIVATE_FROM_C_OBJECT(msg)->getContentType().asString();
	return L_STRING_TO_C(msg->cache.contentType);
}

void linphone_chat_message_set_content_type (LinphoneChatMessage *msg, const char *content_type) {
	L_GET_PRIVATE_FROM_C_OBJECT(msg)->setContentType(LinphonePrivate::ContentType(L_C_TO_STRING(content_type)));
}

const char *linphone_chat_message_get_text (const LinphoneChatMessage *msg) {
	return L_STRING_TO_C(L_GET_PRIVATE_FROM_C_OBJECT(msg)->getText());
}

int linphone_chat_message_set_text (LinphoneChatMessage *msg, const char* text) {
	L_GET_PRIVATE_FROM_C_OBJECT(msg)->setText(L_C_TO_STRING(text));
	return 0;
}

LinphoneContent *linphone_chat_message_get_file_transfer_information (const LinphoneChatMessage *msg) {
	const LinphonePrivate::Content *content = L_GET_PRIVATE_FROM_C_OBJECT(msg)->getFileTransferInformation();
	if (content) return L_GET_C_BACK_PTR(content);
	return NULL;
}

bool_t linphone_chat_message_download_file (LinphoneChatMessage *msg) {
	return !!L_GET_PRIVATE_FROM_C_OBJECT(msg)->downloadFile();
}

// =============================================================================
// Nothing to do, they call other C API methods
// =============================================================================

const LinphoneAddress *linphone_chat_message_get_peer_address (const LinphoneChatMessage *msg) {
	return linphone_chat_room_get_peer_address(linphone_chat_message_get_chat_room(msg));
}

const LinphoneAddress *linphone_chat_message_get_local_address (const LinphoneChatMessage *msg) {
	if (L_GET_CPP_PTR_FROM_C_OBJECT(msg)->getDirection() == LinphonePrivate::ChatMessage::Direction::Outgoing)
		return linphone_chat_message_get_from_address(msg);
	return linphone_chat_message_get_to_address(msg);
}

LinphoneReason linphone_chat_message_get_reason (const LinphoneChatMessage *msg) {
	return linphone_error_info_get_reason(linphone_chat_message_get_error_info(msg));
}

bool_t linphone_chat_message_is_file_transfer (const LinphoneChatMessage *msg) {
	return L_GET_PRIVATE_FROM_C_OBJECT(msg)->hasFileTransferContent();
}

bool_t linphone_chat_message_is_text (const LinphoneChatMessage *msg) {
	return L_GET_PRIVATE_FROM_C_OBJECT(msg)->hasTextContent();
}

const char *linphone_chat_message_state_to_string (const LinphoneChatMessageState state) {
	switch (state) {
	case LinphoneChatMessageStateIdle:
		return "LinphoneChatMessageStateIdle";
	case LinphoneChatMessageStateInProgress:
		return "LinphoneChatMessageStateInProgress";
	case LinphoneChatMessageStateDelivered:
		return "LinphoneChatMessageStateDelivered";
	case LinphoneChatMessageStateNotDelivered:
		return "LinphoneChatMessageStateNotDelivered";
	case LinphoneChatMessageStateFileTransferError:
		return "LinphoneChatMessageStateFileTransferError";
	case LinphoneChatMessageStateFileTransferDone:
		return "LinphoneChatMessageStateFileTransferDone ";
	case LinphoneChatMessageStateDeliveredToUser:
		return "LinphoneChatMessageStateDeliveredToUser";
	case LinphoneChatMessageStateDisplayed:
		return "LinphoneChatMessageStateDisplayed";
	case LinphoneChatMessageStateFileTransferInProgress:
		return "LinphoneChatMessageStateFileTransferInProgress";
	}
	return NULL;
}

void linphone_chat_message_start_file_download (
	LinphoneChatMessage *msg,
	LinphoneChatMessageStateChangedCb status_cb,
	void *ud
) {
	msg->message_state_changed_cb = status_cb;
	msg->message_state_changed_user_data = ud;
	linphone_chat_message_download_file(msg);
}
