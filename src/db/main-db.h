/*
 * main-db.h
 * Copyright (C) 2010-2018 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _L_MAIN_DB_H_
#define _L_MAIN_DB_H_

#include <functional>

#include "linphone/utils/enum-mask.h"

#include "abstract/abstract-db.h"
#include "chat/chat-message/chat-message.h"
#include "conference/conference-id.h"
#include "core/core-accessor.h"
#include "chat/chat-message/chat-message-killer.h"
#include <unordered_map>

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class AbstractChatRoom;
class ChatMessage;
class Core;
class EventLog;
class MainDbKey;
class MainDbPrivate;
class ParticipantDevice;

class LINPHONE_INTERNAL_PUBLIC MainDb : public AbstractDb, public CoreAccessor {
	template<typename Function>
	friend class DbTransaction;

	friend class MainDbChatMessageKey;
	friend class MainDbEventKey;

public:
	enum Filter {
		NoFilter = 0x0,
		ConferenceCallFilter = 1 << 0,
		ConferenceChatMessageFilter = 1 << 1,
		ConferenceInfoFilter = 1 << 2,
		ConferenceInfoNoDeviceFilter = 1 << 3,
		ConferenceChatMessageSecurityFilter = 1 << 4
	};

	typedef EnumMask<Filter> FilterMask;

	struct ParticipantState {
		ParticipantState (const IdentityAddress &address, ChatMessage::State state, time_t timestamp)
			: address(address), state(state), timestamp(timestamp) {}

		IdentityAddress address;
		ChatMessage::State state = ChatMessage::State::Idle;
		time_t timestamp = 0;
	};

	MainDb (const std::shared_ptr<Core> &core);

	// ---------------------------------------------------------------------------
	// Generic.
	// ---------------------------------------------------------------------------

	bool addEvent (const std::shared_ptr<EventLog> &eventLog);
	bool updateEvent (const std::shared_ptr<EventLog> &eventLog);
	static bool deleteEvent (const std::shared_ptr<const EventLog> &eventLog);
	int getEventCount (FilterMask mask = NoFilter) const;

	static std::shared_ptr<EventLog> getEventFromKey (const MainDbKey &dbKey);

	// ---------------------------------------------------------------------------
	// Conference notified events.
	// ---------------------------------------------------------------------------

	std::list<std::shared_ptr<EventLog>> getConferenceNotifiedEvents (
		const ConferenceId &conferenceId,
		unsigned int lastNotifyId
	) const;

	// ---------------------------------------------------------------------------
	// Conference chat message events.
	// ---------------------------------------------------------------------------

	using ParticipantStateRetrievalFunc = std::function<std::list<ParticipantState>(const std::shared_ptr<EventLog> &eventLog)>;

	int getChatMessageCount (const ConferenceId &conferenceId = ConferenceId()) const;
	int getUnreadChatMessageCount (const ConferenceId &conferenceId = ConferenceId()) const;

	void markChatMessagesAsRead (const ConferenceId &conferenceId) const;
	std::list<std::shared_ptr<ChatMessage>> getUnreadChatMessages (const ConferenceId &conferenceId) const;
	void setChatMessagesEphemeralStartTime (const ConferenceId &conferenceId, time_t &time) const;

	std::list<ParticipantState> getChatMessageParticipantsByImdnState (
		const std::shared_ptr<EventLog> &eventLog,
		ChatMessage::State state
	) const;
	std::list<ChatMessage::State> getChatMessageParticipantStates (const std::shared_ptr<EventLog> &eventLog) const;
	ChatMessage::State getChatMessageParticipantState (
		const std::shared_ptr<EventLog> &eventLog,
		const IdentityAddress &participantAddress
	) const;
	void setChatMessageParticipantState (
		const std::shared_ptr<EventLog> &eventLog,
		const IdentityAddress &participantAddress,
		ChatMessage::State state,
		time_t stateChangeTime
	);
	
	void updateEphemeralMessageKillers (std::unordered_map<MainDbEventKey, std::shared_ptr<ChatMessageKiller>> &messageKillers);

	std::shared_ptr<ChatMessage> getLastChatMessage (const ConferenceId &conferenceId) const;

	std::list<std::shared_ptr<ChatMessage>> findChatMessages (
		const ConferenceId &conferenceId,
		const std::string &imdnMessageId
	) const;

	std::list<std::shared_ptr<ChatMessage>> findChatMessagesToBeNotifiedAsDelivered () const;

	// ---------------------------------------------------------------------------
	// Conference events.
	// ---------------------------------------------------------------------------

	std::list<std::shared_ptr<EventLog>> getHistory (
		const ConferenceId &conferenceId,
		int nLast,
		FilterMask mask = NoFilter
	) const;
	std::list<std::shared_ptr<EventLog>> getHistoryRange (
		const ConferenceId &conferenceId,
		int begin,
		int end,
		FilterMask mask = NoFilter
	) const;

	int getHistorySize (const ConferenceId &conferenceId, FilterMask mask = NoFilter) const;

	void cleanHistory (const ConferenceId &conferenceId, FilterMask mask = NoFilter);

	// ---------------------------------------------------------------------------
	// Chat messages.
	// ---------------------------------------------------------------------------

	void loadChatMessageContents (const std::shared_ptr<ChatMessage> &chatMessage);

	void disableDeliveryNotificationRequired (const std::shared_ptr<const EventLog> &eventLog);
	void disableDisplayNotificationRequired (const std::shared_ptr<const EventLog> &eventLog);

	// ---------------------------------------------------------------------------
	// Chat rooms.
	// ---------------------------------------------------------------------------

	std::list<std::shared_ptr<AbstractChatRoom>> getChatRooms () const;
	void insertChatRoom (const std::shared_ptr<AbstractChatRoom> &chatRoom, unsigned int notifyId = 0);
	void deleteChatRoom (const ConferenceId &conferenceId);
	void enableChatRoomMigration (const ConferenceId &conferenceId, bool enable);

	void migrateBasicToClientGroupChatRoom (
		const std::shared_ptr<AbstractChatRoom> &basicChatRoom,
		const std::shared_ptr<AbstractChatRoom> &clientGroupChatRoom
	);

	IdentityAddress findMissingOneToOneConferenceChatRoomParticipantAddress (
		const std::shared_ptr<AbstractChatRoom> &chatRoom,
		const IdentityAddress &presentParticipantAddr
	);
	IdentityAddress findOneToOneConferenceChatRoomAddress (
		const IdentityAddress &participantA,
		const IdentityAddress &participantB,
		bool encrypted
	) const;
	void insertOneToOneConferenceChatRoom (const std::shared_ptr<AbstractChatRoom> &chatRoom, bool encrypted);

	void updateChatRoomParticipantDevice (
		const std::shared_ptr<AbstractChatRoom> &chatRoom,
		const std::shared_ptr<ParticipantDevice> &device
	);

	void deleteChatRoomParticipant (
		const std::shared_ptr<AbstractChatRoom> &chatRoom,
		const IdentityAddress &participant
	);
	
	void deleteChatRoomParticipantDevice (
		const std::shared_ptr<AbstractChatRoom> &chatRoom,
		const std::shared_ptr<ParticipantDevice> &device
	);

	// ---------------------------------------------------------------------------
	// Other.
	// ---------------------------------------------------------------------------

	// Import legacy calls/messages from old db.
	bool import (Backend backend, const std::string &parameters) override;

protected:
	void init () override;

private:
	L_DECLARE_PRIVATE(MainDb);
	L_DISABLE_COPY(MainDb);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_MAIN_DB_H_
