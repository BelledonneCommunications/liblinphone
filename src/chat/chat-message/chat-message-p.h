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

#ifndef _L_CHAT_MESSAGE_P_H_
#define _L_CHAT_MESSAGE_P_H_

#include <belle-sip/types.h>

#include "chat/chat-message/chat-message.h"
#include "conference/conference-id.h"
#include "chat/modifier/file-transfer-chat-message-modifier.h"
#include "chat/notification/imdn.h"
#include "conference/conference-id.h"
#include "content/content-type.h"
#include "content/content.h"
#include "content/file-content.h"
#include "content/file-transfer-content.h"
#include "db/main-db-chat-message-key.h"
#include "db/main-db.h"
#include "event-log/conference/conference-chat-message-event.h"
#include "object/object-p.h"
#include "sal/sal.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ChatMessagePrivate : public ObjectPrivate {
	friend class CpimChatMessageModifier;
	friend class EncryptionChatMessageModifier;
	friend class MultipartChatMessageModifier;
	friend class NotificationMessagePrivate;

public:
	enum Step {
		None = 1 << 0,
		FileUpload = 1 << 1,
		Multipart = 1 << 2,
		Encryption = 1 << 3,
		Cpim = 1 << 4,
		Started = 1 << 5,
		FileDownload = 1 << 6,
		AutoFileDownload = 1 << 7,
		Sent = 1 << 6,
	};

	void setApplyModifiers (bool value) { applyModifiers = value; }

	void setDirection (ChatMessage::Direction dir);

	void setParticipantState (const IdentityAddress &participantAddress, ChatMessage::State newState, time_t stateChangeTime);

	virtual void setState (ChatMessage::State newState);
	void forceState (ChatMessage::State newState) {
		state = newState;
	}

	void setTime (time_t time);

	void setIsReadOnly (bool readOnly);

	void markAsRead ();

	bool isMarkedAsRead () const;

	void setImdnMessageId (const std::string &imdnMessageId);
	
	void setForwardInfo (const std::string &fInfo);

	void enableEphemeralWithTime (long time);

	void setEphemeralExpireTime (time_t expireTime);

	void setAuthenticatedFromAddress (const IdentityAddress &authenticatedFromAddress) {
		this->authenticatedFromAddress = authenticatedFromAddress;
	}

	void forceFromAddress (const IdentityAddress &fromAddress) {
		this->fromAddress = fromAddress;
	}

	void forceToAddress (const IdentityAddress &toAddress) {
		this->toAddress = toAddress;
	}

	void markContentsAsNotLoaded () {
		contentsNotLoadedFromDatabase = true;
	}

	void loadContentsFromDatabase () const;

	std::list<Content* > &getContents () {
		loadContentsFromDatabase();
		return contents;
	}

	const std::list<Content* > &getContents () const {
		loadContentsFromDatabase();
		return contents;
	}

	void setAutoFileTransferDownloadHappened(bool yesno) {
		isAutoDownloadAttachedFilesHappened = yesno;
	}

	bool isAutoFileTransferDownloadHappened() const {
		return isAutoDownloadAttachedFilesHappened;
	}

	belle_http_request_t *getHttpRequest () const;
	void setHttpRequest (belle_http_request_t *request);

	bool getDisplayNotificationRequired () const { return displayNotificationRequired; }
	bool getNegativeDeliveryNotificationRequired () const { return negativeDeliveryNotificationRequired; }
	bool getPositiveDeliveryNotificationRequired () const { return positiveDeliveryNotificationRequired; }
	virtual void setDisplayNotificationRequired (bool value) { displayNotificationRequired = value; }
	virtual void setNegativeDeliveryNotificationRequired (bool value) { negativeDeliveryNotificationRequired = value; }
	virtual void setPositiveDeliveryNotificationRequired (bool value) { positiveDeliveryNotificationRequired = value; }

	SalOp *getSalOp () const;
	void setSalOp (SalOp *op);

	void disableDeliveryNotificationRequiredInDatabase ();
	void disableDisplayNotificationRequiredInDatabase ();

	SalCustomHeader *getSalCustomHeaders () const;
	void setSalCustomHeaders (SalCustomHeader *headers);

	void addSalCustomHeader (const std::string &name, const std::string &value);
	void removeSalCustomHeader (const std::string &name);
	std::string getSalCustomHeaderValue (const std::string &name) const;

	void loadFileTransferUrlFromBodyToContent ();
	std::string createFakeFileTransferFromUrl(const std::string &url);

	void setChatRoom (const std::shared_ptr<AbstractChatRoom> &chatRoom);

	void setEncryptionPrevented (bool value) { encryptionPrevented = value; }

	void doNotRetryAutoDownload() {
		currentRecvStep |= ChatMessagePrivate::Step::AutoFileDownload;
	}
	void enableSenderAuthentication (bool value) { senderAuthenticationEnabled = value; }

	void setUnencryptedContentWarning (bool value) { unencryptedContentWarning = value; }
	bool getUnencryptedContentWarning () { return unencryptedContentWarning; }

	// -----------------------------------------------------------------------------
	// Deprecated methods only used for C wrapper, to be removed some day...
	// -----------------------------------------------------------------------------

	const ContentType &getContentType () const;
	void setContentType (const ContentType &contentType);

	const std::string &getText () const;
	void setText (const std::string &text);

	const std::string &getFileTransferFilepath () const;
	void setFileTransferFilepath (const std::string &path);

	const std::string &getAppdata () const;
	void setAppdata (const std::string &appData);

	const std::string &getExternalBodyUrl () const;
	void setExternalBodyUrl (const std::string &url);

	bool hasTextContent () const;
	const Content *getTextContent () const;

	bool hasFileTransferContent () const;
	const Content *getFileTransferContent () const;
	const Content *getFileTransferInformation () const;

	void addContent (Content *content);
	void removeContent (Content *content);

	bool downloadFile ();

	void notifyReceiving ();
	LinphoneReason receive ();
	void send ();

	void storeInDb ();
	void updateInDb ();

	static bool isValidStateTransition (ChatMessage::State currentState, ChatMessage::State newState);

	void restoreFileTransferContentAsFileContent();

