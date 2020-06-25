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

#ifndef _L_CHAT_ROOM_H_
#define _L_CHAT_ROOM_H_

#include "chat-room-params.h"
#include "abstract-chat-room.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ChatRoomPrivate;

class LINPHONE_PUBLIC ChatRoom : public AbstractChatRoom {
public:
	friend class ChatMessagePrivate;
	friend class Imdn;
	friend class ImdnMessagePrivate;
	friend class ProxyChatRoomPrivate;
	friend class CorePrivate;

	L_OVERRIDE_SHARED_FROM_THIS(ChatRoom);

	~ChatRoom ();

	const IdentityAddress &getPeerAddress () const override;
	const IdentityAddress &getLocalAddress () const override;

	time_t getCreationTime () const override;
	time_t getLastUpdateTime () const override;

	State getState () const override;
	void setState (ConferenceInterface::State newState) override;

	SecurityLevel getSecurityLevel () const override;

	std::list<std::shared_ptr<EventLog>> getMessageHistory (int nLast) const override;
	std::list<std::shared_ptr<EventLog>> getMessageHistoryRange (int begin, int end) const override;
	int getMessageHistorySize () const override;
	std::list<std::shared_ptr<EventLog>> getHistory (int nLast) const override;
	std::list<std::shared_ptr<EventLog>> getHistoryRange (int begin, int end) const override;
	int getHistorySize () const override;

	void deleteFromDb () override;
	void deleteHistory () override;
	void deleteMessageFromHistory (const std::shared_ptr<ChatMessage> &message) override;

	std::shared_ptr<ChatMessage> getLastChatMessageInHistory () const override;
	bool isEmpty () const override;
	int getChatMessageCount () const override;
	int getUnreadChatMessageCount () const override;

	void compose () override;
	bool isRemoteComposing () const override;
	std::list<IdentityAddress> getComposingAddresses () const override;

	std::shared_ptr<ChatMessage> createChatMessage () override;
	std::shared_ptr<ChatMessage> createChatMessage (const std::string &text) override;

	std::shared_ptr<ChatMessage> createFileTransferMessage (FileContent *content) override;
	std::shared_ptr<ChatMessage> createForwardMessage (const std::shared_ptr<ChatMessage> &msg) override;

	std::shared_ptr<ChatMessage> findChatMessage (const std::string &messageId) const override;
	std::shared_ptr<ChatMessage> findChatMessage (
		const std::string &messageId,
		ChatMessage::Direction direction
	) const override;

	void markAsRead () override;
	void enableEphemeral (bool ephem, bool updateDb) override;
	bool ephemeralEnabled () const override;
	void setEphemeralLifetime (long lifetime, bool updateDb) override;
	long getEphemeralLifetime () const override;
	bool ephemeralSupportedByAllParticipants () const override;

	const std::shared_ptr<ChatRoomParams> &getCurrentParams() const override;

	void addListener(std::shared_ptr<ConferenceListenerInterface> listener) override;

	virtual std::shared_ptr<Conference> getConference () const {return conference;};

	static std::list<IdentityAddress> parseResourceLists (const Content &content);

protected:
	explicit ChatRoom (ChatRoomPrivate &p, const std::shared_ptr<Core> &core, const std::shared_ptr<ChatRoomParams> &params, const std::shared_ptr<Conference> &conf = nullptr);

	std::shared_ptr<Conference> conference = nullptr;

private:
	L_DECLARE_PRIVATE(ChatRoom);
	L_DISABLE_COPY(ChatRoom);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CHAT_ROOM_H_
