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

#ifndef _L_CHAT_MESSAGE_H_
#define _L_CHAT_MESSAGE_H_

#include <list>

#include "linphone/api/c-types.h"
#include "linphone/enums/chat-message-enums.h"

// TODO: Remove me later?
#include "address/identity-address.h"

#include "core/core-accessor.h"
#include "object/object.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class AbstractChatRoom;
class Content;
class FileContent;
class FileTransferContent;
class ChatMessagePrivate;
class Participant;
class ParticipantImdnState;

class LINPHONE_PUBLIC ChatMessage : public Object, public CoreAccessor {
	friend class BasicToClientGroupChatRoom;
	friend class BasicToClientGroupChatRoomPrivate;
	friend class ChatRoom;
	friend class ChatRoomPrivate;
	friend class CpimChatMessageModifier;
	friend class FileTransferChatMessageModifier;
	friend class Imdn;
	friend class ImdnMessagePrivate;
	friend class MainDb;
	friend class MainDbPrivate;
	friend class ClientGroupChatRoomPrivate;
	friend class ServerGroupChatRoomPrivate;
	friend class LimeX3dhEncryptionEngine;
	friend class CorePrivate;

public:
	L_OVERRIDE_SHARED_FROM_THIS(ChatMessage);

	enum class State{
		Idle = LinphoneChatMessageStateIdle,
		InProgress = LinphoneChatMessageStateInProgress,
		Delivered = LinphoneChatMessageStateDelivered,
		NotDelivered = LinphoneChatMessageStateNotDelivered,
		FileTransferError = LinphoneChatMessageStateFileTransferError,
		FileTransferDone = LinphoneChatMessageStateFileTransferDone,
		DeliveredToUser = LinphoneChatMessageStateDeliveredToUser,
		Displayed = LinphoneChatMessageStateDisplayed,
		FileTransferInProgress
	};

	//casting to int to get rid of the enum compare warning.
	//Here we are comparing two enums serving the same purpose
	static_assert((int)ChatMessage::State::FileTransferInProgress == (int)LinphoneChatMessageStateFileTransferInProgress, "LinphoneChatMessageState and ChatMessage::State are not synchronized, fix this !");

	enum class Direction{
		Incoming = LinphoneChatMessageDirectionIncoming,
		Outgoing
	};

	//casting to int to get rid of the enum compare warning.
	//Here we are comparing two enums serving the same purpose
	static_assert((int)ChatMessage::Direction::Outgoing == (int)LinphoneChatMessageDirectionOutgoing, "LinphoneChatMessageDirection and ChatMessage::Direction are not synchronized, fix this !");

	virtual ~ChatMessage ();

	// ----- TODO: Remove me.
	void cancelFileTransfer ();
	int putCharacter (uint32_t character);
	void setIsSecured (bool isSecured);
	// ----- TODO: Remove me.

	long long getStorageId () const;
	void deleteChatMessageFromCache ();

	std::shared_ptr<AbstractChatRoom> getChatRoom () const;

	void send ();

	time_t getTime () const;

	bool isSecured () const;
	bool isValid () const;
	State getState () const;
	Direction getDirection () const;

	const std::string &getImdnMessageId () const;

	const IdentityAddress &getAuthenticatedFromAddress () const;
	const IdentityAddress &getFromAddress () const;
	const IdentityAddress &getToAddress () const;
	const IdentityAddress &getLocalAdress () const;
	const std::string &getForwardInfo () const;

	bool isEphemeral () const;
	long getEphemeralLifetime () const;
	time_t getEphemeralExpireTime () const;

	// TODO: Return a cpp reference.
	const LinphoneErrorInfo *getErrorInfo () const;

	bool isRead () const;
	bool isReadOnly () const;

	bool getToBeStored () const;
	virtual void setToBeStored (bool value);


	std::list<ParticipantImdnState> getParticipantsByImdnState (State state) const;

	const std::list<Content *> &getContents () const;
	void addContent (Content *content);
	void removeContent (Content *content);

	const Content &getInternalContent () const;
	void setInternalContent (const Content &content);

	bool downloadFile (FileTransferContent *content);
	bool isFileTransferInProgress () const;
	void fileUploadEndBackgroundTask ();

protected:
	explicit ChatMessage (ChatMessagePrivate &p);

private:
	ChatMessage (const std::shared_ptr<AbstractChatRoom> &chatRoom, ChatMessage::Direction direction);

	L_DECLARE_PRIVATE(ChatMessage);
	L_DISABLE_COPY(ChatMessage);
};

std::ostream& operator<<(std::ostream& lhs, ChatMessage::State e);

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CHAT_MESSAGE_H_
