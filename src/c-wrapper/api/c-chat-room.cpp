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

#include <algorithm>
#include <iterator>

#include <bctoolbox/defs.h>

// TODO: Remove me later.
#include "linphone/chat.h"

#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "chat/chat-message/chat-message-p.h"
#include "chat/chat-room/abstract-chat-room.h"
#include "conference/conference-params.h"
#include "conference/participant-device-identity.h"
#include "linphone/api/c-call-log.h"
#include "linphone/api/c-chat-room.h"
#include "linphone/api/c-content.h"
#include "linphone/api/c-recorder.h"
#include "linphone/wrapper_utils.h"
#ifdef HAVE_ADVANCED_IM
#include "chat/chat-room/client-chat-room.h"
#include "chat/chat-room/server-chat-room.h"
#endif
#include "conference/participant.h"
#include "core/core-p.h"
#include "event-log/event-log.h"
#include "linphone/utils/utils.h"

// =============================================================================

using namespace std;
using namespace LinphonePrivate;

void linphone_chat_room_allow_multipart(LinphoneChatRoom *room) {
	AbstractChatRoom::toCpp(room)->allowMultipart(true);
}

void linphone_chat_room_allow_cpim(LinphoneChatRoom *room) {
	AbstractChatRoom::toCpp(room)->allowCpim(true);
}

// =============================================================================
// Public functions.
// =============================================================================

const LinphoneChatRoomParams *linphone_chat_room_get_current_params(const LinphoneChatRoom *chat_room) {
	return AbstractChatRoom::toCpp(chat_room)->getCurrentParams()->toC();
}

// Deprecated
void linphone_chat_room_send_message(LinphoneChatRoom *chat_room, const char *msg) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	AbstractChatRoom::toCpp(chat_room)->createChatMessage(msg)->send();
}

bool_t linphone_chat_room_is_remote_composing(const LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return AbstractChatRoom::toCpp(chat_room)->isRemoteComposing();
}

LinphoneCore *linphone_chat_room_get_lc(const LinphoneChatRoom *chat_room) {
	return linphone_chat_room_get_core(chat_room);
}

LinphoneCore *linphone_chat_room_get_core(const LinphoneChatRoom *chat_room) {
	return AbstractChatRoom::toCpp(chat_room)->getCore()->getCCore();
}

const LinphoneAddress *linphone_chat_room_get_peer_address(LinphoneChatRoom *chat_room) {
	const auto &address = AbstractChatRoom::toCpp(chat_room)->getPeerAddress();
	if (address) {
		return address->toC();
	} else {
		return NULL;
	}
}

const LinphoneAddress *linphone_chat_room_get_local_address(LinphoneChatRoom *chat_room) {
	const auto &address = AbstractChatRoom::toCpp(chat_room)->getLocalAddress();
	if (address) {
		return address->toC();
	} else {
		return NULL;
	}
}

const char *linphone_chat_room_get_identifier(const LinphoneChatRoom *chat_room) {
	const auto &identifier = AbstractChatRoom::toCpp(chat_room)->getIdentifier();
	if (identifier) {
		return L_STRING_TO_C(identifier.value());
	}
	return NULL;
}

LinphoneAccount *linphone_chat_room_get_account(LinphoneChatRoom *chat_room) {
	shared_ptr<Account> account = AbstractChatRoom::toCpp(chat_room)->getAccount();
	return (account) ? account->toC() : nullptr;
}

LinphoneChatMessage *linphone_chat_room_create_empty_message(LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	shared_ptr<ChatMessage> cppPtr = AbstractChatRoom::toCpp(chat_room)->createChatMessage();
	LinphoneChatMessage *object = L_INIT(ChatMessage);
	L_SET_CPP_PTR_FROM_C_OBJECT(object, cppPtr);
	return object;
}

LinphoneChatMessage *linphone_chat_room_create_message_from_utf8(LinphoneChatRoom *chat_room, const char *message) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	shared_ptr<ChatMessage> cppPtr =
	    AbstractChatRoom::toCpp(chat_room)->createChatMessageFromUtf8(L_C_TO_STRING(message));
	LinphoneChatMessage *object = L_INIT(ChatMessage);
	L_SET_CPP_PTR_FROM_C_OBJECT(object, cppPtr);
	return object;
}

// Deprecated
LinphoneChatMessage *linphone_chat_room_create_message(LinphoneChatRoom *chat_room, const char *message) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	shared_ptr<ChatMessage> cppPtr = AbstractChatRoom::toCpp(chat_room)->createChatMessage(L_C_TO_STRING(message));
	LinphoneChatMessage *object = L_INIT(ChatMessage);
	L_SET_CPP_PTR_FROM_C_OBJECT(object, cppPtr);
	return object;
}

LinphoneChatMessage *linphone_chat_room_create_file_transfer_message(LinphoneChatRoom *chat_room,
                                                                     LinphoneContent *initial_content) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	LinphoneChatMessage *msg = linphone_chat_room_create_empty_message(chat_room);
	linphone_chat_message_add_file_content(msg, initial_content);
	return msg;
}

