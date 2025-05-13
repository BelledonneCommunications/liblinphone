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

#ifndef _L_MAIN_DB_H_
#define _L_MAIN_DB_H_

#include <functional>
#include <memory>
#include <utility>

#include "linphone/utils/enum-mask.h"

#include "abstract/abstract-db.h"
#include "call/call-log.h"
#include "chat/chat-message/chat-message-reaction.h"
#include "chat/chat-message/chat-message.h"
#include "conference/conference-id.h"
#include "conference/conference-info.h"
#include "conference/conference.h"
#include "core/core-accessor.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class AbstractChatRoom;
class ChatMessage;
class Core;
class EventLog;
class Friend;
class FriendDevice;
class FriendList;
class MainDbKey;
class MainDbPrivate;
class ParticipantDevice;

class LINPHONE_INTERNAL_PUBLIC MainDb : public AbstractDb, public CoreAccessor {
	template <typename Function>
	friend class DbTransaction;

	friend class MainDbEventKey;
	friend class ChatMessage;
	friend class EventLog;

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
		ParticipantState(const std::shared_ptr<Address> &address, ChatMessage::State state, time_t timestamp)
		    : address(address), state(state), timestamp(timestamp) {
		}

		std::shared_ptr<Address> address;
		ChatMessage::State state = ChatMessage::State::Idle;
		time_t timestamp = 0;
	};

	struct ChatRoomContext {
		ChatRoomContext(const std::shared_ptr<AbstractChatRoom> &chatRoom,
		                long long dbId,
		                bool conferenceIdChanged,
		                bool updateFlags)
		    : sChatRoom(chatRoom), sDbId(dbId), sConferenceIdChanged(conferenceIdChanged), sUpdateFlags(updateFlags) {
		}

		std::shared_ptr<AbstractChatRoom> sChatRoom;
		long long sDbId = -1;
		bool sConferenceIdChanged = false;
		bool sUpdateFlags = false;
	};

	MainDb(const std::shared_ptr<Core> &core);

	// ---------------------------------------------------------------------------
	// Generic.
	// ---------------------------------------------------------------------------

	bool addEvent(const std::shared_ptr<EventLog> &eventLog);
	bool updateEvent(const std::shared_ptr<EventLog> &eventLog);
	static bool deleteEvent(const std::shared_ptr<const EventLog> &eventLog);
	int getEventCount(FilterMask mask = NoFilter) const;

	static std::shared_ptr<EventLog> getEventFromKey(const MainDbKey &dbKey);
	static std::shared_ptr<EventLog> getEvent(const std::unique_ptr<MainDb> &mainDb, const long long &storageId);

	// ---------------------------------------------------------------------------
	// Conference notified events.
	// ---------------------------------------------------------------------------

	std::list<std::shared_ptr<EventLog>> getConferenceNotifiedEvents(const ConferenceId &conferenceId,
	                                                                 unsigned int lastNotifyId) const;

	// ---------------------------------------------------------------------------
	// Conference chat message events.
	// ---------------------------------------------------------------------------

	using ParticipantStateRetrievalFunc =
	    std::function<std::list<ParticipantState>(const std::shared_ptr<EventLog> &eventLog)>;

	int getChatMessageCount(const ConferenceId &conferenceId = ConferenceId()) const;
	int getUnreadChatMessageCount(const ConferenceId &conferenceId = ConferenceId()) const;

	void markChatMessagesAsRead(const ConferenceId &conferenceId) const;
	void updateChatRoomEphemeralEnabled(const ConferenceId &conferenceId, bool ephemeralEnabled) const;
	void updateChatRoomEphemeralLifetime(const ConferenceId &conferenceId, long time) const;
	std::list<std::shared_ptr<ChatMessage>> getUnreadChatMessages(const ConferenceId &conferenceId) const;
	void updateEphemeralMessageInfos(const long long &eventId, const time_t &eTime) const;

	std::list<ParticipantState> getChatMessageParticipantsByImdnState(const std::shared_ptr<EventLog> &eventLog,
	                                                                  ChatMessage::State state) const;
	std::list<ParticipantState> getChatMessageParticipantStates(const std::shared_ptr<EventLog> &eventLog) const;
	ChatMessage::State getChatMessageParticipantState(const std::shared_ptr<EventLog> &eventLog,
	                                                  const std::shared_ptr<Address> &participantAddress) const;
	void setChatMessageParticipantState(const std::shared_ptr<EventLog> &eventLog,
	                                    const std::shared_ptr<Address> &participantAddress,
	                                    ChatMessage::State state,
	                                    time_t stateChangeTime);

	std::list<std::shared_ptr<ChatMessage>> getEphemeralMessages() const;

	bool isChatRoomEmpty(const ConferenceId &conferenceId) const;
	std::shared_ptr<ChatMessage> getLastChatMessage(const ConferenceId &conferenceId) const;

	std::list<std::shared_ptr<Content>> getMediaContents(const ConferenceId &conferenceId) const;
	std::list<std::shared_ptr<Content>> getDocumentContents(const ConferenceId &conferenceId) const;

	std::list<std::shared_ptr<ChatMessage>> findChatMessages(const ConferenceId &conferenceId,
	                                                         const std::string &imdnMessageId) const;

	std::list<std::shared_ptr<ChatMessage>> findChatMessages(const ConferenceId &conferenceId,
	                                                         const std::list<std::string> &imdnMessageIds) const;

	std::list<std::shared_ptr<ChatMessage>> findChatMessagesFromMessageId(const std::string &messageId) const;
	std::list<std::shared_ptr<ChatMessage>>
	findChatMessagesFromImdnMessageId(const std::list<std::string> &imdnMessageIds) const;
	std::list<std::shared_ptr<ChatMessage>> findChatMessagesFromCallId(const std::string &callId) const;

	std::list<std::shared_ptr<ChatMessage>> findChatMessagesToBeNotifiedAsDelivered() const;

	std::shared_ptr<EventLog> findEventLog(const ConferenceId &conferenceId, const std::string &imdnMessageId) const;
	std::shared_ptr<EventLog> searchChatMessagesByText(const ConferenceId &conferenceId,
	                                                   const std::string &text,
	                                                   const std::shared_ptr<const EventLog> &from,
	                                                   LinphoneSearchDirection direction);

	// ---------------------------------------------------------------------------
	// Conference events.
	// ---------------------------------------------------------------------------

	std::list<std::shared_ptr<EventLog>>
	getHistory(const ConferenceId &conferenceId, int nLast, FilterMask mask = NoFilter) const;
	std::list<std::shared_ptr<EventLog>>
	getHistoryRange(const ConferenceId &conferenceId, int begin, int end, FilterMask mask = NoFilter) const;
	std::list<std::shared_ptr<EventLog>> getHistoryRangeNear(const ConferenceId &conferenceId,
	                                                         unsigned int before,
	                                                         unsigned int after,
	                                                         const std::shared_ptr<EventLog> &event = nullptr,
	                                                         FilterMask = NoFilter) const;
	std::list<std::shared_ptr<EventLog>> getHistoryRangeBetween(const ConferenceId &conferenceId,
	                                                            const std::shared_ptr<EventLog> &firstEvent = nullptr,
	                                                            const std::shared_ptr<EventLog> &lastEvent = nullptr,
	                                                            FilterMask = NoFilter) const;

	int getHistorySize(const ConferenceId &conferenceId, FilterMask mask = NoFilter) const;

	void cleanHistory(const ConferenceId &conferenceId, FilterMask mask = NoFilter);

	// ---------------------------------------------------------------------------
	// Chat messages.
	// ---------------------------------------------------------------------------

	void loadChatMessageContents(const std::shared_ptr<ChatMessage> &chatMessage);
	std::list<std::shared_ptr<ChatMessageReaction>>
	getChatMessageReactions(const std::shared_ptr<ChatMessage> &chatMessage);
	void removeConferenceChatMessageReactionEvent(const std::string &messageId,
	                                              const std::shared_ptr<const Address> &fromAddress);

	void disableDeliveryNotificationRequired(const std::shared_ptr<const EventLog> &eventLog);
	void disableDisplayNotificationRequired(const std::shared_ptr<const EventLog> &eventLog);

	// ---------------------------------------------------------------------------
	// Chat rooms.
	// ---------------------------------------------------------------------------

	std::list<std::shared_ptr<AbstractChatRoom>> getChatRooms();
	void insertChatRoom(const std::shared_ptr<AbstractChatRoom> &chatRoom, unsigned int notifyId = 0);
	void deleteChatRoom(const ConferenceId &conferenceId);
	void updateNotifyId(const std::shared_ptr<AbstractChatRoom> &chatRoom, const unsigned int lastNotify);
	void updateChatRoomConferenceId(const ConferenceId oldConferenceId, const ConferenceId &newConferenceId);
	void updateChatRoomLastUpdatedTime(const ConferenceId &conferenceId, time_t lastUpdatedTime);
	void updateChatRoomMutedState(const ConferenceId &conferenceId, bool isMuted);
	long long addConferenceParticipantEventToDb(const std::shared_ptr<EventLog> &eventLog,
	                                            long long *chatRoomId = nullptr);

	std::shared_ptr<Address>
	findMissingOneToOneConferenceChatRoomParticipantAddress(const std::shared_ptr<AbstractChatRoom> &chatRoom,
	                                                        const std::shared_ptr<Address> &presentParticipantAddr);
	std::shared_ptr<Address> findOneToOneConferenceChatRoomAddress(const std::shared_ptr<Address> &participantA,
	                                                               const std::shared_ptr<Address> &participantB,
	                                                               bool encrypted) const;
	void insertOneToOneConferenceChatRoom(const std::shared_ptr<AbstractChatRoom> &chatRoom, bool encrypted);

	void updateChatRoomParticipantDevice(const std::shared_ptr<AbstractChatRoom> &chatRoom,
	                                     const std::shared_ptr<ParticipantDevice> &device);

	std::list<std::shared_ptr<Participant>>
	selectChatRoomParticipants(const std::shared_ptr<AbstractChatRoom> &chatRoom) const;
	std::list<std::shared_ptr<Participant>> selectChatRoomParticipants(const long long chatRoomId) const;
	void insertChatRoomParticipant(const std::shared_ptr<AbstractChatRoom> &chatRoom,
	                               const std::shared_ptr<Participant> &participant);
	void deleteChatRoomParticipant(const std::shared_ptr<AbstractChatRoom> &chatRoom,
	                               const std::shared_ptr<Address> &participant);

	void insertChatRoomParticipantDevice(const std::shared_ptr<AbstractChatRoom> &chatRoom,
	                                     const std::shared_ptr<ParticipantDevice> &device);
	void deleteChatRoomParticipantDevice(const std::shared_ptr<AbstractChatRoom> &chatRoom,
	                                     const std::shared_ptr<ParticipantDevice> &device);

	void insertNewPreviousConferenceId(const ConferenceId &currentConfId, const ConferenceId &previousConfId);
	void removePreviousConferenceId(const ConferenceId &confId);

	// ---------------------------------------------------------------------------
	// Conference Info.
	// ---------------------------------------------------------------------------

	std::list<std::shared_ptr<ConferenceInfo>>
	getConferenceInfos(time_t afterThisTime = -1, const std::list<LinphoneStreamType> capabilities = {});
	std::list<std::shared_ptr<ConferenceInfo>>
	getConferenceInfosWithParticipant(const std::shared_ptr<Address> &address,
	                                  const std::list<LinphoneStreamType> capabilities = {});
	std::shared_ptr<ConferenceInfo> getConferenceInfo(long long conferenceInfoId);
	std::shared_ptr<ConferenceInfo> getConferenceInfoFromURI(const std::shared_ptr<Address> &uri);
	std::shared_ptr<ConferenceInfo> getConferenceInfoFromCcmpUri(const std::string &uri);
	long long insertConferenceInfo(const std::shared_ptr<ConferenceInfo> &conferenceInfo);
	void deleteConferenceInfo(long long dbConferenceId);
	void deleteConferenceInfo(const std::shared_ptr<Address> &address);
	void deleteConferenceInfo(const std::shared_ptr<ConferenceInfo> &conferenceInfo);
	void migrateConferenceInfos();
	void cleanupConferenceInfo(time_t expiredBeforeThisTime);

	long long findExpiredConferenceId(const std::shared_ptr<Address> &uri);
	void insertExpiredConference(const std::shared_ptr<Address> &uri);

	// ---------------------------------------------------------------------------
	// Call log.
	// ---------------------------------------------------------------------------

	long long insertCallLog(const std::shared_ptr<CallLog> &callLog);
	void updateCallLog(const std::shared_ptr<CallLog> &callLog);
	void deleteCallLog(const std::shared_ptr<CallLog> &callLog);

	std::shared_ptr<CallLog> getCallLog(const std::string &callId, int limit);

	std::list<std::shared_ptr<CallLog>> getCallHistory(int limit = -1);
	std::list<std::shared_ptr<CallLog>> getCallHistoryForLocalAddress(const std::shared_ptr<Address> &localAddress,
	                                                                  int limit = -1);
	std::list<std::shared_ptr<CallLog>> getCallHistory(const std::shared_ptr<const Address> &peer,
	                                                   const std::shared_ptr<const Address> &local,
	                                                   int limit = -1);
	std::shared_ptr<CallLog> getLastOutgoingCall();
	void deleteCallHistory();
	void deleteCallHistoryForLocalAddress(const std::shared_ptr<Address> &localAddress);

	int getCallHistorySize();

	// ---------------------------------------------------------------------------
	// Friend & FriendList.
	// ---------------------------------------------------------------------------
	long long insertFriend(const std::shared_ptr<Friend> &f);
	long long insertFriendList(const std::shared_ptr<FriendList> &list);
	void deleteFriend(const std::shared_ptr<Friend> &f);
	void deleteFriendList(const std::shared_ptr<FriendList> &list);
	void deleteFriendList(const std::string &name);
	void deleteOrphanFriends();
	std::list<std::shared_ptr<Friend>> getFriends(const std::shared_ptr<FriendList> &list);
	std::list<std::shared_ptr<FriendList>> getFriendLists();

	void insertDevices(const std::list<std::pair<std::shared_ptr<Address>, std::string>> &devices);
	long long insertDevice(const std::shared_ptr<Address> &addressWithGruu, const std::string &displayName);
	void removeDevice(const std::shared_ptr<Address> &addressWithGruu);
	std::list<std::shared_ptr<FriendDevice>> getDevices(const std::shared_ptr<Address> &address);

	// ---------------------------------------------------------------------------
	// Other.
	// ---------------------------------------------------------------------------

	// Import legacy calls/messages from old db. Returns true if something was done.
	bool import(Backend backend, const std::string &parameters) override;

	static FilterMask getFilterMaskFromHistoryFilterMask(AbstractChatRoom::HistoryFilterMask historyFilterMask);

protected:
	void init() override;

private:
	L_DECLARE_PRIVATE(MainDb);
	L_DISABLE_COPY(MainDb);
	using ChatRoomWeakCompareMap =
	    std::unordered_map<ConferenceId, ChatRoomContext, ConferenceId::WeakHash, ConferenceId::WeakEqual>;
	void initCleanup();
	bool addChatroomToList(ChatRoomWeakCompareMap &chatRoomsMap,
	                       const std::shared_ptr<AbstractChatRoom> &chatRoom,
	                       long long id,
	                       int unreadMessageCount) const;
	std::shared_ptr<AbstractChatRoom> mergeChatRooms(const std::shared_ptr<AbstractChatRoom> &chatRoom1,
	                                                 const std::shared_ptr<AbstractChatRoom> &chatRoom2,
	                                                 long long id1,
	                                                 long long id2,
	                                                 int unreadMessageCount) const;

	std::string getConferenceInfoTypeQuery(const std::list<LinphoneStreamType> &capabilities) const;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_MAIN_DB_H_
