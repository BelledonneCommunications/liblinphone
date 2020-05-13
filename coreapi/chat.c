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

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlwriter.h>

#include <linphone/utils/utils.h>

#include "linphone/core.h"
#include "private.h"
#include "linphone/lpconfig.h"
#include "belle-sip/belle-sip.h"
#include "ortp/b64.h"
#include "linphone/wrapper_utils.h"

#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "chat/chat-room/chat-room-params.h"
#include "chat/chat-room/basic-chat-room.h"
#ifdef HAVE_ADVANCED_IM
#include "chat/chat-room/client-group-chat-room.h"
#include "chat/chat-room/client-group-to-basic-chat-room.h"
#endif
#include "chat/chat-room/real-time-text-chat-room-p.h"
#include "chat/chat-room/real-time-text-chat-room.h"
#include "content/content-type.h"
#include "core/core-p.h"
#include "linphone/api/c-chat-room-params.h"

using namespace std;

void linphone_core_disable_chat(LinphoneCore *lc, LinphoneReason deny_reason) {
	lc->chat_deny_code = deny_reason;
}

void linphone_core_enable_chat(LinphoneCore *lc) {
	lc->chat_deny_code = LinphoneReasonNone;
}

bool_t linphone_core_chat_enabled(const LinphoneCore *lc) {
	return lc->chat_deny_code != LinphoneReasonNone;
}

const bctbx_list_t *linphone_core_get_chat_rooms (LinphoneCore *lc) {
	if (lc->chat_rooms)
		bctbx_list_free_with_data(lc->chat_rooms, (bctbx_list_free_func)linphone_chat_room_unref);
	lc->chat_rooms = L_GET_RESOLVED_C_LIST_FROM_CPP_LIST(L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getChatRooms());
	return lc->chat_rooms;
}

static LinphoneChatRoom *linphone_chat_room_new (LinphoneCore *core, const LinphoneAddress *addr) {
	return L_GET_C_BACK_PTR(L_GET_CPP_PTR_FROM_C_OBJECT(core)->getOrCreateBasicChatRoom(
		*L_GET_CPP_PTR_FROM_C_OBJECT(addr),
		!!linphone_core_realtime_text_enabled(core)
	));
}

LinphoneChatRoom *_linphone_core_create_chat_room_from_call(LinphoneCall *call){
	LinphoneChatRoom *cr = linphone_chat_room_new(linphone_call_get_core(call),
		linphone_address_clone(linphone_call_get_remote_address(call)));
	linphone_chat_room_set_call(cr, call);
	return cr;
}

LinphoneChatRoom *linphone_core_get_chat_room (LinphoneCore *lc, const LinphoneAddress *peerAddr) {
	return L_GET_C_BACK_PTR(L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getOrCreateBasicChatRoom(*L_GET_CPP_PTR_FROM_C_OBJECT(peerAddr)));
}

LinphoneChatRoom *linphone_core_get_chat_room_2 (
	LinphoneCore *lc,
	const LinphoneAddress *peer_addr,
	const LinphoneAddress *local_addr
) {
	return L_GET_C_BACK_PTR(L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getOrCreateBasicChatRoom(LinphonePrivate::ConferenceId(
		LinphonePrivate::ConferenceAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(peer_addr)),
		LinphonePrivate::ConferenceAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(local_addr))
	)));
}

//Deprecated
LinphoneChatRoom *linphone_core_create_client_group_chat_room(LinphoneCore *lc, const char *subject, bool_t fallback) {
	return linphone_core_create_client_group_chat_room_2(lc, subject, fallback, FALSE);
}

//Deprecated
LinphoneChatRoom *linphone_core_create_client_group_chat_room_2(LinphoneCore *lc, const char *subject, bool_t fallback, bool_t encrypted) {
	return L_GET_C_BACK_PTR(L_GET_PRIVATE_FROM_C_OBJECT(lc)->createClientGroupChatRoom(L_C_TO_STRING(subject), !!fallback, !!encrypted));
}

LinphoneChatRoom *linphone_core_create_chat_room(LinphoneCore *lc, const LinphoneChatRoomParams *params, const LinphoneAddress *localAddr, const char *subject, const bctbx_list_t *participants) {
	return L_GET_C_BACK_PTR(L_GET_PRIVATE_FROM_C_OBJECT(lc)->createChatRoom(LinphonePrivate::ChatRoomParams::toCpp(params)->clone()->toSharedPtr(), LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(localAddr)), L_C_TO_STRING(subject), L_GET_CPP_LIST_FROM_C_LIST_2(participants, LinphoneAddress *, LinphonePrivate::IdentityAddress, [] (LinphoneAddress *addr) {
					return LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(addr));
				})));
}

LinphoneChatRoom *linphone_core_create_chat_room_2(LinphoneCore *lc, const LinphoneChatRoomParams *params, const char *subject, const bctbx_list_t *participants) {
	return L_GET_C_BACK_PTR(L_GET_PRIVATE_FROM_C_OBJECT(lc)->createChatRoom(LinphonePrivate::ChatRoomParams::toCpp(params)->clone()->toSharedPtr(), L_C_TO_STRING(subject), L_GET_CPP_LIST_FROM_C_LIST_2(participants, LinphoneAddress *, LinphonePrivate::IdentityAddress, [] (LinphoneAddress *addr) {
					return LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(addr));
				})));
}

LinphoneChatRoom *linphone_core_create_chat_room_3(LinphoneCore *lc, const char *subject, const bctbx_list_t *participants) {
	return L_GET_C_BACK_PTR(L_GET_PRIVATE_FROM_C_OBJECT(lc)->createChatRoom(L_C_TO_STRING(subject), L_GET_CPP_LIST_FROM_C_LIST_2(participants, LinphoneAddress *, LinphonePrivate::IdentityAddress, [] (LinphoneAddress *addr) {
					return LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(addr));
				})));
}

