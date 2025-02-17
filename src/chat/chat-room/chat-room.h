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

#ifndef _L_CHAT_ROOM_H_
#define _L_CHAT_ROOM_H_

#include <list>

#include "abstract-chat-room.h"
#include "chat/notification/imdn.h"
#include "chat/notification/is-composing.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Imdn;
class IsComposing;
class IsComposingMessage;
class ImdnMessage;

class LINPHONE_PUBLIC ChatRoom : public AbstractChatRoom, public IsComposingListener {
public:
	friend class ChatMessagePrivate;
	friend class Imdn;
	friend class ImdnMessagePrivate;
	friend class CorePrivate;

	virtual ~ChatRoom();

	const std::shared_ptr<Address> &getPeerAddress() const override;
	const std::shared_ptr<Address> &getLocalAddress() const override;
	std::optional<std::reference_wrapper<const std::string>> getIdentifier() const override;

	time_t getCreationTime() const override;
	time_t getLastUpdateTime() const override;

	SecurityLevel getSecurityLevel() const override;

	std::list<std::shared_ptr<Content>> getMediaContents() const override;
	std::list<std::shared_ptr<Content>> getDocumentContents() const override;

	std::list<std::shared_ptr<EventLog>> getMessageHistory(int nLast) const override;
	std::list<std::shared_ptr<EventLog>> getMessageHistoryRange(int begin, int end) const override;
	std::list<std::shared_ptr<ChatMessage>> getUnreadChatMessages() const override;
	int getMessageHistorySize() const override;
	std::list<std::shared_ptr<EventLog>> getHistory(int nLast) const override;
	std::list<std::shared_ptr<EventLog>> getHistory(int nLast, HistoryFilterMask filters) const override;
	std::list<std::shared_ptr<EventLog>> getHistoryRange(int begin, int end) const override;
	std::list<std::shared_ptr<EventLog>> getHistoryRange(int begin, int end, HistoryFilterMask filters) const override;
	std::list<std::shared_ptr<EventLog>> getHistoryRangeNear(unsigned int before,
	                                                         unsigned int after,
	                                                         const std::shared_ptr<EventLog> &event,
	                                                         HistoryFilterMask filters) const override;
	std::list<std::shared_ptr<EventLog>> getHistoryRangeBetween(const std::shared_ptr<EventLog> &firstEvent,
	                                                            const std::shared_ptr<EventLog> &lastEvent,
	                                                            HistoryFilterMask filters) const override;
	int getHistorySize() const override;
	int getHistorySize(HistoryFilterMask filters) const override;

	void deleteFromDb() override;
	void deleteHistory() override;
	void deleteMessageFromHistory(const std::shared_ptr<ChatMessage> &message) override;

	std::shared_ptr<ChatMessage> getLastChatMessageInHistory() const override;
	bool isEmpty() const override;
	int getChatMessageCount() const override;
	int getUnreadChatMessageCount() const override;

	void compose() override;
	bool isRemoteComposing() const override;
	std::list<std::shared_ptr<Address>> getComposingAddresses() const override;

	std::shared_ptr<ChatMessage> createChatMessage() override;
	std::shared_ptr<ChatMessage> createChatMessage(const std::string &text) override;
	std::shared_ptr<ChatMessage> createChatMessageFromUtf8(const std::string &text) override;

	std::shared_ptr<ChatMessage> createFileTransferMessage(const std::shared_ptr<FileContent> &content) override;
	std::shared_ptr<ChatMessage> createForwardMessage(const std::shared_ptr<ChatMessage> &msg) override;
	std::shared_ptr<ChatMessage> createReplyMessage(const std::shared_ptr<ChatMessage> &msg) override;

	std::shared_ptr<ChatMessage> findChatMessage(const std::string &messageId) const override;
	std::shared_ptr<ChatMessage> findChatMessage(const std::string &messageId,
	                                             ChatMessage::Direction direction) const override;
	std::shared_ptr<ChatMessage> findChatMessageFromMessageId(const std::string &messageId) const override;
	std::shared_ptr<ChatMessage> findChatMessageFromCallId(const std::string &callId) const override;
	std::list<std::shared_ptr<ChatMessage>> findChatMessages(const std::string &messageId) const;
	std::list<std::shared_ptr<ChatMessage>> findChatMessages(const std::list<std::string> &messageIds) const override;

