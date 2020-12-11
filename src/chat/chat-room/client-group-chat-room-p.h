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

#ifndef _L_CLIENT_GROUP_CHAT_ROOM_P_H_
#define _L_CLIENT_GROUP_CHAT_ROOM_P_H_

#include "chat/chat-room/chat-room-p.h"
#include "client-group-chat-room.h"
#include "utils/background-task.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ClientGroupChatRoomPrivate : public ChatRoomPrivate {
public:
	ClientGroupChatRoomPrivate(void) : ChatRoomPrivate(AbstractChatRoom::CapabilitiesMask({ChatRoom::Capabilities::Conference})) {};
	ClientGroupChatRoomPrivate(AbstractChatRoom::CapabilitiesMask value) : ChatRoomPrivate((value | ChatRoom::Capabilities::Conference)) {};

	virtual ~ClientGroupChatRoomPrivate() = default;

	std::list<IdentityAddress> cleanAddressesList (const std::list<IdentityAddress> &addresses) const;
	std::shared_ptr<CallSession> createSessionTo (Address sessionTo);
	std::shared_ptr<CallSession> createSession ();
	void notifyReceived (const std::string &body);
	void multipartNotifyReceived (const std::string &body);

	void confirmJoining (SalCallOp *op);
	void setCallSessionListener (CallSessionListener *listener);
	void setChatRoomListener (ChatRoomListener *listener) { chatRoomListener = listener; }

	void addOneToOneCapability ();
	unsigned int getLastNotifyId () const;

	// ChatRoomListener
	void onChatRoomInsertRequested (const std::shared_ptr<AbstractChatRoom> &chatRoom) override;
	void onChatRoomInsertInDatabaseRequested (const std::shared_ptr<AbstractChatRoom> &chatRoom) override;
	void onChatRoomDeleteRequested (const std::shared_ptr<AbstractChatRoom> &chatRoom) override;

	// CallSessionListener
	void onCallSessionSetReleased (const std::shared_ptr<CallSession> &session) override;
	void onCallSessionStateChanged (const std::shared_ptr<CallSession> &session, CallSession::State state, const std::string &message) override;

	void onChatRoomCreated (const Address &remoteContact);
	void onExhumedConference(const ConferenceId& oldConfId, const ConferenceId& newConfId);
	void onLocallyExhumedConference (const Address &remoteContact);
	void onRemotelyExhumedConference (SalCallOp *op);
	void sendChatMessage (const std::shared_ptr<ChatMessage> &chatMessage) override;

	// 1-1 exhume related
	const std::list<ConferenceId>& getPreviousConferenceIds() const {
		return previousConferenceIds;
	};
	void addConferenceIdToPreviousList(const ConferenceId& confId) {
		previousConferenceIds.push_back(confId);
	}
	void removeConferenceIdFromPreviousList(const ConferenceId& confId);

private:
	void acceptSession (const std::shared_ptr<CallSession> &session);

	CallSessionListener *callSessionListener = this;
	ChatRoomListener *chatRoomListener = this;
	bool deletionOnTerminationEnabled = false;
	bool listHandlerUsed = false;
	BackgroundTask bgTask { "Subscribe/notify of full state conference" };

	bool isEphemeral = false;
	long ephemeralLifetime = 86400;  //24 hours = 86400s

	// 1-1 exhume related
	bool localExhumePending = false;
	std::list<std::shared_ptr<ChatMessage>> pendingExhumeMessages;
	std::list<ConferenceId> previousConferenceIds;
	
	L_DECLARE_PUBLIC(ClientGroupChatRoom);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CLIENT_GROUP_CHAT_ROOM_P_H_