// Deprecated
LinphoneChatMessage *linphone_chat_room_create_message_2(LinphoneChatRoom *chat_room,
                                                         const char *message,
                                                         const char *external_body_url,
                                                         LinphoneChatMessageState state,
                                                         time_t time,
                                                         BCTBX_UNUSED(bool_t is_read),
                                                         BCTBX_UNUSED(bool_t is_incoming)) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	LinphoneChatMessage *msg = linphone_chat_room_create_message(chat_room, message);

	linphone_chat_message_set_external_body_url(msg, external_body_url ? ms_strdup(external_body_url) : NULL);

	ChatMessagePrivate *dMsg = L_GET_PRIVATE_FROM_C_OBJECT(msg);
	dMsg->setTime(time);
	dMsg->setParticipantState(AbstractChatRoom::toCpp(chat_room)->getMe()->getAddress(),
	                          static_cast<ChatMessage::State>(state), ::ms_time(NULL));

	return msg;
}

LinphoneChatMessage *linphone_chat_room_create_forward_message(LinphoneChatRoom *chat_room, LinphoneChatMessage *msg) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	shared_ptr<ChatMessage> cppPtr =
	    AbstractChatRoom::toCpp(chat_room)->createForwardMessage(L_GET_CPP_PTR_FROM_C_OBJECT(msg));
	LinphoneChatMessage *object = L_INIT(ChatMessage);
	L_SET_CPP_PTR_FROM_C_OBJECT(object, cppPtr);
	return object;
}

LinphoneChatMessage *linphone_chat_room_create_reply_message(LinphoneChatRoom *chat_room, LinphoneChatMessage *msg) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	shared_ptr<ChatMessage> cppPtr =
	    AbstractChatRoom::toCpp(chat_room)->createReplyMessage(L_GET_CPP_PTR_FROM_C_OBJECT(msg));
	LinphoneChatMessage *object = L_INIT(ChatMessage);
	L_SET_CPP_PTR_FROM_C_OBJECT(object, cppPtr);
	return object;
}

LinphoneChatMessage *linphone_chat_room_create_voice_recording_message(LinphoneChatRoom *chat_room,
                                                                       LinphoneRecorder *recorder) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	LinphoneChatMessage *chat_message = linphone_chat_room_create_empty_message(chat_room);

	LinphoneContent *c_content = linphone_recorder_create_content(recorder);
	if (c_content != nullptr) {
		linphone_chat_message_add_content(chat_message, c_content);
		linphone_content_unref(c_content);
	}

	return chat_message;
}

void linphone_chat_room_send_chat_message_2(BCTBX_UNUSED(LinphoneChatRoom *chat_room), LinphoneChatMessage *msg) {
	linphone_chat_message_ref(msg);
	L_GET_CPP_PTR_FROM_C_OBJECT(msg)->send();
}

void linphone_chat_room_send_chat_message(BCTBX_UNUSED(LinphoneChatRoom *chat_room), LinphoneChatMessage *msg) {
	L_GET_CPP_PTR_FROM_C_OBJECT(msg)->send();
}

void linphone_chat_room_receive_chat_message(BCTBX_UNUSED(LinphoneChatRoom *chat_room), LinphoneChatMessage *msg) {
	L_GET_PRIVATE_FROM_C_OBJECT(msg)->receive();
}

uint32_t linphone_chat_room_get_char(LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return AbstractChatRoom::toCpp(chat_room)->getChar();
}

void linphone_chat_room_compose(LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	AbstractChatRoom::toCpp(chat_room)->compose();
}

LinphoneCall *linphone_chat_room_get_call(const LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	shared_ptr<Call> call = AbstractChatRoom::toCpp(chat_room)->getCall();
	if (call) return call->toC();
	return nullptr;
}

void linphone_chat_room_set_call(LinphoneChatRoom *chat_room, LinphoneCall *call) {
	AbstractChatRoom::toCpp(chat_room)->setCallId(linphone_call_log_get_call_id(linphone_call_get_call_log(call)));
}

void linphone_chat_room_mark_as_read(LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	AbstractChatRoom::toCpp(chat_room)->markAsRead();
}

void linphone_chat_room_set_ephemeral_mode(LinphoneChatRoom *chat_room, LinphoneChatRoomEphemeralMode mode) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	AbstractChatRoom::toCpp(chat_room)->setEphemeralMode(static_cast<AbstractChatRoom::EphemeralMode>(mode), true);
}

LinphoneChatRoomEphemeralMode linphone_chat_room_get_ephemeral_mode(const LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return static_cast<LinphoneChatRoomEphemeralMode>(AbstractChatRoom::toCpp(chat_room)->getEphemeralMode());
}

void linphone_chat_room_enable_ephemeral(LinphoneChatRoom *chat_room, bool_t ephem) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	AbstractChatRoom::toCpp(chat_room)->enableEphemeral(!!ephem, true);
}

bool_t linphone_chat_room_ephemeral_enabled(const LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return (bool_t)AbstractChatRoom::toCpp(chat_room)->ephemeralEnabled();
}

