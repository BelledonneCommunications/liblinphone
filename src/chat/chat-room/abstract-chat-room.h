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

#ifndef _L_ABSTRACT_CHAT_ROOM_H_
#define _L_ABSTRACT_CHAT_ROOM_H_

#include <bctoolbox/defs.h>

#include "belle-sip/object++.hh"

#include "chat/chat-message/chat-message.h"
#include "chat/chat-room/chat-room-cbs.h"
#include "conference/conference-interface.h"
#include "conference/session/call-session-listener.h"
#include "core/core-accessor.h"
#include "linphone/utils/enum-mask.h"

// =============================================================================

L_DECL_C_STRUCT_PREFIX_LESS(SalMessage);

LINPHONE_BEGIN_NAMESPACE

class Imdn;
class ConferenceId;
class ConferenceParams;
class EventLog;

class LINPHONE_PUBLIC AbstractChatRoom : public bellesip::HybridObject<LinphoneChatRoom, AbstractChatRoom>,
                                         public CallbacksHolder<ChatRoomCbs>,
                                         public ConferenceListenerInterface,
                                         public CoreAccessor,
                                         public UserDataAccessor {
	friend class Call;
	friend class ChatMessage;
	friend class ChatMessagePrivate;
	friend class Core;
	friend class CorePrivate;
	friend class MainDb;

public:
	/*enum class is used to create namespaces for Enums
	  doing this prevents the compiler to confuse members from different Enums with same name.
	    i.e. "None" for the "State" Enum and "None" from the Capabilities Enum */
	enum class Capabilities {
		None = LinphoneChatRoomCapabilitiesNone,
		Basic = LinphoneChatRoomCapabilitiesBasic,
		RealTimeText = LinphoneChatRoomCapabilitiesRealTimeText,
		Conference = LinphoneChatRoomCapabilitiesConference,
		Proxy = LinphoneChatRoomCapabilitiesProxy,
		Migratable = LinphoneChatRoomCapabilitiesMigratable,
		OneToOne = LinphoneChatRoomCapabilitiesOneToOne,
		Encrypted = LinphoneChatRoomCapabilitiesEncrypted,
		Ephemeral = 1 << 7 // Entered like this to check enum order in static_assert below
	};

	enum class SecurityLevel {
		Unsafe = LinphoneChatRoomSecurityLevelUnsafe,
		ClearText = LinphoneChatRoomSecurityLevelClearText,
		Encrypted = LinphoneChatRoomSecurityLevelEncrypted,
		Safe
	};

	enum class EphemeralMode {
		DeviceManaged = LinphoneChatRoomEphemeralModeDeviceManaged,
		AdminManaged = LinphoneChatRoomEphemeralModeAdminManaged
	};

	enum class HistoryFilter : unsigned {
		None = LinphoneChatRoomHistoryFilterNone,
		Call = LinphoneChatRoomHistoryFilterCall,
		ChatMessage = LinphoneChatRoomHistoryFilterChatMessage,
		ChatMessageSecurity = LinphoneChatRoomHistoryFilterChatMessageSecurity,
		Info = LinphoneChatRoomHistoryFilterInfo,
		InfoNoDevice = LinphoneChatRoomHistoryFilterInfoNoDevice
	};

	typedef enum EphemeralMode ChatRoomEphemeralMode;

	// casting to int to get rid of the enum compare warning.
	// Here we are comparing two enums serving the same purpose
	//  char as in byte
	static_assert((char)AbstractChatRoom::Capabilities::Encrypted == (char)LinphoneChatRoomCapabilitiesEncrypted,
	              "LinphoneChatRoomCapabilities and AbstractChatRoom::Capabilities are not synchronized, fix this !");

	// casting to int to get rid of the enum compare warning.
	// Here we are comparing two enums serving the same purpose
	static_assert(
	    (int)AbstractChatRoom::SecurityLevel::Safe == (int)LinphoneChatRoomSecurityLevelSafe,
	    "LinphoneEncryptionEngineSecurityLevel and AbstractChatRoom::SecurityLevel are not synchronized, fix this !");

	typedef EnumMask<Capabilities> CapabilitiesMask;
	typedef EnumMask<HistoryFilter> HistoryFilterMask;

	virtual void allowCpim(bool value) = 0;
	virtual void allowMultipart(bool value) = 0;
	virtual bool canHandleCpim() const = 0;
	virtual bool canHandleMultipart() const = 0;

	virtual const std::shared_ptr<Address> &getPeerAddress() const = 0;
	virtual const std::shared_ptr<Address> &getLocalAddress() const = 0;
	virtual std::optional<std::reference_wrapper<const std::string>> getIdentifier() const = 0;

	virtual time_t getCreationTime() const = 0;
	virtual time_t getLastUpdateTime() const = 0;

	virtual CapabilitiesMask getCapabilities() const = 0;
	virtual SecurityLevel getSecurityLevel() const = 0;
	virtual bool hasBeenLeft() const = 0;
	virtual bool isReadOnly() const = 0;

	virtual std::list<std::shared_ptr<Content>> getMediaContents() const = 0;
	virtual std::list<std::shared_ptr<Content>> getDocumentContents() const = 0;

	virtual std::list<std::shared_ptr<EventLog>> getMessageHistory(int nLast) const = 0;
	virtual std::list<std::shared_ptr<EventLog>> getMessageHistoryRange(int begin, int end) const = 0;
	virtual std::list<std::shared_ptr<ChatMessage>> getUnreadChatMessages() const = 0;
	virtual int getMessageHistorySize() const = 0;
	virtual std::list<std::shared_ptr<EventLog>> getHistory(int nLast) const = 0;
	virtual std::list<std::shared_ptr<EventLog>> getHistory(int nLast, HistoryFilterMask filters) const = 0;
	virtual std::list<std::shared_ptr<EventLog>> getHistoryRange(int begin, int end) const = 0;
	virtual std::list<std::shared_ptr<EventLog>>
	getHistoryRange(int begin, int end, HistoryFilterMask filters) const = 0;
	virtual std::list<std::shared_ptr<EventLog>> getHistoryRangeNear(unsigned int before,
	                                                                 unsigned int after,
	                                                                 const std::shared_ptr<EventLog> &event,
	                                                                 HistoryFilterMask filters) const = 0;
	virtual std::list<std::shared_ptr<EventLog>> getHistoryRangeBetween(const std::shared_ptr<EventLog> &firstEvent,
	                                                                    const std::shared_ptr<EventLog> &lastEvent,
	                                                                    HistoryFilterMask filters) const = 0;
	virtual int getHistorySize() const = 0;
	virtual int getHistorySize(HistoryFilterMask filters) const = 0;

	virtual void deleteFromDb() = 0;
	virtual void deleteHistory() = 0;
	virtual void deleteMessageFromHistory(const std::shared_ptr<ChatMessage> &message) = 0;

	virtual std::shared_ptr<ChatMessage> getLastChatMessageInHistory() const = 0;
	virtual bool isEmpty() const = 0;
	virtual int getChatMessageCount() const = 0;
	virtual int getUnreadChatMessageCount() const = 0;

	virtual void compose() = 0;
	virtual bool isRemoteComposing() const = 0;
	virtual std::list<std::shared_ptr<Address>> getComposingAddresses() const = 0;
	const bctbx_list_t *getComposingCAddresses() const;

	virtual std::shared_ptr<ChatMessage> createChatMessage() = 0;
	virtual std::shared_ptr<ChatMessage> createChatMessage(const std::string &text) = 0;
	virtual std::shared_ptr<ChatMessage> createChatMessageFromUtf8(const std::string &text) = 0;

	virtual std::shared_ptr<ChatMessage> createFileTransferMessage(const std::shared_ptr<FileContent> &content) = 0;
	virtual std::shared_ptr<ChatMessage> createForwardMessage(const std::shared_ptr<ChatMessage> &msg) = 0;
	virtual std::shared_ptr<ChatMessage> createReplyMessage(const std::shared_ptr<ChatMessage> &msg) = 0;

	virtual std::shared_ptr<ChatMessage> findChatMessage(const std::string &messageId) const = 0;
	virtual std::shared_ptr<ChatMessage> findChatMessage(const std::string &messageId,
	                                                     ChatMessage::Direction direction) const = 0;
	virtual std::shared_ptr<ChatMessage> findChatMessageFromMessageId(const std::string &messageId) const = 0;
	virtual std::shared_ptr<ChatMessage> findChatMessageFromCallId(const std::string &callId) const = 0;
	virtual std::list<std::shared_ptr<ChatMessage>>
	findChatMessages(const std::list<std::string> &messageIds) const = 0;

	virtual std::shared_ptr<EventLog> findChatMessageEventLog(const std::string &messageId) const = 0;
	virtual std::shared_ptr<EventLog> searchChatMessageByText(const std::string &text,
	                                                          const std::shared_ptr<const EventLog> &from,
	                                                          LinphoneSearchDirection direction) const = 0;

	virtual void sendPendingMessages() {};

	virtual void markAsRead() = 0;
	virtual void enableEphemeral(bool ephem, bool updateDb) = 0;
	virtual bool ephemeralEnabled() const = 0;
	virtual void setEphemeralLifetime(long lifetime, bool updateDb) = 0;
	virtual long getEphemeralLifetime() const = 0;
	virtual void setEphemeralMode(AbstractChatRoom::EphemeralMode mode, bool updateDb) = 0;
	virtual AbstractChatRoom::EphemeralMode getEphemeralMode() const = 0;
	virtual bool ephemeralSupportedByAllParticipants() const = 0;

	virtual std::shared_ptr<ConferenceParams> getCurrentParams() const = 0;

	virtual bool isSubscriptionUnderWay() const = 0;

	virtual bool isMe(const std::shared_ptr<Address> &address) const = 0;
	virtual std::shared_ptr<Participant> getMe() const = 0;

	virtual std::shared_ptr<Address> getConferenceAddress() const = 0;
	virtual std::shared_ptr<Participant> findParticipant(const std::shared_ptr<Address> &address) const = 0;
	virtual std::list<std::shared_ptr<Participant>> getParticipants() const = 0;
	virtual std::list<std::shared_ptr<Address>> getParticipantAddresses() const = 0;

	virtual bool canHandleParticipants() const = 0;
	virtual std::shared_ptr<Conference> getConference() const = 0;
	virtual const ConferenceId &getConferenceId() const = 0;

	virtual ConferenceInterface::State getState() const = 0;
	virtual void setState(ConferenceInterface::State newState) = 0;

	virtual void invalidateAccount() = 0;
	virtual std::shared_ptr<Account> getAccount() = 0;
	virtual const std::string &getSubjectUtf8() const = 0;
	virtual void setUtf8Subject(const std::string &subject) = 0;
	void setSubject(const std::string &subject);
	const std::string &getSubject() const;

	virtual uint32_t getChar() = 0;
	virtual std::shared_ptr<Call> getCall() const = 0;

	virtual bool getIsMuted() const = 0;
	virtual void setIsMuted(const bool muted, const bool updateDb = true) = 0;

	virtual void setCreationTime(time_t creationTime) = 0;
	virtual void setLastUpdateTime(time_t lastUpdateTime) = 0;

	virtual void sendChatMessage(const std::shared_ptr<ChatMessage> &chatMessage) = 0;
	virtual void onChatMessageSent(const std::shared_ptr<ChatMessage> &chatMessage) = 0;

	virtual void addEvent(const std::shared_ptr<EventLog> &eventLog) = 0;

	virtual void addTransientEvent(const std::shared_ptr<EventLog> &eventLog) = 0;
	virtual void removeTransientEvent(const std::shared_ptr<EventLog> &eventLog) = 0;

	virtual void sendDeliveryNotifications(const std::shared_ptr<ChatMessage> &chatMessage) = 0;

	virtual void notifyChatMessageReceived(const std::shared_ptr<ChatMessage> &chatMessage) = 0;
	virtual void notifyUndecryptableChatMessageReceived(const std::shared_ptr<ChatMessage> &chatMessage) = 0;

	virtual LinphoneReason onSipMessageReceived(SalOp *op, const SalMessage *message) = 0;
	virtual void onChatMessageReceived(const std::shared_ptr<ChatMessage> &chatMessage) = 0;
	virtual void handleMessageRejected(const std::shared_ptr<ChatMessage> &chatMessage) = 0;

	virtual void addTransientChatMessage(const std::shared_ptr<ChatMessage> &message) = 0;
	virtual void removeTransientChatMessage(const std::shared_ptr<ChatMessage> &message) = 0;
	virtual std::list<std::shared_ptr<ChatMessage>> getTransientChatMessages() = 0;

	virtual void setIsEmpty(const bool empty) = 0;

	virtual void realtimeTextReceived(uint32_t character, const std::shared_ptr<Call> &call) = 0;
#ifdef HAVE_BAUDOT
	virtual void baudotCharacterReceived(char character, const std::shared_ptr<Call> &call) = 0;
#endif /* HAVE_BAUDOT */
	virtual void setCallId(const std::string &value) = 0;

	virtual void notifyAggregatedChatMessages() = 0;

	virtual void addPendingMessage(const std::shared_ptr<ChatMessage> &chatMessage) = 0;

	virtual void addCapability(AbstractChatRoom::CapabilitiesMask capability) = 0;

	virtual Imdn *getImdnHandler() const = 0;

	virtual std::shared_ptr<ConferenceInfo> getConferenceInfo() const = 0;

protected:
	explicit AbstractChatRoom(const std::shared_ptr<Core> &core);
	virtual ~AbstractChatRoom();

private:
	mutable bctbx_list_t *composingCAddresses = nullptr;
	mutable std::string mSubject;

	L_DISABLE_COPY(AbstractChatRoom);
};

std::ostream &operator<<(std::ostream &lhs, AbstractChatRoom::Capabilities e);

std::ostream &operator<<(std::ostream &lhs, AbstractChatRoom::SecurityLevel e);

std::ostream &operator<<(std::ostream &lhs, AbstractChatRoom::EphemeralMode e);

inline std::ostream &operator<<(std::ostream &str, const AbstractChatRoom &chatRoom) {
	const auto &conferenceAddress = chatRoom.getConferenceAddress();
	str << "ChatRoom [" << &chatRoom << "] ("
	    << (conferenceAddress ? conferenceAddress->toString() : std::string("sip:")) << ")";
	return str;
}

LINPHONE_END_NAMESPACE

#endif // ifndef _L_ABSTRACT_CHAT_ROOM_H_
