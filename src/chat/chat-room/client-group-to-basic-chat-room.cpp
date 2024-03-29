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

#include "client-group-to-basic-chat-room.h"
#include "c-wrapper/c-wrapper.h"
#include "client-group-chat-room-p.h"
#include "conference/session/call-session.h"
#include "core/core-p.h"
#include "proxy-chat-room-p.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

class ClientGroupToBasicChatRoomPrivate : public ProxyChatRoomPrivate {
public:
	void onChatRoomInsertRequested(BCTBX_UNUSED(const shared_ptr<AbstractChatRoom> &chatRoom)) override {
		L_Q();
		// Insert the proxy chat room instead of the real one
		q->getCore()->getPrivate()->insertChatRoom(q->getSharedFromThis());
	}

	void onChatRoomInsertInDatabaseRequested(const shared_ptr<AbstractChatRoom> &chatRoom) override {
		L_Q();
		// Insert the proxy chat room instead of the real one
		unsigned int notifyId = static_cast<ClientGroupChatRoomPrivate *>(chatRoom->getPrivate())->getLastNotifyId();
		q->getCore()->getPrivate()->insertChatRoomWithDb(q->getSharedFromThis(), notifyId);
	}

	void onChatRoomDeleteRequested(BCTBX_UNUSED(const shared_ptr<AbstractChatRoom> &chatRoom)) override {
		L_Q();
		// Keep a ref, otherwise the object might be destroyed before we can set the Deleted state
		shared_ptr<AbstractChatRoom> ref = q->getSharedFromThis();
		q->getCore()->deleteChatRoom(ref);
		q->setState(ConferenceInterface::State::Deleted);
	}

	void onCallSessionSetReleased(const shared_ptr<CallSession> &session) override {
		if (!(chatRoom->getCapabilities() & ChatRoom::Capabilities::Conference)) return;
		static_pointer_cast<ClientGroupChatRoom>(chatRoom)->getPrivate()->onCallSessionSetReleased(session);
	}

	void onCallSessionStateChanged(const shared_ptr<CallSession> &session,
	                               CallSession::State newState,
	                               const string &message) override {
		L_Q();
		// Keep a ref, otherwise the object might be destroyed when calling Core::deleteChatRoom()
		shared_ptr<AbstractChatRoom> ref = q->getSharedFromThis();
		// TODO: Avoid cast, use capabilities.
		shared_ptr<ClientGroupChatRoom> cgcr = dynamic_pointer_cast<ClientGroupChatRoom>(chatRoom);
		if (!cgcr) return;
		if ((newState == CallSession::State::Error) &&
		    (cgcr->getState() == ConferenceInterface::State::CreationPending) &&
		    (session->getReason() == LinphoneReasonNotAcceptable) && (invitedAddresses.size() == 1)) {
			teardownProxy();
			cgcr->getPrivate()->onCallSessionStateChanged(session, newState, message);
			cgcr->getPrivate()->setCallSessionListener(nullptr);
			cgcr->getPrivate()->setChatRoomListener(nullptr);
			Core::deleteChatRoom(q->getSharedFromThis());

			LinphoneChatRoom *lcr = L_GET_C_BACK_PTR(q);
			shared_ptr<AbstractChatRoom> bcr =
			    cgcr->getCore()->getOrCreateBasicChatRoom(q->getLocalAddress(), invitedAddresses.front());
			L_SET_CPP_PTR_FROM_C_OBJECT(lcr, bcr);
			/* getOrCreateBasicChatRoom will automatically set the state to Instantiated and Created
			 * but because CPP ptr hasn't been set yet in this case the application's ChatRoom won't be notified
			 * that's why we set both states again here... */
			bcr->setState(ConferenceInterface::State::Instantiated);
			bcr->setState(ConferenceInterface::State::Created);
			return;
		}
		cgcr->getPrivate()->onCallSessionStateChanged(session, newState, message);
	}

private:
	list<std::shared_ptr<Address>> invitedAddresses;

	L_DECLARE_PUBLIC(ClientGroupToBasicChatRoom);
};

// =============================================================================

ClientGroupToBasicChatRoom::ClientGroupToBasicChatRoom(const shared_ptr<ChatRoom> &chatRoom)
    : ProxyChatRoom(*new ClientGroupToBasicChatRoomPrivate, chatRoom) {
}

bool ClientGroupToBasicChatRoom::addParticipant(const std::shared_ptr<Address> &participantAddress) {
	L_D();
	if (getState() == ConferenceInterface::State::Instantiated) {
		d->invitedAddresses.clear();
		d->invitedAddresses.push_back(participantAddress);
	}
	return ProxyChatRoom::addParticipant(participantAddress);
}

bool ClientGroupToBasicChatRoom::addParticipants(const list<std::shared_ptr<Address>> &addresses) {
	L_D();
	if ((getState() == ConferenceInterface::State::Instantiated) && (addresses.size() == 1))
		d->invitedAddresses = addresses;
	return ProxyChatRoom::addParticipants(addresses);
}

LINPHONE_END_NAMESPACE