void linphone_chat_room_set_ephemeral_lifetime(LinphoneChatRoom *chat_room, long time) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	AbstractChatRoom::toCpp(chat_room)->setEphemeralLifetime(time, true);
}

long linphone_chat_room_get_ephemeral_lifetime(const LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return AbstractChatRoom::toCpp(chat_room)->getEphemeralLifetime();
}

bool_t linphone_chat_room_ephemeral_supported_by_all_participants(const LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return (bool_t)AbstractChatRoom::toCpp(chat_room)->ephemeralSupportedByAllParticipants();
}

int linphone_chat_room_get_unread_messages_count(LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return AbstractChatRoom::toCpp(chat_room)->getUnreadChatMessageCount();
}

int linphone_chat_room_get_history_size(LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return AbstractChatRoom::toCpp(chat_room)->getChatMessageCount();
}

int linphone_chat_room_get_history_size_2(LinphoneChatRoom *chat_room, LinphoneChatRoomHistoryFilterMask filters) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return AbstractChatRoom::toCpp(chat_room)->getHistorySize(filters);
}

bool_t linphone_chat_room_is_empty(LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return (bool_t)AbstractChatRoom::toCpp(chat_room)->isEmpty();
}

void linphone_chat_room_delete_message(LinphoneChatRoom *chat_room, LinphoneChatMessage *msg) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	AbstractChatRoom::toCpp(chat_room)->deleteMessageFromHistory(L_GET_CPP_PTR_FROM_C_OBJECT(msg));
}

void linphone_chat_room_delete_history(LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	AbstractChatRoom::toCpp(chat_room)->deleteHistory();
}

bctbx_list_t *linphone_chat_room_get_media_contents(LinphoneChatRoom *chat_room) {
	LinphonePrivate::ChatRoomLogContextualizer logContextualizer(chat_room);
	list<shared_ptr<LinphonePrivate::Content>> contents = AbstractChatRoom::toCpp(chat_room)->getMediaContents();
	return LinphonePrivate::Content::getCListFromCppList(contents, true);
}

bctbx_list_t *linphone_chat_room_get_document_contents(LinphoneChatRoom *chat_room) {
	LinphonePrivate::ChatRoomLogContextualizer logContextualizer(chat_room);
	list<shared_ptr<LinphonePrivate::Content>> contents = AbstractChatRoom::toCpp(chat_room)->getDocumentContents();
	return LinphonePrivate::Content::getCListFromCppList(contents, true);
}

bctbx_list_t *linphone_chat_room_get_history_range(LinphoneChatRoom *chat_room, int startm, int endm) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	list<shared_ptr<ChatMessage>> chatMessages;
	for (auto &event : AbstractChatRoom::toCpp(chat_room)->getMessageHistoryRange(startm, endm))
		chatMessages.push_back(static_pointer_cast<ConferenceChatMessageEvent>(event)->getChatMessage());

	return L_GET_RESOLVED_C_LIST_FROM_CPP_LIST(chatMessages);
}

bctbx_list_t *linphone_chat_room_get_history_range_2(LinphoneChatRoom *chat_room,
                                                     int startm,
                                                     int endm,
                                                     LinphoneChatRoomHistoryFilterMask filters) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return L_GET_RESOLVED_C_LIST_FROM_CPP_LIST(
	    AbstractChatRoom::toCpp(chat_room)->getHistoryRange(startm, endm, filters));
}

bctbx_list_t *linphone_chat_room_get_history(LinphoneChatRoom *chat_room, int nb_message) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return linphone_chat_room_get_history_range(chat_room, 0, nb_message);
}

bctbx_list_t *linphone_chat_room_get_history_2(LinphoneChatRoom *chat_room,
                                               int nb_message,
                                               LinphoneChatRoomHistoryFilterMask filters) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return linphone_chat_room_get_history_range_2(chat_room, 0, nb_message, filters);
}

bctbx_list_t *linphone_chat_room_get_history_range_near(LinphoneChatRoom *chat_room,
                                                        unsigned int before,
                                                        unsigned int after,
                                                        LinphoneEventLog *event,
                                                        LinphoneChatRoomHistoryFilterMask filters) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return L_GET_RESOLVED_C_LIST_FROM_CPP_LIST(AbstractChatRoom::toCpp(chat_room)->getHistoryRangeNear(
	    before, after, event ? L_GET_CPP_PTR_FROM_C_OBJECT(event) : nullptr, filters));
}

bctbx_list_t *linphone_chat_room_get_history_range_between(LinphoneChatRoom *chat_room,
                                                           LinphoneEventLog *first_event,
                                                           LinphoneEventLog *last_event,
                                                           LinphoneChatRoomHistoryFilterMask filters) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return L_GET_RESOLVED_C_LIST_FROM_CPP_LIST(AbstractChatRoom::toCpp(chat_room)->getHistoryRangeBetween(
	    first_event ? L_GET_CPP_PTR_FROM_C_OBJECT(first_event) : nullptr,
	    last_event ? L_GET_CPP_PTR_FROM_C_OBJECT(last_event) : nullptr, filters));
}

