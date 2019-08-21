/*
 * main-db-p.h
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

#ifndef _L_MAIN_DB_P_H_
#define _L_MAIN_DB_P_H_

#include <unordered_map>

#include "linphone/utils/utils.h"

#include "abstract/abstract-db-p.h"
#include "containers/lru-cache.h"
#include "event-log/event-log.h"
#include "main-db.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Content;

class MainDbPrivate : public AbstractDbPrivate {
public:
	mutable std::unordered_map<long long, std::weak_ptr<EventLog>> storageIdToEvent;
	mutable std::unordered_map<long long, std::weak_ptr<ChatMessage>> storageIdToChatMessage;
	mutable std::unordered_map<long long, ConferenceId> storageIdToConferenceId;

private:
	// ---------------------------------------------------------------------------
	// Misc helpers.
	// ---------------------------------------------------------------------------

	std::shared_ptr<AbstractChatRoom> findChatRoom (const ConferenceId &conferenceId) const;

	// ---------------------------------------------------------------------------
	// Low level API.
	// ---------------------------------------------------------------------------

	long long insertSipAddress (const std::string &sipAddress);
	void insertContent (long long chatMessageId, const Content &content);
	long long insertContentType (const std::string &contentType);
	long long insertOrUpdateImportedBasicChatRoom (
		long long peerSipAddressId,
		long long localSipAddressId,
		const tm &creationTime
	);
	long long insertChatRoom (const std::shared_ptr<AbstractChatRoom> &chatRoom, unsigned int notifyId = 0);
	long long insertChatRoomParticipant (long long chatRoomId, long long participantSipAddressId, bool isAdmin);
	void insertChatRoomParticipantDevice (long long participantId, long long participantDeviceSipAddressId, const std::string &deviceName);
	void insertChatMessageParticipant (long long chatMessageId, long long sipAddressId, int state, time_t stateChangeTime);

	long long selectSipAddressId (const std::string &sipAddress) const;
	long long selectChatRoomId (long long peerSipAddressId, long long localSipAddressId) const;
	long long selectChatRoomId (const ConferenceId &conferenceId) const;
	ConferenceId selectConferenceId (const long long chatRoomId) const;
	long long selectChatRoomParticipantId (long long chatRoomId, long long participantSipAddressId) const;
	long long selectOneToOneChatRoomId (long long sipAddressIdA, long long sipAddressIdB, bool encrypted) const;

	void deleteContents (long long chatMessageId);
	void deleteChatRoomParticipant (long long chatRoomId, long long participantSipAddressId);
	void deleteChatRoomParticipantDevice (long long participantId, long long participantDeviceSipAddressId);

	// ---------------------------------------------------------------------------
	// Events API.
	// ---------------------------------------------------------------------------

#ifdef HAVE_DB_STORAGE
	long long getConferenceEventIdFromRow (const soci::row &row) const {
		return dbSession.resolveId(row, 0);
	}

	time_t getConferenceEventCreationTimeFromRow (const soci::row &row) const {
		return Utils::getTmAsTimeT(row.get<tm>(2));
	}

	unsigned int getConferenceEventNotifyIdFromRow (const soci::row &row) const {
		L_Q();
		return q->getBackend() == MainDb::Backend::Mysql
			? row.get<unsigned int>(10, 0)
			: static_cast<unsigned int>(row.get<int>(10, 0));
	}

	std::shared_ptr<EventLog> selectGenericConferenceEvent (
		const std::shared_ptr<AbstractChatRoom> &chatRoom,
		const soci::row &row
	) const;

	std::shared_ptr<EventLog> selectConferenceInfoEvent (
		const ConferenceId &conferenceId,
		const soci::row &row
	) const;

	std::shared_ptr<EventLog> selectConferenceEvent (
		const ConferenceId &conferenceId,
		EventLog::Type type,
		const soci::row &row
	) const;

	std::shared_ptr<EventLog> selectConferenceCallEvent (
		const ConferenceId &conferenceId,
		EventLog::Type type,
		const soci::row &row
	) const;

	std::shared_ptr<EventLog> selectConferenceChatMessageEvent (
		const std::shared_ptr<AbstractChatRoom> &chatRoom,
		EventLog::Type type,
		const soci::row &row
	) const;

	std::shared_ptr<EventLog> selectConferenceParticipantEvent (
		const ConferenceId &conferenceId,
		EventLog::Type type,
		const soci::row &row
	) const;

	std::shared_ptr<EventLog> selectConferenceParticipantDeviceEvent (
		const ConferenceId &conferenceId,
		EventLog::Type type,
		const soci::row &row
	) const;

	std::shared_ptr<EventLog> selectConferenceSecurityEvent (
		const ConferenceId &conferenceId,
		EventLog::Type type,
		const soci::row &row
	) const;

	std::shared_ptr<EventLog> selectConferenceSubjectEvent (
		const ConferenceId &conferenceId,
		EventLog::Type type,
		const soci::row &row
	) const;
#endif

	long long insertEvent (const std::shared_ptr<EventLog> &eventLog);
	long long insertConferenceEvent (const std::shared_ptr<EventLog> &eventLog, long long *chatRoomId = nullptr);
	long long insertConferenceCallEvent (const std::shared_ptr<EventLog> &eventLog);
	long long insertConferenceChatMessageEvent (const std::shared_ptr<EventLog> &eventLog);
	void updateConferenceChatMessageEvent(const std::shared_ptr<EventLog> &eventLog);
	long long insertConferenceNotifiedEvent (const std::shared_ptr<EventLog> &eventLog, long long *chatRoomId = nullptr);
	long long insertConferenceParticipantEvent (const std::shared_ptr<EventLog> &eventLog, long long *chatRoomId = nullptr);
	long long insertConferenceParticipantDeviceEvent (const std::shared_ptr<EventLog> &eventLog);
	long long insertConferenceSubjectEvent (const std::shared_ptr<EventLog> &eventLog);
	long long insertConferenceSecurityEvent (const std::shared_ptr<EventLog> &eventLog);

	void setChatMessageParticipantState (
		const std::shared_ptr<EventLog> &eventLog,
		const IdentityAddress &participantAddress,
		ChatMessage::State state,
		time_t stateChangeTime
	);

	// ---------------------------------------------------------------------------
	// Cache API.
	// ---------------------------------------------------------------------------

	void cache (const std::shared_ptr<EventLog> &eventLog, long long storageId) const;
	void cache (const std::shared_ptr<ChatMessage> &chatMessage, long long storageId) const;
	void cache (const ConferenceId &conferenceId, long long storageId) const;

	std::shared_ptr<EventLog> getEventFromCache (long long storageId) const;
	std::shared_ptr<ChatMessage> getChatMessageFromCache (long long storageId) const;
	ConferenceId getConferenceIdFromCache(long long storageId) const;

	void invalidConferenceEventsFromQuery (const std::string &query, long long chatRoomId);

	// ---------------------------------------------------------------------------
	// Versions.
	// ---------------------------------------------------------------------------

	unsigned int getModuleVersion (const std::string &name);
	void updateModuleVersion (const std::string &name, unsigned int version);
	void updateSchema ();

	// ---------------------------------------------------------------------------
	// Import.
	// ---------------------------------------------------------------------------

#ifdef HAVE_DB_STORAGE
	void importLegacyFriends (DbSession &inDbSession);
	void importLegacyHistory (DbSession &inDbSession);
#endif

	// ---------------------------------------------------------------------------

	mutable LruCache<ConferenceId, int> unreadChatMessageCountCache;

	L_DECLARE_PUBLIC(MainDb);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_MAIN_DB_P_H_
