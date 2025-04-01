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

#ifndef _L_MAIN_DB_P_H_
#define _L_MAIN_DB_P_H_

#include <unordered_map>

#include "linphone/utils/utils.h"

#include "abstract/abstract-db-p.h"
#include "conference/participant-info.h"
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
	mutable std::unordered_map<long long, std::weak_ptr<CallLog>> storageIdToCallLog;
	mutable std::unordered_map<long long, std::weak_ptr<ConferenceInfo>> storageIdToConferenceInfo;

private:
	// ---------------------------------------------------------------------------
	// Misc helpers.
	// ---------------------------------------------------------------------------

	std::shared_ptr<AbstractChatRoom> findChatRoom(const ConferenceId &conferenceId) const;
	std::shared_ptr<Conference> findConference(const ConferenceId &conferenceId) const;

	// ---------------------------------------------------------------------------
	// Low level API.
	// ---------------------------------------------------------------------------
	long long insertSipAddress(const Address &address);
	long long insertSipAddress(const std::shared_ptr<Address> &address);
	long long insertSipAddress(const std::string &sipAddress, const std::string &displayName);
	void insertContent(long long chatMessageId, const Content &content);
	long long insertContentType(const std::string &contentType);
	long long
	insertOrUpdateImportedBasicChatRoom(long long peerSipAddressId, long long localSipAddressId, const time_t &time);
	long long insertChatRoom(const std::shared_ptr<AbstractChatRoom> &chatRoom, unsigned int notifyId = 0);
	long long insertChatRoomParticipant(long long chatRoomId, long long participantSipAddressId, bool isAdmin);
	void insertChatRoomParticipantDevice(long long participantId, const std::shared_ptr<ParticipantDevice> &device);
	void insertChatRoomParticipantDevice(long long participantId,
	                                     long long participantDeviceSipAddressId,
	                                     const std::string &deviceName);
	void
	insertChatMessageParticipant(long long chatMessageId, long long sipAddressId, int state, time_t stateChangeTime);
	ParticipantInfo::participant_params_t selectConferenceInfoParticipantParams(const long long participantId) const;
	ParticipantInfo::participant_params_t
	migrateConferenceInfoParticipantParams(const ParticipantInfo::participant_params_t &unprocessedParticipantParams,
	                                       const long long participantId) const;
	long long insertConferenceInfo(const std::shared_ptr<ConferenceInfo> &conferenceInfo,
	                               const std::shared_ptr<ConferenceInfo> &oldConferenceInfo);
	long long insertOrUpdateConferenceInfoParticipant(long long conferenceInfoId,
	                                                  const std::shared_ptr<ParticipantInfo> &participantInfo,
	                                                  bool deleted,
	                                                  bool isOrganizer,
	                                                  bool isParticipant);
	long long insertOrUpdateConferenceInfoParticipant(long long conferenceInfoId,
	                                                  long long participantSipAddressId,
	                                                  bool deleted,
	                                                  const ParticipantInfo::participant_params_t params,
	                                                  bool isOrganizer,
	                                                  bool isParticipant,
	                                                  const std::string ccmpUri);
	long long insertOrUpdateConferenceInfoOrganizer(long long conferenceInfoId,
	                                                const std::shared_ptr<ParticipantInfo> &organizer,
	                                                bool isParticipant);
	long long insertOrUpdateConferenceInfoOrganizer(long long conferenceInfoId,
	                                                long long organizerSipAddressId,
	                                                const ParticipantInfo::participant_params_t params,
	                                                bool isParticipant,
	                                                const std::string ccmpUri);
	void insertOrUpdateConferenceInfoParticipantParams(long long conferenceInfoParticipantId,
	                                                   const ParticipantInfo::participant_params_t params) const;

	long long insertOrUpdateConferenceCall(const std::shared_ptr<CallLog> &callLog,
	                                       const std::shared_ptr<ConferenceInfo> &conferenceInfo = nullptr);
	long long updateConferenceCall(const std::shared_ptr<CallLog> &callLog);
	long long insertOrUpdateFriend(const std::shared_ptr<Friend> &f);
	long long insertOrUpdateFriendList(const std::shared_ptr<FriendList> &list);
	long long insertOrUpdateDevice(const std::shared_ptr<Address> &addressWithGruu, const std::string &displayName);

	long long selectSipAddressId(const Address &address, const bool caseSensitive) const;
	long long selectSipAddressId(const std::string &sipAddress, const bool caseSensitive) const;
	long long selectSipAddressId(const std::shared_ptr<Address> &address, const bool caseSensitive) const;
	std::string selectSipAddressFromId(long long sipAddressId) const;
	void deleteChatRoom(const long long &dbId) const;
	void deleteChatRoom(const ConferenceId &conferenceId);
	long long selectChatRoomId(long long peerSipAddressId) const;
	long long selectChatRoomId(long long peerSipAddressId, long long localSipAddressId) const;
	long long selectChatRoomId(const ConferenceId &conferenceId) const;
	ConferenceId selectConferenceId(const long long chatRoomId) const;
	long long selectChatRoomParticipantId(long long chatRoomId, long long participantSipAddressId) const;
	long long selectOneToOneChatRoomId(long long sipAddressIdA, long long sipAddressIdB, bool encrypted) const;
	long long selectConferenceInfoId(long long uriSipAddressId);
	long long selectConferenceInfoParticipantId(long long conferenceInfoId, long long participantSipAddressId) const;
	long long selectConferenceCallId(const std::string &callId);

	void deleteContents(long long chatMessageId);
	void deleteChatRoomParticipant(long long chatRoomId, long long participantSipAddressId);
	void deleteChatRoomParticipantDevice(long long participantId, long long participantDeviceSipAddressId);

	// ---------------------------------------------------------------------------
	// Events API.
	// ---------------------------------------------------------------------------