bctbx_list_t *linphone_chat_room_get_unread_history(LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return L_GET_RESOLVED_C_LIST_FROM_CPP_LIST(AbstractChatRoom::toCpp(chat_room)->getUnreadChatMessages());
}

bctbx_list_t *linphone_chat_room_get_history_range_message_events(LinphoneChatRoom *chat_room, int startm, int endm) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return L_GET_RESOLVED_C_LIST_FROM_CPP_LIST(
	    AbstractChatRoom::toCpp(chat_room)->getMessageHistoryRange(startm, endm));
}

bctbx_list_t *linphone_chat_room_get_history_message_events(LinphoneChatRoom *chat_room, int nb_events) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return L_GET_RESOLVED_C_LIST_FROM_CPP_LIST(AbstractChatRoom::toCpp(chat_room)->getMessageHistory(nb_events));
}

bctbx_list_t *linphone_chat_room_get_history_events(LinphoneChatRoom *chat_room, int nb_events) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return L_GET_RESOLVED_C_LIST_FROM_CPP_LIST(AbstractChatRoom::toCpp(chat_room)->getHistory(nb_events));
}

bctbx_list_t *linphone_chat_room_get_history_range_events(LinphoneChatRoom *chat_room, int begin, int end) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return L_GET_RESOLVED_C_LIST_FROM_CPP_LIST(AbstractChatRoom::toCpp(chat_room)->getHistoryRange(begin, end));
}

int linphone_chat_room_get_history_events_size(LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return AbstractChatRoom::toCpp(chat_room)->getHistorySize();
}

LinphoneChatMessage *linphone_chat_room_get_last_message_in_history(LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	shared_ptr<ChatMessage> cppPtr = AbstractChatRoom::toCpp(chat_room)->getLastChatMessageInHistory();
	if (!cppPtr) return nullptr;

	return linphone_chat_message_ref(L_GET_C_BACK_PTR(cppPtr));
}

LinphoneChatMessage *linphone_chat_room_find_message(LinphoneChatRoom *chat_room, const char *message_id) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	shared_ptr<ChatMessage> cppPtr = AbstractChatRoom::toCpp(chat_room)->findChatMessage(message_id);
	if (!cppPtr) return nullptr;

	return linphone_chat_message_ref(L_GET_C_BACK_PTR(cppPtr));
}

LinphoneEventLog *linphone_chat_room_find_event_log(LinphoneChatRoom *chat_room, const char *message_id) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	shared_ptr<EventLog> cppPtr = AbstractChatRoom::toCpp(chat_room)->findChatMessageEventLog(message_id);
	if (!cppPtr) return nullptr;

	return linphone_event_log_ref(L_GET_C_BACK_PTR(cppPtr));
}

LinphoneEventLog *linphone_chat_room_search_chat_message_by_text(LinphoneChatRoom *chat_room,
                                                                 const char *text,
                                                                 const LinphoneEventLog *from,
                                                                 LinphoneSearchDirection direction) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	shared_ptr<EventLog> cppPtr = AbstractChatRoom::toCpp(chat_room)->searchChatMessageByText(
	    L_C_TO_STRING(text), from ? L_GET_CPP_PTR_FROM_C_OBJECT(from) : nullptr, direction);
	if (!cppPtr) return nullptr;

	return linphone_event_log_ref(L_GET_C_BACK_PTR(cppPtr));
}

LinphoneChatRoomState linphone_chat_room_get_state(const LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return linphone_conference_state_to_chat_room_state(
	    static_cast<LinphoneConferenceState>(AbstractChatRoom::toCpp(chat_room)->getState()));
}

bool_t linphone_chat_room_has_been_left(const LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return (bool_t)AbstractChatRoom::toCpp(chat_room)->hasBeenLeft();
}

bool_t linphone_chat_room_is_read_only(const LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return (bool_t)AbstractChatRoom::toCpp(chat_room)->isReadOnly();
}

time_t linphone_chat_room_get_creation_time(const LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return AbstractChatRoom::toCpp(chat_room)->getCreationTime();
}

time_t linphone_chat_room_get_last_update_time(const LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return AbstractChatRoom::toCpp(chat_room)->getLastUpdateTime();
}

void linphone_chat_room_add_participant(LinphoneChatRoom *chat_room, LinphoneAddress *addr) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	if (linphone_chat_room_can_handle_participants(chat_room)) {
		AbstractChatRoom::toCpp(chat_room)->getConference()->addParticipant(Address::toCpp(addr)->getSharedFromThis());
	}
}

bool_t linphone_chat_room_add_participants(LinphoneChatRoom *chat_room, const bctbx_list_t *addresses) {
	LinphonePrivate::ChatRoomLogContextualizer logContextualizer(chat_room);
	if (linphone_chat_room_can_handle_participants(chat_room)) {
		std::list<std::shared_ptr<LinphonePrivate::Address>> addressList =
		    LinphonePrivate::Utils::bctbxListToCppSharedPtrList<LinphoneAddress, LinphonePrivate::Address>(addresses);
		return AbstractChatRoom::toCpp(chat_room)->getConference()->addParticipants(addressList);
	}
	return FALSE;
}