private:
	ChatMessagePrivate(const std::shared_ptr<AbstractChatRoom> &cr, ChatMessage::Direction dir);

public:
	mutable MainDbChatMessageKey dbKey;

protected:
	bool displayNotificationRequired = true;
	bool negativeDeliveryNotificationRequired = true;
	bool positiveDeliveryNotificationRequired = true;
	bool toBeStored = true;
	std::string contentEncoding;

private:
	// TODO: Clean attributes.
	time_t time = ::ms_time(0); // TODO: Change me in all files.
	std::string imdnId;
	std::string rttMessage;
	std::string externalBodyUrl;
	bool isSecured = false;
	mutable bool isReadOnly = false;
	bool markedAsRead = false;
	Content internalContent;
	bool isAutoDownloadAttachedFilesHappened = false;

	// TODO: to replace salCustomheaders
	std::unordered_map<std::string, std::string> customHeaders;

	mutable LinphoneErrorInfo *errorInfo = nullptr;
	SalOp *salOp = nullptr;
	SalCustomHeader *salCustomHeaders = nullptr;
	unsigned char currentSendStep = Step::None;
	unsigned char currentRecvStep = Step::None;
	bool applyModifiers = true;
	FileTransferChatMessageModifier fileTransferChatMessageModifier;

	// Cache for returned values, used for compatibility with previous C API
	std::string fileTransferFilePath;
	mutable ContentType cContentType;
	mutable std::string cText;

	// TODO: Remove my comment. VARIABLES OK.
	// Do not expose.

	std::weak_ptr<AbstractChatRoom> chatRoom;
	ConferenceId conferenceId;
	IdentityAddress fromAddress;
	IdentityAddress authenticatedFromAddress;
	bool senderAuthenticationEnabled = true;
	bool unencryptedContentWarning = false;
	IdentityAddress toAddress;

	ChatMessage::State state = ChatMessage::State::Idle;
	ChatMessage::Direction direction = ChatMessage::Direction::Incoming;
	std::string forwardInfo;

	bool isEphemeral = false;
	long ephemeralLifetime = 86400; // 24h
	time_t ephemeralExpireTime = 0x7fffffff; // Max number of time_t

	std::list<Content *> contents;

	bool encryptionPrevented = false;
	mutable bool contentsNotLoadedFromDatabase = false;
	L_DECLARE_PUBLIC(ChatMessage);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CHAT_MESSAGE_P_H_