#ifdef HAVE_DB_STORAGE
	long long getConferenceEventIdFromRow(const soci::row &row) const {
		return dbSession.resolveId(row, 0);
	}

	time_t getConferenceEventCreationTimeFromRow(const soci::row &row) const {
		return dbSession.getTime(row, 2);
	}

	unsigned int getConferenceEventNotifyIdFromRow(const soci::row &row) const {
		L_Q();
		return q->getBackend() == MainDb::Backend::Mysql ? row.get<unsigned int>(10, 0)
		                                                 : static_cast<unsigned int>(row.get<int>(10, 0));
	}

	std::shared_ptr<EventLog> selectGenericConferenceEvent(const std::shared_ptr<AbstractChatRoom> &chatRoom,
	                                                       const soci::row &row) const;

	std::shared_ptr<EventLog> selectConferenceInfoEvent(const ConferenceId &conferenceId, const soci::row &row) const;

	std::shared_ptr<EventLog>
	selectConferenceEvent(const ConferenceId &conferenceId, EventLog::Type type, const soci::row &row) const;

	std::shared_ptr<EventLog> selectConferenceChatMessageEvent(const std::shared_ptr<AbstractChatRoom> &chatRoom,
	                                                           EventLog::Type type,
	                                                           const soci::row &row) const;

	std::shared_ptr<EventLog>
	selectConferenceParticipantEvent(const ConferenceId &conferenceId, EventLog::Type type, const soci::row &row) const;

	std::shared_ptr<EventLog> selectConferenceParticipantDeviceEvent(const ConferenceId &conferenceId,
	                                                                 EventLog::Type type,
	                                                                 const soci::row &row) const;

	std::shared_ptr<EventLog>
	selectConferenceSecurityEvent(const ConferenceId &conferenceId, EventLog::Type type, const soci::row &row) const;

	std::shared_ptr<EventLog> selectConferenceEphemeralMessageEvent(const ConferenceId &conferenceId,
	                                                                EventLog::Type type,
	                                                                const soci::row &row) const;

	std::shared_ptr<EventLog> selectConferenceAvailableMediaEvent(const ConferenceId &conferenceId,
	                                                              EventLog::Type type,
	                                                              const soci::row &row) const;

	std::shared_ptr<EventLog>
	selectConferenceSubjectEvent(const ConferenceId &conferenceId, EventLog::Type type, const soci::row &row) const;