LinphoneParticipant *linphone_chat_room_find_participant(const LinphoneChatRoom *chat_room, LinphoneAddress *addr) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	std::shared_ptr<Participant> participant =
	    AbstractChatRoom::toCpp(chat_room)->findParticipant(Address::toCpp(addr)->getSharedFromThis());
	if (participant) {
		return participant->toC();
	}
	return NULL;
}

bool_t linphone_chat_room_can_handle_participants(const LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return AbstractChatRoom::toCpp(chat_room)->canHandleParticipants();
}

LinphoneChatRoomCapabilitiesMask linphone_chat_room_get_capabilities(const LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return AbstractChatRoom::toCpp(chat_room)->getCapabilities();
}

bool_t linphone_chat_room_has_capability(const LinphoneChatRoom *chat_room, int mask) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return static_cast<bool_t>(AbstractChatRoom::toCpp(chat_room)->getCapabilities() & mask);
}

const LinphoneAddress *linphone_chat_room_get_conference_address(const LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	std::shared_ptr<Conference> conference = AbstractChatRoom::toCpp(chat_room)->getConference();
	if (conference) {
		const auto &confAddress = conference->getConferenceAddress();
		if (confAddress && confAddress->isValid()) {
			return confAddress->toC();
		}
	}
	return NULL;
}

LinphoneParticipant *linphone_chat_room_get_me(const LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	std::shared_ptr<Participant> me = AbstractChatRoom::toCpp(chat_room)->getMe();
	if (me) {
		return me->toC();
	}
	return NULL;
}

int linphone_chat_room_get_nb_participants(const LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	if (linphone_chat_room_can_handle_participants(chat_room)) {
		return AbstractChatRoom::toCpp(chat_room)->getConference()->getParticipantCount();
	}
	return -1;
}

bctbx_list_t *linphone_chat_room_get_participants(const LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return Participant::getCListFromCppList(AbstractChatRoom::toCpp(chat_room)->getParticipants());
}

const char *linphone_chat_room_get_subject(const LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return L_STRING_TO_C(AbstractChatRoom::toCpp(chat_room)->getSubject());
}

const char *linphone_chat_room_get_subject_utf8(const LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return L_STRING_TO_C(AbstractChatRoom::toCpp(chat_room)->getSubjectUtf8());
}

LinphoneChatRoomSecurityLevel linphone_chat_room_get_security_level(LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return (LinphoneChatRoomSecurityLevel)AbstractChatRoom::toCpp(chat_room)->getSecurityLevel();
}

void linphone_chat_room_leave(LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	AbstractChatRoom::toCpp(chat_room)->getConference()->leave();
}

void linphone_chat_room_remove_participant(LinphoneChatRoom *chat_room, LinphoneParticipant *participant) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	if (linphone_chat_room_can_handle_participants(chat_room)) {
		AbstractChatRoom::toCpp(chat_room)->getConference()->removeParticipant(
		    Participant::toCpp(participant)->getSharedFromThis());
	}
}

void linphone_chat_room_remove_participants(LinphoneChatRoom *chat_room, const bctbx_list_t *participants) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	if (linphone_chat_room_can_handle_participants(chat_room)) {
		AbstractChatRoom::toCpp(chat_room)->getConference()->removeParticipants(
		    Utils::bctbxListToCppSharedPtrList<LinphoneParticipant, Participant>(participants));
	}
}

void linphone_chat_room_set_participant_admin_status(LinphoneChatRoom *chat_room,
                                                     LinphoneParticipant *participant,
                                                     bool_t isAdmin) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	if (linphone_chat_room_can_handle_participants(chat_room)) {
		shared_ptr<Participant> p = Participant::toCpp(participant)->getSharedFromThis();
		AbstractChatRoom::toCpp(chat_room)->getConference()->setParticipantAdminStatus(p, !!isAdmin);
	}
}

void linphone_chat_room_set_subject(LinphoneChatRoom *chat_room, const char *subject) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	AbstractChatRoom::toCpp(chat_room)->setSubject(L_C_TO_STRING(subject));
}

void linphone_chat_room_set_subject_utf8(LinphoneChatRoom *chat_room, const char *subject) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	AbstractChatRoom::toCpp(chat_room)->setUtf8Subject(L_C_TO_STRING(subject));
}

const bctbx_list_t *linphone_chat_room_get_composing_addresses(LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return AbstractChatRoom::toCpp(chat_room)->getComposingCAddresses();
}

bool_t linphone_chat_room_get_muted(const LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	return AbstractChatRoom::toCpp(chat_room)->getIsMuted();
}

void linphone_chat_room_set_muted(LinphoneChatRoom *chat_room, bool_t muted) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	AbstractChatRoom::toCpp(chat_room)->setIsMuted(!!muted);
}

