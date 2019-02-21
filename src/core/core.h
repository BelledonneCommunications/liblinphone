/*
 * core.h
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

#ifndef _L_CORE_H_
#define _L_CORE_H_

#include <list>
#include <functional>

#include "object/object.h"

#include "linphone/types.h"

// =============================================================================

L_DECL_C_STRUCT(LinphoneCore);

LINPHONE_BEGIN_NAMESPACE

class ChatRoomParams;
class AbstractChatRoom;
class Address;
class Call;
class ConferenceId;
class CorePrivate;
class IdentityAddress;
class EncryptionEngine;

class LINPHONE_PUBLIC Core : public Object {
	friend class BasicToClientGroupChatRoom;
	friend class BasicToClientGroupChatRoomPrivate;
	friend class CallPrivate;
	friend class CallSession;
	friend class ChatMessage;
	friend class ChatMessagePrivate;
	friend class ChatRoom;
	friend class ChatRoomPrivate;
	friend class ClientGroupChatRoom;
	friend class ClientGroupChatRoomPrivate;
	friend class ClientGroupToBasicChatRoomPrivate;
	friend class Imdn;
	friend class LocalConferenceEventHandlerPrivate;
	friend class MainDb;
	friend class MainDbChatMessageKey;
	friend class MainDbEventKey;
	friend class MediaSessionPrivate;
	friend class RealTimeTextChatRoomPrivate;
	friend class RemoteConferenceEventHandler;
	friend class RemoteConferenceListEventHandler;
	friend class ServerGroupChatRoom;
	friend class ServerGroupChatRoomPrivate;

public:
	L_OVERRIDE_SHARED_FROM_THIS(Core);

	~Core ();

	// Return a new Core instance. Entry point of Linphone.
	static std::shared_ptr<Core> create (LinphoneCore *cCore);

	// ---------------------------------------------------------------------------
	// Application lifecycle.
	// ---------------------------------------------------------------------------

	void enterBackground ();
	void enterForeground ();

	// ---------------------------------------------------------------------------
	// C-Core.
	// ---------------------------------------------------------------------------

	// TODO: Remove me later.
	LinphoneCore *getCCore () const;

	// ---------------------------------------------------------------------------
	// Call.
	// ---------------------------------------------------------------------------

	bool areSoundResourcesLocked () const;
	std::shared_ptr<Call> getCallByRemoteAddress (const Address &addr) const;
	const std::list<std::shared_ptr<Call>> &getCalls () const;
	unsigned int getCallCount () const;
	std::shared_ptr<Call> getCurrentCall () const;
	LinphoneStatus pauseAllCalls ();
	void soundcardHintCheck ();
	LinphoneStatus terminateAllCalls ();

	// ---------------------------------------------------------------------------
	// ChatRoom.
	// ---------------------------------------------------------------------------

	const std::list<std::shared_ptr<AbstractChatRoom>> &getChatRooms () const;

	std::shared_ptr<AbstractChatRoom> findChatRoom (const ConferenceId &conferenceId) const;
	std::list<std::shared_ptr<AbstractChatRoom>> findChatRooms (const IdentityAddress &peerAddress) const;

	std::shared_ptr<AbstractChatRoom> findOneToOneChatRoom (
		const IdentityAddress &localAddress,
		const IdentityAddress &participantAddress,
		bool encrypted
	) const;

	std::shared_ptr<AbstractChatRoom> createClientGroupChatRoom (const std::string &subject, bool fallback = true);
	std::shared_ptr<AbstractChatRoom> createClientGroupChatRoom (const std::string &subject,
								     LinphoneChatRoomCapabilitiesMask capabilities,
								     bool fallback = true);
	std::shared_ptr<AbstractChatRoom> createClientGroupChatRoom (const std::string &subject,
								     const Address *localAddress,
								     LinphoneChatRoomCapabilitiesMask capabilities,
								     bool fallback = true);


	/* std::shared_ptr<AbstractChatRoom> createChatRoom(const ChatRoomParams *params, */
	/* 						 const std::string &subject, */
	/* 						 const std::list<Address> &participants); */
	/* std::shared_ptr<AbstractChatRoom> createChatRoom(const std::string &subject, */
	/* 						 const std::list<Address> &participants); */
	/* std::shared_ptr<AbstractChatRoom> createChatRoom(const ChatRoomParams *params, const IdentityAddress &participant); */
	/* std::shared_ptr<AbstractChatRoom> createChatRoom(const IdentityAddress &participant); */


	std::shared_ptr<AbstractChatRoom> getOrCreateBasicChatRoom (const ConferenceId &conferenceId, bool isRtt = false);

	//TODO: Remove me in the future, a chatroom is identified by a local and peer address now!
	std::shared_ptr<AbstractChatRoom> getOrCreateBasicChatRoom (const IdentityAddress &peerAddress, bool isRtt = false);

	std::shared_ptr<AbstractChatRoom> getOrCreateBasicChatRoomFromUri (const std::string &uri, bool isRtt = false);

	static void deleteChatRoom (const std::shared_ptr<const AbstractChatRoom> &chatRoom);

	// ---------------------------------------------------------------------------
	// Paths.
	// ---------------------------------------------------------------------------

	std::string getDataPath() const;
	std::string getConfigPath() const;
	std::string getDownloadPath() const;

	// ---------------------------------------------------------------------------
	// EncryptionEngine.
	// ---------------------------------------------------------------------------

	EncryptionEngine *getEncryptionEngine () const;
	void setEncryptionEngine (EncryptionEngine *imee);
	void enableLimeX3dh (bool enable);
	void setX3dhServerUrl (const std::string &url);
	bool limeX3dhEnabled () const;
	bool limeX3dhAvailable () const;

	// ---------------------------------------------------------------------------
	// Specs.
	// ---------------------------------------------------------------------------

	void setSpecsList(const std::list<std::string> &specsList);
	void addSpec (const std::string &spec);
	void removeSpec (const std::string &spec);
	const std::list<std::string> &getSpecsList () const;
	void setSpecs (const std::string &specs);
	std::string getSpecs() const;

	// ---------------------------------------------------------------------------
	// Friends.
	// ---------------------------------------------------------------------------

	void enableFriendListSubscription (bool enable);
	bool isFriendListSubscriptionEnabled () const;

	// ---------------------------------------------------------------------------
	// Misc.
	// ---------------------------------------------------------------------------

	int getUnreadChatMessageCount () const;
	int getUnreadChatMessageCount (const IdentityAddress &localAddress) const;
	int getUnreadChatMessageCountFromActiveLocals () const;

	Address interpretUrl (const std::string &url) const;
	// Execute specified lambda later in main loop. This method can be used from any thread to execute something later on main thread.
	void doLater(const std::function<void ()> &something);

private:
	Core ();

	L_DECLARE_PRIVATE(Core);
	L_DISABLE_COPY(Core);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CORE_H_
