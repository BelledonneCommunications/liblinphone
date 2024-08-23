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

#ifndef _L_CHAT_MESSAGE_H_
#define _L_CHAT_MESSAGE_H_

#include <list>

#include <belle-sip/mainloop.h>

#include "core/core-accessor.h"
#include "core/core-listener.h"
#include "linphone/api/c-types.h"
#include "linphone/enums/chat-message-enums.h"
#include "object/object.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class AbstractChatRoom;
class ClientChatRoom;
class Address;
class Conference;
class Content;
class FileContent;
class FileTransferContent;
class ChatMessagePrivate;
class Participant;
class ParticipantImdnState;
class ChatMessageListener;
class ConferenceScheduler;
class ChatMessageReaction;

class LINPHONE_PUBLIC ChatMessage : public Object, public CoreAccessor, public CoreListener {
	friend class ServerConference;
	friend class ClientConference;
	friend class ChatRoom;
	friend class ClientChatRoom;
	friend class ServerChatRoom;
	friend class CpimChatMessageModifier;
	friend class FileTransferChatMessageModifier;
	friend class Imdn;
	friend class ImdnMessagePrivate;
	friend class MainDb;
	friend class MainDbPrivate;
	friend class LimeX3dhEncryptionEngine;
	friend class CorePrivate;
	friend class ConferenceScheduler;
	friend class ChatMessageReaction;

public:
	L_OVERRIDE_SHARED_FROM_THIS(ChatMessage);

	// Retry to send message expire value of the timer in seconds
	static constexpr unsigned int resendTimerExpiresS = 10;

	enum class State {
		Idle = LinphoneChatMessageStateIdle,
		InProgress = LinphoneChatMessageStateInProgress,
		Delivered = LinphoneChatMessageStateDelivered,
		NotDelivered = LinphoneChatMessageStateNotDelivered,
		FileTransferError = LinphoneChatMessageStateFileTransferError,
		FileTransferDone = LinphoneChatMessageStateFileTransferDone,
		DeliveredToUser = LinphoneChatMessageStateDeliveredToUser,
		Displayed = LinphoneChatMessageStateDisplayed,
		PendingDelivery = LinphoneChatMessageStatePendingDelivery,
		FileTransferInProgress = LinphoneChatMessageStateFileTransferInProgress,
		FileTransferCancelling = LinphoneChatMessageStateFileTransferCancelling
	};

	// casting to int to get rid of the enum compare warning.
	// Here we are comparing two enums serving the same purpose
	static_assert((int)ChatMessage::State::FileTransferInProgress ==
	                  (int)LinphoneChatMessageStateFileTransferInProgress,
	              "LinphoneChatMessageState and ChatMessage::State are not synchronized, fix this !");

	enum class Direction {
		Incoming = LinphoneChatMessageDirectionIncoming,
		Outgoing = LinphoneChatMessageDirectionOutgoing
	};

	// casting to int to get rid of the enum compare warning.
	// Here we are comparing two enums serving the same purpose
	static_assert((int)ChatMessage::Direction::Outgoing == (int)LinphoneChatMessageDirectionOutgoing,
	              "LinphoneChatMessageDirection and ChatMessage::Direction are not synchronized, fix this !");

	virtual ~ChatMessage();

	// ----- TODO: Remove me.
	void cancelFileTransfer();
	int putCharacter(uint32_t character);
	void setIsSecured(bool isSecured);
	// ----- TODO: Remove me.

	long long getStorageId() const;
	void deleteChatMessageFromCache();

	const std::shared_ptr<Address> &getMeAddress() const;
	std::shared_ptr<AbstractChatRoom> getChatRoom() const;

	void send();

	time_t getTime() const;

	bool isSecured() const;
	bool isValid() const;
	State getState() const;
	Direction getDirection() const;

	const std::string &getImdnMessageId() const;

	const Address &getAuthenticatedFromAddress() const;
	const std::shared_ptr<Address> &getFromAddress() const;
	const std::shared_ptr<Address> &getToAddress() const;
	const std::shared_ptr<Address> &getLocalAddress() const;
	const std::shared_ptr<Address> &getRecipientAddress() const;
	const std::string &getForwardInfo() const;

	bool isReply() const;
	const std::string &getReplyToMessageId() const;
	const std::shared_ptr<Address> &getReplyToSenderAddress() const;
	std::shared_ptr<ChatMessage> getReplyToMessage() const;

	bool isEphemeral() const;
	long getEphemeralLifetime() const;
	time_t getEphemeralExpireTime() const;

	bool isReaction() const;
	const std::string &getReactionToMessageId() const;
	std::shared_ptr<ChatMessage> getReactionToMessage() const;
	const std::list<std::shared_ptr<ChatMessageReaction>> getReactions() const;
	const std::shared_ptr<ChatMessageReaction> getOwnReaction() const;

	// TODO: Return a cpp reference.
	const LinphoneErrorInfo *getErrorInfo() const;

	bool isRead() const;
	bool isReadOnly() const;
	void markAsRead();

	bool getToBeStored() const;
	virtual void setToBeStored(bool value);

	void setInAggregationQueue(bool isInQueue);

	const std::list<std::shared_ptr<Content>> &getContents() const;
	void addContent(std::shared_ptr<Content> content);
	void removeContent(std::shared_ptr<Content> content);

	std::list<ParticipantImdnState> getParticipantsByImdnState(State state) const;
	std::list<ParticipantImdnState> getParticipantsState() const;
	ChatMessage::State getParticipantState(const std::shared_ptr<Address> &address) const;

	const Content &getInternalContent() const;
	void setInternalContent(const Content &content);

	bool downloadFile(std::shared_ptr<FileTransferContent> content, bool retry = false);
	bool downloadFiles();
	bool downloadTerminated();
	bool isFileTransferInProgress() const;
	void fileUploadEndBackgroundTask();

	void addListener(std::shared_ptr<ChatMessageListener> listener);
	void removeListener(std::shared_ptr<ChatMessageListener> listener);

	// Core listener
	void onNetworkReachable(bool sipNetworkReachable, bool mediaNetworkReachable) override;
	void onAccountRegistrationStateChanged(std::shared_ptr<Account> account,
	                                       LinphoneRegistrationState state,
	                                       const std::string &message) override;

protected:
	explicit ChatMessage(ChatMessagePrivate &p);

private:
	ChatMessage(const std::shared_ptr<AbstractChatRoom> &chatRoom, ChatMessage::Direction direction);

	std::list<std::shared_ptr<Content>> contentsToDownload;
	belle_sip_source_t *mResendTimer = nullptr;

	belle_sip_source_t *getResendTimer() const;
	void createResendTimer();
	static int resendTimerExpired(void *data, unsigned int revents);
	int handleAutomaticResend();
	void stopResendTimer();

	L_DECLARE_PRIVATE(ChatMessage);
	L_DISABLE_COPY(ChatMessage);
};

std::ostream &operator<<(std::ostream &lhs, ChatMessage::State e);

class ChatMessageLogContextualizer : CoreLogContextualizer {
public:
	ChatMessageLogContextualizer(const LinphoneChatMessage *msg);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CHAT_MESSAGE_H_