const LinphoneConferenceInfo *linphone_chat_room_get_conference_info(LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	auto info = AbstractChatRoom::toCpp(chat_room)->getConferenceInfo();
	return info ? info->toC() : nullptr;
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void linphone_chat_room_set_conference_address(LinphoneChatRoom *chat_room, LinphoneAddress *confAddr) {
#ifdef HAVE_ADVANCED_IM
	shared_ptr<ServerChatRoom> sgcr =
	    dynamic_pointer_cast<ServerChatRoom>(AbstractChatRoom::toCpp(chat_room)->getSharedFromThis());
	if (sgcr) {
		std::shared_ptr<LinphonePrivate::Address> idAddr =
		    confAddr ? LinphonePrivate::Address::toCpp(confAddr)->getSharedFromThis() : NULL;
		sgcr->getConference()->setConferenceAddress(idAddr);
	}
#else
	lWarning() << "Advanced IM such as group chat is disabled!";
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void linphone_chat_room_set_participant_devices(LinphoneChatRoom *chat_room,
                                                LinphoneAddress *partAddr,
                                                const bctbx_list_t *deviceIdentities) {
#ifdef HAVE_ADVANCED_IM
	list<shared_ptr<ParticipantDeviceIdentity>> lDevicesIdentities =
	    Utils::bctbxListToCppSharedPtrList<LinphoneParticipantDeviceIdentity,
	                                       LinphonePrivate::ParticipantDeviceIdentity>(deviceIdentities);
	shared_ptr<ServerChatRoom> sgcr =
	    dynamic_pointer_cast<ServerChatRoom>(AbstractChatRoom::toCpp(chat_room)->getSharedFromThis());
	if (sgcr)
		dynamic_pointer_cast<ServerConference>(sgcr->getConference())
		    ->setParticipantDevices(Address::toCpp(partAddr)->getSharedFromThis(), lDevicesIdentities);
#else
	lWarning() << "Advanced IM such as group chat is disabled!";
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void linphone_chat_room_notify_participant_device_registration(LinphoneChatRoom *chat_room,
                                                               const LinphoneAddress *participant_device) {
#ifdef HAVE_ADVANCED_IM
	shared_ptr<ServerChatRoom> sgcr =
	    dynamic_pointer_cast<ServerChatRoom>(AbstractChatRoom::toCpp(chat_room)->getSharedFromThis());
	if (sgcr) {
		sgcr->notifyParticipantDeviceRegistration(
		    LinphonePrivate::Address::toCpp(participant_device)->getSharedFromThis());
	}
#else
	lWarning() << "Advanced IM such as group chat is disabled!";
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

// =============================================================================
// Callbacks
// =============================================================================

void linphone_chat_room_add_callbacks(LinphoneChatRoom *chat_room, LinphoneChatRoomCbs *cbs) {
	AbstractChatRoom::toCpp(chat_room)->addCallbacks(ChatRoomCbs::toCpp(cbs)->getSharedFromThis());
}

void linphone_chat_room_remove_callbacks(LinphoneChatRoom *chat_room, LinphoneChatRoomCbs *cbs) {
	AbstractChatRoom::toCpp(chat_room)->removeCallbacks(ChatRoomCbs::toCpp(cbs)->getSharedFromThis());
}

void linphone_chat_room_set_current_callbacks(LinphoneChatRoom *chat_room, LinphoneChatRoomCbs *cbs) {
	AbstractChatRoom::toCpp(chat_room)->setCurrentCallbacks(cbs ? ChatRoomCbs::toCpp(cbs)->getSharedFromThis()
	                                                            : nullptr);
}

LinphoneChatRoomCbs *linphone_chat_room_get_current_callbacks(const LinphoneChatRoom *chat_room) {
	return AbstractChatRoom::toCpp(chat_room)->getCurrentCallbacks()->toC();
}

const bctbx_list_t *linphone_chat_room_get_callbacks_list(const LinphoneChatRoom *chat_room) {
	return AbstractChatRoom::toCpp(chat_room)->getCCallbacksList();
}

void _linphone_chat_room_notify_is_composing_received(LinphoneChatRoom *chat_room,
                                                      const LinphoneAddress *remoteAddr,
                                                      bool_t isComposing) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_is_composing_received, remoteAddr, isComposing);
}

void linphone_chat_room_notify_session_state_changed(LinphoneChatRoom *chat_room,
                                                     LinphoneCallState cstate,
                                                     const char *message) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_session_state_changed, cstate, message);
}

void _linphone_chat_room_notify_message_received(LinphoneChatRoom *chat_room, LinphoneChatMessage *msg) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_message_received, msg);
}

void _linphone_chat_room_notify_messages_received(LinphoneChatRoom *chat_room, const bctbx_list_t *chat_messages) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_messages_received, chat_messages);
}

void _linphone_chat_room_notify_new_event(LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_new_event, event_log);
}

void _linphone_chat_room_notify_new_events(LinphoneChatRoom *chat_room, const bctbx_list_t *event_logs) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_new_events, event_logs);
}

void _linphone_chat_room_notify_participant_added(LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log) {
	_linphone_chat_room_notify_new_event(chat_room, event_log);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_participant_added, event_log);
}

void _linphone_chat_room_notify_participant_removed(LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log) {
	_linphone_chat_room_notify_new_event(chat_room, event_log);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_participant_removed, event_log);
}