LinphoneChatRoom *linphone_core_create_chat_room_4(LinphoneCore *lc, const LinphoneChatRoomParams *params, const LinphoneAddress *localAddr, const LinphoneAddress *participant) {
	return L_GET_C_BACK_PTR(L_GET_PRIVATE_FROM_C_OBJECT(lc)->createChatRoom(LinphonePrivate::ChatRoomParams::toCpp(params)->clone()->toSharedPtr(), LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(localAddr)), LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(participant))));
}

LinphoneChatRoom *linphone_core_create_chat_room_5(LinphoneCore *lc, const LinphoneAddress *participant) {
	return L_GET_C_BACK_PTR(L_GET_PRIVATE_FROM_C_OBJECT(lc)->createChatRoom(LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(participant))));
}

LinphoneChatRoomParams *linphone_core_create_default_chat_room_params(LinphoneCore *lc) {
	auto params = LinphonePrivate::ChatRoomParams::getDefaults(L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getSharedFromThis());
	params->ref();
	return params->toC();
}

LinphoneChatRoom *_linphone_core_create_server_group_chat_room (LinphoneCore *lc, LinphonePrivate::SalCallOp *op) {
	return _linphone_server_group_chat_room_new(lc, op);
}

void linphone_core_delete_chat_room (LinphoneCore *, LinphoneChatRoom *cr) {
	L_GET_CPP_PTR_FROM_C_OBJECT(cr)->deleteFromDb();
}

LinphoneChatRoom *linphone_core_get_chat_room_from_uri(LinphoneCore *lc, const char *to) {
	return L_GET_C_BACK_PTR(L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getOrCreateBasicChatRoomFromUri(L_C_TO_STRING(to)));
}

LinphoneChatRoom *linphone_core_find_chat_room(
	const LinphoneCore *lc,
	const LinphoneAddress *peer_addr,
	const LinphoneAddress *local_addr
) {
	return L_GET_C_BACK_PTR(L_GET_CPP_PTR_FROM_C_OBJECT(lc)->findChatRoom(LinphonePrivate::ConferenceId(
		LinphonePrivate::ConferenceAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(peer_addr)),
		LinphonePrivate::ConferenceAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(local_addr))
	)));
}

LinphoneChatRoom *linphone_core_find_one_to_one_chat_room (
	const LinphoneCore *lc,
	const LinphoneAddress *local_addr,
	const LinphoneAddress *participant_addr
) {
	return L_GET_C_BACK_PTR(L_GET_CPP_PTR_FROM_C_OBJECT(lc)->findOneToOneChatRoom(
		LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(local_addr)),
		LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(participant_addr)),
		false, false)
	);
}

LinphoneChatRoom *linphone_core_find_one_to_one_chat_room_2 (
	const LinphoneCore *lc,
	const LinphoneAddress *local_addr,
	const LinphoneAddress *participant_addr,
	bool_t encrypted
) {
	return L_GET_C_BACK_PTR(L_GET_CPP_PTR_FROM_C_OBJECT(lc)->findOneToOneChatRoom(
		LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(local_addr)),
		LinphonePrivate::IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(participant_addr)),
		false, !!encrypted)
	);
}

LinphoneReason linphone_core_message_received(LinphoneCore *lc, LinphonePrivate::SalOp *op, const SalMessage *sal_msg) {
	LinphoneReason reason = LinphoneReasonNotAcceptable;
	std::string peerAddress;
	std::string localAddress;

	const char *session_mode = sal_custom_header_find(op->getRecvCustomHeaders(), "Session-mode");

	if (linphone_core_conference_server_enabled(lc)) {
		localAddress = peerAddress = op->getTo();
	} else {
		peerAddress = op->getFrom();
		localAddress = op->getTo();
	}

	LinphonePrivate::ConferenceId conferenceId{
		LinphonePrivate::ConferenceAddress(LinphonePrivate::Address(peerAddress)),
		LinphonePrivate::ConferenceAddress(LinphonePrivate::Address(localAddress))
	};
	shared_ptr<LinphonePrivate::AbstractChatRoom> chatRoom = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->findChatRoom(conferenceId);
	if (chatRoom)
		reason = L_GET_PRIVATE(chatRoom)->onSipMessageReceived(op, sal_msg);
	else if (!linphone_core_conference_server_enabled(lc)) {
		/* Client mode but check that it is really for basic chatroom before creating it.*/
		if (session_mode && strcasecmp(session_mode, "true") == 0) {
			lError() << "Message is received in the context of a client chatroom for which we have no context.";
			reason = LinphoneReasonNotAcceptable;
		} else {
			chatRoom = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->getOrCreateBasicChatRoom(conferenceId);
			if (chatRoom)
				reason = L_GET_PRIVATE(chatRoom)->onSipMessageReceived(op, sal_msg);
		}
	} else {
		/* Server mode but chatroom not found. */
		reason = LinphoneReasonNotFound;
	}
	return reason;
}

void linphone_core_real_time_text_received(LinphoneCore *lc, LinphoneChatRoom *cr, uint32_t character, LinphoneCall *call) {
	if (!(L_GET_CPP_PTR_FROM_C_OBJECT(cr)->getCapabilities() & LinphonePrivate::ChatRoom::Capabilities::RealTimeText))
		return;
	L_GET_PRIVATE_FROM_C_OBJECT(cr, RealTimeTextChatRoom)->realtimeTextReceived(character, LinphonePrivate::Call::toCpp(call)->getSharedFromThis());
}

unsigned int linphone_chat_message_store(LinphoneChatMessage *msg) {
	// DO nothing, just for old JNI compat...
	return 1;
}