#endif

	long long insertEvent(const std::shared_ptr<EventLog> &eventLog);
	long long insertConferenceEvent(const std::shared_ptr<EventLog> &eventLog, long long *chatRoomId = nullptr);
	long long insertConferenceCallEvent(const std::shared_ptr<EventLog> &eventLog);
	long long insertConferenceChatMessageEvent(const std::shared_ptr<EventLog> &eventLog);
	long long insertConferenceChatMessageReactionEvent(const std::shared_ptr<EventLog> &eventLog);
	void updateConferenceChatMessageEvent(const std::shared_ptr<EventLog> &eventLog);
	long long insertConferenceNotifiedEvent(const std::shared_ptr<EventLog> &eventLog, long long *chatRoomId = nullptr);
	long long insertConferenceParticipantEvent(const std::shared_ptr<EventLog> &eventLog,
	                                           long long *chatRoomId = nullptr,
	                                           bool executeAction = true);
	long long insertConferenceParticipantDeviceEvent(const std::shared_ptr<EventLog> &eventLog);
	long long insertConferenceSubjectEvent(const std::shared_ptr<EventLog> &eventLog);
	long long insertConferenceSecurityEvent(const std::shared_ptr<EventLog> &eventLog);
	long long insertConferenceAvailableMediaEvent(const std::shared_ptr<EventLog> &eventLog);
	long long insertConferenceEphemeralMessageEvent(const std::shared_ptr<EventLog> &eventLog);

	void setChatMessageParticipantState(const std::shared_ptr<EventLog> &eventLog,
	                                    const std::shared_ptr<Address> &participantAddress,
	                                    ChatMessage::State state,
	                                    time_t stateChangeTime);

	void insertNewPreviousConferenceId(const ConferenceId &currentConfId, const ConferenceId &previousConfId);
	void removePreviousConferenceId(const ConferenceId &confId);

	// ---------------------------------------------------------------------------
	// Call log API.
	// ---------------------------------------------------------------------------

#ifdef HAVE_DB_STORAGE
	std::shared_ptr<CallLog> selectCallLog(const soci::row &row) const;
#endif

	// ---------------------------------------------------------------------------
	// Conference Info API.
	// ---------------------------------------------------------------------------

#ifdef HAVE_DB_STORAGE
	std::shared_ptr<ConferenceInfo> selectConferenceInfo(const soci::row &row);
	long long findExpiredConferenceId(const std::shared_ptr<Address> &uri);
#endif

	// ---------------------------------------------------------------------------
	// Friend API.
	// ---------------------------------------------------------------------------

#ifdef HAVE_DB_STORAGE
	std::list<std::shared_ptr<Friend>> getFriends(const std::shared_ptr<FriendList> &list);
	std::shared_ptr<Friend> selectFriend(const soci::row &row) const;
	std::shared_ptr<FriendList> selectFriendList(const soci::row &row) const;
#endif

	// ---------------------------------------------------------------------------
	// Cache API.
	// ---------------------------------------------------------------------------

	void cache(const std::shared_ptr<EventLog> &eventLog, long long storageId) const;
	void cache(const std::shared_ptr<ChatMessage> &chatMessage, long long storageId) const;
	void cache(const ConferenceId &conferenceId, long long storageId) const;
	void cache(const std::shared_ptr<CallLog> &callLog, long long storageId) const;
	void cache(const std::shared_ptr<ConferenceInfo> &conferenceInfo, long long storageId) const;

	std::shared_ptr<EventLog> getEventFromCache(long long storageId) const;
	std::shared_ptr<ChatMessage> getChatMessageFromCache(long long storageId) const;
	ConferenceId getConferenceIdFromCache(long long storageId) const;
	std::shared_ptr<CallLog> getCallLogFromCache(long long storageId) const;
	std::shared_ptr<ConferenceInfo> getConferenceInfoFromCache(long long storageId) const;

	void invalidConferenceEventsFromQuery(const std::string &query, long long chatRoomId) const;

	// ---------------------------------------------------------------------------
	// Versions.
	// ---------------------------------------------------------------------------

	unsigned int getModuleVersion(const std::string &name);
	void updateModuleVersion(const std::string &name, unsigned int version);
	void updateSchema();

	// ---------------------------------------------------------------------------
	// Import.
	// ---------------------------------------------------------------------------

#ifdef HAVE_DB_STORAGE
	bool importLegacyFriends(DbSession &inDbSession);
	bool importLegacyHistory(DbSession &inDbSession);
	bool importLegacyCallLogs(DbSession &inDbSession);
#endif

	// ---------------------------------------------------------------------------

	mutable LruCache<ConferenceId, int> unreadChatMessageCountCache;

	L_DECLARE_PUBLIC(MainDb);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_MAIN_DB_P_H_