void _linphone_chat_room_notify_participant_device_added(LinphoneChatRoom *chat_room,
                                                         const LinphoneEventLog *event_log) {
	_linphone_chat_room_notify_new_event(chat_room, event_log);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_participant_device_added, event_log);
}

void _linphone_chat_room_notify_participant_device_removed(LinphoneChatRoom *chat_room,
                                                           const LinphoneEventLog *event_log) {
	_linphone_chat_room_notify_new_event(chat_room, event_log);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_participant_device_removed, event_log);
}

void _linphone_chat_room_notify_participant_device_state_changed(LinphoneChatRoom *chat_room,
                                                                 const LinphoneEventLog *event_log,
                                                                 const LinphoneParticipantDeviceState state) {
	_linphone_chat_room_notify_new_event(chat_room, event_log);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_participant_device_state_changed, event_log, state);
}

void _linphone_chat_room_notify_participant_device_media_availability_changed(LinphoneChatRoom *chat_room,
                                                                              const LinphoneEventLog *event_log) {
	_linphone_chat_room_notify_new_event(chat_room, event_log);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_participant_device_media_availability_changed,
	                                  event_log);
}

void _linphone_chat_room_notify_participant_admin_status_changed(LinphoneChatRoom *chat_room,
                                                                 const LinphoneEventLog *event_log) {
	_linphone_chat_room_notify_new_event(chat_room, event_log);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_participant_admin_status_changed, event_log);
}

void _linphone_chat_room_notify_state_changed(LinphoneChatRoom *chat_room, LinphoneChatRoomState newState) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_state_changed, newState);
}

void _linphone_chat_room_notify_security_event(LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log) {
	_linphone_chat_room_notify_new_event(chat_room, event_log);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_security_event, event_log);
}

void _linphone_chat_room_notify_subject_changed(LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log) {
	_linphone_chat_room_notify_new_event(chat_room, event_log);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_subject_changed, event_log);
}

void _linphone_chat_room_notify_conference_joined(LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log) {
	_linphone_chat_room_notify_new_event(chat_room, event_log);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_conference_joined, event_log);
}

void _linphone_chat_room_notify_conference_left(LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log) {
	_linphone_chat_room_notify_new_event(chat_room, event_log);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_conference_left, event_log);
}

void _linphone_chat_room_notify_ephemeral_event(LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log) {
	_linphone_chat_room_notify_new_event(chat_room, event_log);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_ephemeral_event, event_log);
}

void _linphone_chat_room_notify_ephemeral_message_timer_started(LinphoneChatRoom *chat_room,
                                                                const LinphoneEventLog *event_log) {
	_linphone_chat_room_notify_new_event(chat_room, event_log);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_ephemeral_message_timer_started, event_log);
}

void _linphone_chat_room_notify_ephemeral_message_deleted(LinphoneChatRoom *chat_room,
                                                          const LinphoneEventLog *event_log) {
	_linphone_chat_room_notify_new_event(chat_room, event_log);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_ephemeral_message_deleted, event_log);
}

void _linphone_chat_room_notify_undecryptable_message_received(LinphoneChatRoom *chat_room, LinphoneChatMessage *msg) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_undecryptable_message_received, msg);
}

void _linphone_chat_room_notify_chat_message_received(LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log) {
	_linphone_chat_room_notify_new_event(chat_room, event_log);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_chat_message_received, event_log);
}

void _linphone_chat_room_notify_chat_messages_received(LinphoneChatRoom *chat_room, const bctbx_list_t *event_logs) {
	_linphone_chat_room_notify_new_events(chat_room, event_logs);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_chat_messages_received, event_logs);
}

void _linphone_chat_room_notify_chat_message_sending(LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log) {
	_linphone_chat_room_notify_new_event(chat_room, event_log);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_chat_message_sending, event_log);
}

void _linphone_chat_room_notify_chat_message_sent(LinphoneChatRoom *chat_room, const LinphoneEventLog *event_log) {
	_linphone_chat_room_notify_new_event(chat_room, event_log);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_chat_message_sent, event_log);
}

void _linphone_chat_room_notify_conference_address_generation(LinphoneChatRoom *chat_room) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_NO_ARG(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                         linphone_chat_room_cbs_get_conference_address_generation);
}

void _linphone_chat_room_notify_participant_registration_subscription_requested(
    LinphoneChatRoom *chat_room, const LinphoneAddress *participantAddr) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_participant_registration_subscription_requested,
	                                  participantAddr);
}

void _linphone_chat_room_notify_participant_registration_unsubscription_requested(
    LinphoneChatRoom *chat_room, const LinphoneAddress *participantAddr) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_participant_registration_unsubscription_requested,
	                                  participantAddr);
}

void _linphone_chat_room_notify_chat_message_should_be_stored(LinphoneChatRoom *chat_room, LinphoneChatMessage *msg) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_chat_message_should_be_stored, msg);
}

void _linphone_chat_room_notify_chat_message_participant_imdn_state_changed(LinphoneChatRoom *chat_room,
                                                                            LinphoneChatMessage *msg,
                                                                            const LinphoneParticipantImdnState *state) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_chat_message_participant_imdn_state_changed, msg,
	                                  state);
}