	std::shared_ptr<EventLog> findChatMessageEventLog(const std::string &messageId) const override;
	std::shared_ptr<EventLog> searchChatMessageByText(const std::string &text,
	                                                  const std::shared_ptr<const EventLog> &from,
	                                                  LinphoneSearchDirection direction) const override;

	void markAsRead() override;
	void enableEphemeral(bool ephem, bool updateDb) override;
	bool ephemeralEnabled() const override;
	void setEphemeralLifetime(long lifetime, bool updateDb) override;
	long getEphemeralLifetime() const override;
	void setEphemeralMode(AbstractChatRoom::EphemeralMode mode, bool updateDb) override;
	AbstractChatRoom::EphemeralMode getEphemeralMode() const override;
	bool ephemeralSupportedByAllParticipants() const override;

	ConferenceInterface::State getState() const override;
	void setState(ConferenceInterface::State newState) override;

	void invalidateAccount() override;
	std::shared_ptr<Account> getAccount() override;

	void setUtf8Subject(const std::string &subject) override;
	const std::string &getSubjectUtf8() const override;

	std::shared_ptr<ConferenceParams> getCurrentParams() const override;

	bool isSubscriptionUnderWay() const override;

	bool isMe(const std::shared_ptr<Address> &address) const override;
	std::shared_ptr<Participant> getMe() const override;

	std::shared_ptr<Address> getConferenceAddress() const override;
	std::shared_ptr<Participant> findParticipant(const std::shared_ptr<Address> &address) const override;
	std::list<std::shared_ptr<Participant>> getParticipants() const override;
	std::list<std::shared_ptr<Address>> getParticipantAddresses() const override;

	virtual std::shared_ptr<Conference> getConference() const override;

	virtual const ConferenceId &getConferenceId() const override;

	bool canHandleParticipants() const override {
		return (getConference() != nullptr);
	}

	uint32_t getChar() override;
	std::shared_ptr<Call> getCall() const override;

	virtual bool getIsMuted() const override;
	virtual void setIsMuted(const bool muted, const bool updateDb = true) override;

	inline void setCreationTime(time_t creationTime) override {
		this->creationTime = creationTime;
	}

	inline void setLastUpdateTime(time_t lastUpdateTime) override {
		this->lastUpdateTime = lastUpdateTime;
	}

	void sendChatMessage(const std::shared_ptr<ChatMessage> &chatMessage) override;
	void onChatMessageSent(const std::shared_ptr<ChatMessage> &chatMessage) override;
	void sendIsComposingNotification();

	void addEvent(const std::shared_ptr<EventLog> &eventLog) override;

	void addTransientEvent(const std::shared_ptr<EventLog> &eventLog) override;
	void removeTransientEvent(const std::shared_ptr<EventLog> &eventLog) override;

	void addTransientChatMessage(const std::shared_ptr<ChatMessage> &message) override;
	void removeTransientChatMessage(const std::shared_ptr<ChatMessage> &message) override;
	std::list<std::shared_ptr<ChatMessage>> getTransientChatMessages() override;

	void setIsEmpty(const bool empty) override;

	virtual void addPendingMessage(const std::shared_ptr<ChatMessage> &chatMessage) override;

	std::shared_ptr<ChatMessage> createChatMessage(ChatMessage::Direction direction);
	std::list<std::shared_ptr<ImdnMessage>>
	createImdnMessages(const std::list<std::shared_ptr<ChatMessage>> &deliveredMessages,
	                   const std::list<std::shared_ptr<ChatMessage>> &displayedMessages,
	                   bool aggregate);
	std::list<std::shared_ptr<ImdnMessage>>
	createImdnMessages(const std::list<Imdn::MessageReason> &nonDeliveredMessages, bool aggregate);
	std::shared_ptr<ImdnMessage> createImdnMessage(const std::shared_ptr<ImdnMessage> &message);
	std::shared_ptr<IsComposingMessage> createIsComposingMessage();

