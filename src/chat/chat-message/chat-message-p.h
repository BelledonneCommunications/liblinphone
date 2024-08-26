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

#ifndef _L_CHAT_MESSAGE_P_H_
#define _L_CHAT_MESSAGE_P_H_

#include <belle-sip/types.h>

#include "chat/chat-message/chat-message.h"
#include "chat/modifier/file-transfer-chat-message-modifier.h"
#include "conference/conference-id.h"
#include "content/content-type.h"
#include "content/content.h"
#include "content/file-content.h"
#include "content/file-transfer-content.h"
#include "event-log/conference/conference-chat-message-event.h"
#include "object/object-p.h"
#include "sal/sal.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class AbstractChatRoom;

class ChatMessagePrivate : public ObjectPrivate {
	friend class CpimChatMessageModifier;
	friend class EncryptionChatMessageModifier;
	friend class MultipartChatMessageModifier;
	friend class NotificationMessagePrivate;

public:
	enum Step : int {
		None = 1 << 0,
		FileUpload = 1 << 1,
		Multipart = 1 << 2,
		Encryption = 1 << 3,
		Cpim = 1 << 4,
		Started = 1 << 5,
		FileDownload = 1 << 6,
		AutoFileDownload = 1 << 7,
		Sent = 1 << 8,
		Sending = 1 << 9
	};

	void setApplyModifiers(bool value) {
		applyModifiers = value;
	}

	void setStorageId(long long id);
	void resetStorageId();

	void setDirection(ChatMessage::Direction dir);

	void setParticipantState(const std::shared_ptr<Address> &participantAddress,
	                         ChatMessage::State newState,
	                         time_t stateChangeTime,
	                         LinphoneReason reason = LinphoneReasonNone);

	void forceState(ChatMessage::State newState) {
		state = newState;
	}

	void setTime(time_t time);

	void setIsReadOnly(bool readOnly);

	void markAsRead();

	bool isMarkedAsRead() const;

	void setImdnMessageId(const std::string &imdnMessageId);

	void setForwardInfo(const std::string &fInfo);
	void setReplyToMessageIdAndSenderAddress(const std::string &id, const std::shared_ptr<Address> &sender);
	void setReactionToMessageId(const std::string &id);

	void enableEphemeralWithTime(long time);

	void setEphemeralExpireTime(time_t expireTime);

	void startEphemeralCountDown();

	void setAuthenticatedFromAddress(const Address &authenticatedFromAddress) {
		this->authenticatedFromAddress = authenticatedFromAddress;
	}

	void forceFromAddress(const std::shared_ptr<Address> &fromAddress) {
		this->fromAddress = fromAddress;
	}

	void forceToAddress(const std::shared_ptr<Address> &toAddress) {
		this->toAddress = toAddress;
	}

	// Used by the ConferenceScheduler to keep track of the recipient Address in One-To-One Flexisip chat room
	void setRecipientAddress(const std::shared_ptr<Address> &recipientAddress) {
		this->recipientAddress = recipientAddress;
	}

	void markContentsAsNotLoaded() {
		contentsNotLoadedFromDatabase = true;
	}

	void loadContentsFromDatabase() const;

	std::list<std::shared_ptr<Content>> &getContents() {
		loadContentsFromDatabase();
		return contents;
	}

	const std::list<std::shared_ptr<Content>> &getContents() const {
		loadContentsFromDatabase();
		return contents;
	}

	void setAutoFileTransferDownloadInProgress(bool yesno) {
		isAutoDownloadAttachedFilesInProgress = yesno;
	}

	bool isAutoFileTransferDownloadInProgress() const {
		return isAutoDownloadAttachedFilesInProgress;
	}

	void handleAutoDownload();

	belle_http_request_t *getHttpRequest() const;
	void setHttpRequest(belle_http_request_t *request);

	bool getDisplayNotificationRequired() const {
		return displayNotificationRequired;
	}
	bool getNegativeDeliveryNotificationRequired() const {
		return negativeDeliveryNotificationRequired;
	}
	bool getPositiveDeliveryNotificationRequired() const {
		return positiveDeliveryNotificationRequired;
	}
	virtual void setDisplayNotificationRequired(bool value) {
		displayNotificationRequired = value;
	}
	virtual void setNegativeDeliveryNotificationRequired(bool value) {
		negativeDeliveryNotificationRequired = value;
	}
	virtual void setPositiveDeliveryNotificationRequired(bool value) {
		positiveDeliveryNotificationRequired = value;
	}

	SalOp *getSalOp() const;
	void setSalOp(SalOp *op);

	void disableDeliveryNotificationRequiredInDatabase();
	void disableDisplayNotificationRequiredInDatabase();

	SalCustomHeader *getSalCustomHeaders() const;
	void setSalCustomHeaders(SalCustomHeader *headers);

	void addSalCustomHeader(const std::string &name, const std::string &value);
	void removeSalCustomHeader(const std::string &name);
	std::string getSalCustomHeaderValue(const std::string &name) const;

	void loadFileTransferUrlFromBodyToContent();
	std::string createFakeFileTransferFromUrl(const std::string &url);

	void setChatRoom(const std::shared_ptr<AbstractChatRoom> &chatRoom);