void _linphone_chat_room_notify_chat_room_read(LinphoneChatRoom *chat_room) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_NO_ARG(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                         linphone_chat_room_cbs_get_chat_room_read);
}

void _linphone_chat_room_notify_new_reaction_received(LinphoneChatRoom *chat_room,
                                                      LinphoneChatMessage *message,
                                                      const LinphoneChatMessageReaction *reaction) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, AbstractChatRoom::toCpp(chat_room),
	                                  linphone_chat_room_cbs_get_new_message_reaction, message, reaction);
}

// =============================================================================
// Reference and user data handling functions.
// =============================================================================

LinphoneChatRoom *linphone_chat_room_ref(LinphoneChatRoom *chat_room) {
	belle_sip_object_ref(chat_room);
	return chat_room;
}

void linphone_chat_room_unref(LinphoneChatRoom *chat_room) {
	ChatRoomLogContextualizer logContextualizer(chat_room);
	belle_sip_object_unref(chat_room);
}

void *linphone_chat_room_get_user_data(const LinphoneChatRoom *chat_room) {
	return AbstractChatRoom::toCpp(chat_room)->getUserData();
}

void linphone_chat_room_set_user_data(LinphoneChatRoom *chat_room, void *ud) {
	AbstractChatRoom::toCpp(chat_room)->setUserData(ud);
}

// =============================================================================
// Constructor and destructor functions.
// =============================================================================

// Convert chat room enum to conference state enum
LinphoneConferenceState linphone_chat_room_state_to_conference_state(LinphoneChatRoomState state) {
	// No default statement to trigger an error in case a new value is added to LinphoneChatRoomState
	switch (state) {
		case LinphoneChatRoomStateNone:
			return LinphoneConferenceStateNone;
		case LinphoneChatRoomStateInstantiated:
			return LinphoneConferenceStateInstantiated;
		case LinphoneChatRoomStateCreationPending:
			return LinphoneConferenceStateCreationPending;
		case LinphoneChatRoomStateCreated:
			return LinphoneConferenceStateCreated;
		case LinphoneChatRoomStateCreationFailed:
			return LinphoneConferenceStateCreationFailed;
		case LinphoneChatRoomStateTerminationPending:
			return LinphoneConferenceStateTerminationPending;
		case LinphoneChatRoomStateTerminated:
			return LinphoneConferenceStateTerminated;
		case LinphoneChatRoomStateTerminationFailed:
			return LinphoneConferenceStateTerminationFailed;
		case LinphoneChatRoomStateDeleted:
			return LinphoneConferenceStateDeleted;
	}
	return LinphoneConferenceStateNone;
}

// Convert conference state enum to chat room state enum
LinphoneChatRoomState linphone_conference_state_to_chat_room_state(LinphoneConferenceState state) {
	// No default statement to trigger an error in case a new value is added to LinphoneConferenceState
	switch (state) {
		case LinphoneConferenceStateNone:
			return LinphoneChatRoomStateNone;
		case LinphoneConferenceStateInstantiated:
			return LinphoneChatRoomStateInstantiated;
		case LinphoneConferenceStateCreationPending:
			return LinphoneChatRoomStateCreationPending;
		case LinphoneConferenceStateCreated:
			return LinphoneChatRoomStateCreated;
		case LinphoneConferenceStateCreationFailed:
			return LinphoneChatRoomStateCreationFailed;
		case LinphoneConferenceStateTerminationPending:
			return LinphoneChatRoomStateTerminationPending;
		case LinphoneConferenceStateTerminated:
			return LinphoneChatRoomStateTerminated;
		case LinphoneConferenceStateTerminationFailed:
			return LinphoneChatRoomStateTerminationFailed;
		case LinphoneConferenceStateDeleted:
			return LinphoneChatRoomStateDeleted;
	}
	return LinphoneChatRoomStateNone;
}

const char *linphone_chat_room_state_to_string(const LinphoneChatRoomState state) {
	switch (state) {
		case LinphoneChatRoomStateNone:
			return "LinphoneChatRoomStateNone";
		case LinphoneChatRoomStateInstantiated:
			return "LinphoneChatRoomStateInstantiated";
		case LinphoneChatRoomStateCreationPending:
			return "LinphoneChatRoomStateCreationPending";
		case LinphoneChatRoomStateCreated:
			return "LinphoneChatRoomStateCreated";
		case LinphoneChatRoomStateCreationFailed:
			return "LinphoneChatRoomStateCreationFailed";
		case LinphoneChatRoomStateTerminationPending:
			return "LinphoneChatRoomStateTerminationPending";
		case LinphoneChatRoomStateTerminated:
			return "LinphoneChatRoomStateTerminated";
		case LinphoneChatRoomStateTerminationFailed:
			return "LinphoneChatRoomStateTerminationFailed";
		case LinphoneChatRoomStateDeleted:
			return "LinphoneChatRoomStateDeleted";
	}
	return "Unknown state";
}
