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

#include "basic-to-client-group-chat-room.h"
#include "proxy-chat-room-p.h"
#include "client-group-chat-room-p.h"
#include "chat-room-params.h"
#include "chat/chat-message/chat-message-p.h"
#include "conference/participant.h"
#include "conference/session/call-session.h"
#include "core/core-p.h"
#include "c-wrapper/c-wrapper.h"

#include "linphone/utils/algorithm.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

class BasicToClientGroupChatRoomPrivate : public ProxyChatRoomPrivate {
public:
	void onChatRoomInsertRequested (const shared_ptr<AbstractChatRoom> &chatRoom) override {
		L_Q();
		// Insert the client group chat room temporarily
		q->getCore()->getPrivate()->insertChatRoom(chatRoom);
	}

	void onChatRoomInsertInDatabaseRequested (const shared_ptr<AbstractChatRoom> &chatRoom) override {
		// Do not insert the client group chat room in database, the migration will do it
	}

	void onChatRoomDeleteRequested (const shared_ptr<AbstractChatRoom> &chatRoom) override {
		L_Q();
		q->getCore()->deleteChatRoom(q->getSharedFromThis());
		q->setState(ConferenceInterface::State::Deleted);
	}

	void sendChatMessage (const shared_ptr<ChatMessage> &chatMessage) override {
		L_Q();

		ProxyChatRoomPrivate::sendChatMessage(chatMessage);
		const list<string> &specs = chatMessage->getCore()->getSpecsList();
		time_t currentRealTime = ms_time(nullptr);
		LinphoneAddress *lAddr = linphone_address_new(
			chatMessage->getChatRoom()->getConferenceId().getLocalAddress().asString().c_str()
		);
		LinphoneProxyConfig *proxy = linphone_core_lookup_known_proxy(q->getCore()->getCCore(), lAddr);
		linphone_address_unref(lAddr);
		const char *conferenceFactoryUri = nullptr;
		if (proxy) {
			conferenceFactoryUri = linphone_proxy_config_get_conference_factory_uri(proxy);
		}
		if (!conferenceFactoryUri
		    || (chatRoom->getCapabilities() & ChatRoom::Capabilities::Conference)
		    || clientGroupChatRoom
		    || findIf(specs, [] (const string &spec) { return spec.find("groupchat") != string::npos;}) == specs.cend()
		    || ((currentRealTime - migrationRealTime) <
			linphone_config_get_int(linphone_core_get_config(chatMessage->getCore()->getCCore()),
						"misc", "basic_to_client_group_chat_room_migration_timer", 86400) // Try migration every 24 hours
			)
   	        ) {
			return;
		}
		migrationRealTime = currentRealTime;
		char *tmp = linphone_address_as_string(linphone_proxy_config_get_contact(proxy)); //get the gruu address
		IdentityAddress localAddress(tmp);
		bctbx_free(tmp);
		clientGroupChatRoom = static_pointer_cast<ClientGroupChatRoom>(
		       //make sure to have a one2one chatroom
		       chatRoom->getCore()->getPrivate()->createChatRoom(
			   ChatRoomParams::create(chatRoom->getCapabilities() & ChatRoom::Capabilities::Encrypted, false, ChatRoomParams::ChatRoomBackend::FlexisipChat), localAddress, chatRoom->getSubject(), {Address(chatRoom->getPeerAddress())}
		       )
		);
		clientGroupChatRoom->getPrivate()->setCallSessionListener(this);
		clientGroupChatRoom->getPrivate()->setChatRoomListener(this);
	}

	void onCallSessionStateChanged (
		const shared_ptr<CallSession> &session,
		CallSession::State newState,
		const string &message
	) override {
		if (!clientGroupChatRoom)
			return;
		if ((newState == CallSession::State::Error) && (clientGroupChatRoom->getState() == ConferenceInterface::State::CreationPending)) {
			Core::deleteChatRoom(clientGroupChatRoom);
			if (session->getReason() == LinphoneReasonNotAcceptable) {
				clientGroupChatRoom = nullptr;
				return;
			}
		}
		clientGroupChatRoom->getPrivate()->onCallSessionStateChanged(session, newState, message);
	}

private:
	shared_ptr<ClientGroupChatRoom> clientGroupChatRoom;
	time_t migrationRealTime = 0;

	L_DECLARE_PUBLIC(BasicToClientGroupChatRoom);
};

// =============================================================================

BasicToClientGroupChatRoom::BasicToClientGroupChatRoom (const shared_ptr<ChatRoom> &chatRoom) :
	ProxyChatRoom(*new BasicToClientGroupChatRoomPrivate, chatRoom) {}

BasicToClientGroupChatRoom::CapabilitiesMask BasicToClientGroupChatRoom::getCapabilities () const {
	L_D();
	CapabilitiesMask capabilities = d->chatRoom->getCapabilities();
	capabilities.set(Capabilities::Proxy);
	if (capabilities.isSet(Capabilities::Basic)) {
		capabilities.set(Capabilities::Migratable);
	} else {
		capabilities.set(Capabilities::Conference);
	}
	return capabilities;
}

shared_ptr<ChatMessage> BasicToClientGroupChatRoom::createChatMessage () {
	shared_ptr<ChatMessage> msg = ProxyChatRoom::createChatMessage();
	msg->getPrivate()->setChatRoom(getSharedFromThis());
	return msg;
}

shared_ptr<ChatMessage> BasicToClientGroupChatRoom::createChatMessage (const string &text) {
	shared_ptr<ChatMessage> msg = ProxyChatRoom::createChatMessage(text);
	msg->getPrivate()->setChatRoom(getSharedFromThis());
	return msg;
}

void BasicToClientGroupChatRoom::migrate(const std::shared_ptr<ClientGroupChatRoom> &clientGroupChatRoom, const std::shared_ptr<AbstractChatRoom> &chatRoom) {
	clientGroupChatRoom->getCore()->getPrivate()->mainDb->migrateBasicToClientGroupChatRoom(chatRoom, clientGroupChatRoom);

	if (chatRoom->getCapabilities() & ChatRoom::Capabilities::Proxy) {
		shared_ptr<BasicToClientGroupChatRoom> btcgcr = static_pointer_cast<BasicToClientGroupChatRoom>(chatRoom);
		btcgcr->getCore()->getPrivate()->replaceChatRoom(chatRoom, clientGroupChatRoom);
		btcgcr->getPrivate()->chatRoom = clientGroupChatRoom;
		btcgcr->getPrivate()->setupProxy();
	} else {
		LinphoneChatRoom *lcr = L_GET_C_BACK_PTR(chatRoom);
		L_SET_CPP_PTR_FROM_C_OBJECT(lcr, clientGroupChatRoom);
		clientGroupChatRoom->getCore()->getPrivate()->replaceChatRoom(chatRoom, clientGroupChatRoom);
	}
}

LINPHONE_END_NAMESPACE