	void setEncryptionPrevented(bool value) {
		encryptionPrevented = value;
	}

	void doNotRetryAutoDownload() {
		currentRecvStep |= ChatMessagePrivate::Step::AutoFileDownload;
	}
	void enableSenderAuthentication(bool value) {
		senderAuthenticationEnabled = value;
	}

	void setUnencryptedContentWarning(bool value) {
		unencryptedContentWarning = value;
	}
	bool getUnencryptedContentWarning() {
		return unencryptedContentWarning;
	}

	// -----------------------------------------------------------------------------
	// Deprecated methods only used for C wrapper, to be removed some day...
	// -----------------------------------------------------------------------------

	const ContentType &getContentType() const;
	void setContentType(const ContentType &contentType);

	const std::string &getText() const;    // Deprecated. Use UTF8
	void setText(const std::string &text); // Deprecated. Use UTF8

	const std::string &getUtf8Text() const;
	void setUtf8Text(const std::string &text);

	const std::string &getFileTransferFilepath() const;
	void setFileTransferFilepath(const std::string &path);

	const std::string &getAppdata() const;
	void setAppdata(const std::string &appData);

	const std::string &getExternalBodyUrl() const;
	void setExternalBodyUrl(const std::string &url);

	const std::string &getMessageId() const;
	void setMessageId(const std::string &id);

	const std::string &getCallId() const;
	void setCallId(const std::string &id);

	bool hasTextContent() const;
	const std::shared_ptr<Content> getTextContent() const;
	bool hasConferenceInvitationContent() const;

	bool hasFileTransferContent() const;
	const std::shared_ptr<Content> getFileTransferContent() const;
	const std::shared_ptr<Content> getFileTransferInformation() const;

	void addContent(std::shared_ptr<Content> content);
	void removeContent(std::shared_ptr<Content> content);
	void replaceContent(std::shared_ptr<Content> contentToRemove, std::shared_ptr<Content> contentToAdd);

	bool downloadFile();

	void endMessageReception();
	LinphoneReason receive();
	void send();

	void storeInDb();
	void updateInDb();

	static bool isImdnControlledState(ChatMessage::State state);

	void restoreFileTransferContentAsFileContent();

	void setAutomaticallyResent(bool enable);

	long long storageId = -1;

protected:
	bool displayNotificationRequired = true;
	bool negativeDeliveryNotificationRequired = true;
	bool positiveDeliveryNotificationRequired = true;
	bool toBeStored = true;
	bool mAutomaticallyResent = false;
	std::string contentEncoding;

private:
	// Keep setState private as the chat message state must only be set through setParticipantState
	virtual void setState(ChatMessage::State newState);

	ChatMessagePrivate(const std::shared_ptr<AbstractChatRoom> &chatRoom, ChatMessage::Direction dir);
	virtual ~ChatMessagePrivate();

	// TODO: Clean attributes.
	time_t time = ::ms_time(0); // TODO: Change me in all files.
	std::string imdnId;
	std::string rttMessage;
	std::string externalBodyUrl;
	bool isSecured = false;
	mutable bool isReadOnly = false;
	bool isAutoDownloadAttachedFilesInProgress = false;
	bool markedAsRead = false;
	Content internalContent;
	std::string callId;
	std::string mMessageId;

	mutable LinphoneErrorInfo *errorInfo = nullptr;
	SalOp *salOp = nullptr;
	SalCustomHeader *salCustomHeaders = nullptr;
	int currentSendStep = Step::None;
	int currentRecvStep = Step::None;
	bool applyModifiers = true;
	FileTransferChatMessageModifier fileTransferChatMessageModifier;

	// Cache for returned values, used for compatibility with previous C API
	std::string fileTransferFilePath;
	mutable ContentType cContentType;
	mutable std::string cText;

	// TODO: Remove my comment. VARIABLES OK.
	// Do not expose.

	std::weak_ptr<AbstractChatRoom> mChatRoom;
	ConferenceId conferenceId;
	std::shared_ptr<Address> mMeAddress;
	std::shared_ptr<Address> fromAddress;
	Address authenticatedFromAddress;
	bool senderAuthenticationEnabled = true;
	bool unencryptedContentWarning = false;
	std::shared_ptr<Address> toAddress;

	ChatMessage::State state = ChatMessage::State::Idle;
	ChatMessage::Direction direction = ChatMessage::Direction::Incoming;
	std::string forwardInfo;
	std::string replyingToMessageId;
	std::string reactionToMessageId;
	std::shared_ptr<Address> replyingToMessageSender;
	std::shared_ptr<Address> recipientAddress;

	bool isEphemeral = false;
	long ephemeralLifetime = 0;
	time_t ephemeralExpireTime = 0;

	std::list<std::shared_ptr<Content>> contents;
	mutable std::list<std::shared_ptr<ChatMessageReaction>> reactions;

	bool encryptionPrevented = false;
	mutable bool contentsNotLoadedFromDatabase = false;
	bool isInAggregationQueue = false;

	std::list<std::shared_ptr<ChatMessageListener>> listeners;

	L_DECLARE_PUBLIC(ChatMessage);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CHAT_MESSAGE_P_H_