	void sendDeliveryErrorNotification(const std::shared_ptr<ChatMessage> &chatMessage, LinphoneReason reason);
	void sendDeliveryNotification(const std::shared_ptr<ChatMessage> &chatMessage);
	void sendDeliveryNotifications(const std::shared_ptr<ChatMessage> &chatMessage) override;
	void sendDisplayNotification(const std::shared_ptr<ChatMessage> &chatMessage);

	void notifyAggregatedChatMessages() override;
	void notifyMessageReceived(const std::shared_ptr<ChatMessage> &chatMessage);
	void notifyChatMessageReceived(const std::shared_ptr<ChatMessage> &chatMessage) override;
	void notifyIsComposingReceived(const std::shared_ptr<Address> &remoteAddress, bool isComposing);
	void notifyUndecryptableChatMessageReceived(const std::shared_ptr<ChatMessage> &chatMessage) override;

	LinphoneReason onSipMessageReceived(SalOp *op, const SalMessage *message) override;
	void onChatMessageReceived(const std::shared_ptr<ChatMessage> &chatMessage) override;
	void handleMessageRejected(const std::shared_ptr<ChatMessage> &chatMessage) override;
	void onImdnReceived(const std::shared_ptr<ChatMessage> &chatMessage);
	void onIsComposingReceived(const std::shared_ptr<Address> &remoteAddress, const std::string &text);
	void onIsComposingRefreshNeeded() override;
	void onIsComposingStateChanged(bool isComposing) override;
	void onIsRemoteComposingStateChanged(const std::shared_ptr<Address> &remoteAddress, bool isComposing) override;

	void realtimeTextReceived(uint32_t character, const std::shared_ptr<Call> &call) override;
#ifdef HAVE_BAUDOT
	void baudotCharacterReceived(char character, const std::shared_ptr<Call> &call) override;
#endif /* HAVE_BAUDOT */
	void setCallId(const std::string &value) override;

	std::shared_ptr<ConferenceInfo> getConferenceInfo() const override;

	Imdn *getImdnHandler() const override {
		return mImdnHandler.get();
	}

	LinphoneChatRoom *getCChatRoom();
	void notifyStateChanged();

	void addCapability(AbstractChatRoom::CapabilitiesMask capability) override;
	void onStateChanged(ConferenceInterface::State state) override;

	std::list<std::shared_ptr<Address>> remoteIsComposing;
	std::list<std::shared_ptr<EventLog>> transientEvents;
	std::list<std::shared_ptr<ChatMessage>> transientMessages;
	std::list<std::shared_ptr<ChatMessage>> aggregatedMessages;

protected:
	Address getImdnChatRoomPeerAddress(const std::shared_ptr<ChatMessage> &message) const;
	std::shared_ptr<AbstractChatRoom> getImdnChatRoom(const std::shared_ptr<Address> peerAddress);
	std::shared_ptr<ChatMessage> getMessageFromSal(SalOp *op, const SalMessage *message);
	explicit ChatRoom(const std::shared_ptr<Core> &core, const std::shared_ptr<Conference> &conf = nullptr);

	std::shared_ptr<Conference> conference;

private:
	void
	realtimeTextOrBaudotCharacterReceived(uint32_t character, const std::shared_ptr<Call> &call, bool isRealTimeText);

	time_t creationTime = std::time(nullptr);
	time_t lastUpdateTime = std::time(nullptr);

	std::unique_ptr<Imdn> mImdnHandler;
	std::unique_ptr<IsComposing> mIsComposingHandler;

	bool mIsComposing = false;
	bool mEmpty = true;
	bool mIsMuted = false;

	std::string mCallId;
	size_t mReadCharacterIndex = 0;
	std::vector<uint32_t> mReceivedRttCharacters;
	std::vector<uint32_t> mLastMessageCharacters;

	L_DISABLE_COPY(ChatRoom);
};

class ChatRoomLogContextualizer : CoreLogContextualizer {
public:
	ChatRoomLogContextualizer(const LinphoneChatRoom *cr);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CHAT_ROOM_H_
