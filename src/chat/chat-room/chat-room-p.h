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

#ifndef _L_CHAT_ROOM_P_H_
#define _L_CHAT_ROOM_P_H_

#include <ctime>

#include "abstract-chat-room-p.h"
#include "chat-room.h"
#include "chat/notification/imdn.h"
#include "chat/notification/is-composing.h"
#include "conference/conference-id.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ImdnMessage;
class IsComposingMessage;

class ChatRoomPrivate : public AbstractChatRoomPrivate, public IsComposingListener {
public:
	ChatRoomPrivate (AbstractChatRoom::CapabilitiesMask value) { capabilities = value; }
	virtual ~ChatRoomPrivate() = default;

	inline void setProxyChatRoom (AbstractChatRoom *value) { proxyChatRoom = value; }

	inline void setCreationTime (time_t creationTime) override {
		this->creationTime = creationTime;
	}

	inline void setLastUpdateTime (time_t lastUpdateTime) override {
		this->lastUpdateTime = lastUpdateTime;
	}

	void sendChatMessage (const std::shared_ptr<ChatMessage> &chatMessage) override;
	void onChatMessageSent (const std::shared_ptr<ChatMessage> &chatMessage) override;
	void sendIsComposingNotification ();

	void addEvent (const std::shared_ptr<EventLog> &eventLog) override;

	void addTransientEvent (const std::shared_ptr<EventLog> &eventLog) override;
	void removeTransientEvent (const std::shared_ptr<EventLog> &eventLog) override;

	void addTransientChatMessage (const std::shared_ptr<ChatMessage> &message) override;
	void removeTransientChatMessage (const std::shared_ptr<ChatMessage> &message) override;
	std::list<std::shared_ptr<ChatMessage>> getTransientChatMessages () override;

	void setIsEmpty (const bool empty) override;

	std::shared_ptr<ChatMessage> createChatMessage (ChatMessage::Direction direction);
	std::shared_ptr<ImdnMessage> createImdnMessage (
		const std::list<std::shared_ptr<ChatMessage>> &deliveredMessages,
		const std::list<std::shared_ptr<ChatMessage>> &displayedMessages
	);
	std::shared_ptr<ImdnMessage> createImdnMessage (const std::list<Imdn::MessageReason> &nonDeliveredMessages);
	std::shared_ptr<ImdnMessage> createImdnMessage (const std::shared_ptr<ImdnMessage> &message);
	std::shared_ptr<IsComposingMessage> createIsComposingMessage ();
	std::list<std::shared_ptr<ChatMessage>> findChatMessages (const std::string &messageId) const;

	void sendDeliveryErrorNotification (const std::shared_ptr<ChatMessage> &chatMessage, LinphoneReason reason);
	void sendDeliveryNotification (const std::shared_ptr<ChatMessage> &chatMessage);
	void sendDeliveryNotifications (const std::shared_ptr<ChatMessage> &chatMessage) override;
	void sendDisplayNotification (const std::shared_ptr<ChatMessage> &chatMessage);

	void notifyChatMessageReceived (const std::shared_ptr<ChatMessage> &chatMessage) override;
	void notifyIsComposingReceived (const Address &remoteAddress, bool isComposing);
	void notifyStateChanged ();
	void notifyUndecryptableChatMessageReceived (const std::shared_ptr<ChatMessage> &chatMessage) override;

	LinphoneReason onSipMessageReceived (SalOp *op, const SalMessage *message) override;
	void onChatMessageReceived (const std::shared_ptr<ChatMessage> &chatMessage) override;
	void onImdnReceived (const std::shared_ptr<ChatMessage> &chatMessage);
	void onIsComposingReceived (const Address &remoteAddress, const std::string &text);
	void onIsComposingRefreshNeeded () override;
	void onIsComposingStateChanged (bool isComposing) override;
	void onIsRemoteComposingStateChanged (const Address &remoteAddress, bool isComposing) override;

	void realtimeTextReceived (uint32_t character, const std::shared_ptr<Call> &call) override;
	void setCallId (const std::string &value) override { callId = value; };

	Imdn *getImdnHandler () const { return imdnHandler.get(); }

	LinphoneChatRoom *getCChatRoom () const;

	std::list<IdentityAddress> remoteIsComposing;
	std::list<std::shared_ptr<EventLog>> transientEvents;
	std::list<std::shared_ptr<ChatMessage>> transientMessages;

	std::shared_ptr<ChatRoomParams> params;

protected:
	AbstractChatRoom *proxyChatRoom = nullptr;
	AbstractChatRoom::CapabilitiesMask capabilities;

private:

	time_t creationTime = std::time(nullptr);
	time_t lastUpdateTime = std::time(nullptr);

	std::unique_ptr<Imdn> imdnHandler;
	std::unique_ptr<IsComposing> isComposingHandler;

	bool isComposing = false;
	bool isEmpty = true;
	
	std::string callId;
	size_t readCharacterIndex = 0;
	std::vector<uint32_t> receivedRttCharacters;
	std::vector<uint32_t> lastMessageCharacters;

	L_DECLARE_PUBLIC(ChatRoom);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CHAT_ROOM_P_H_
