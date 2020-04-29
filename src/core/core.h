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

#ifndef _L_CORE_H_
#define _L_CORE_H_

#include <list>
#include <functional>

#include "object/object.h"

#include "linphone/types.h"

// =============================================================================

L_DECL_C_STRUCT(LinphoneCore);

typedef struct belle_sip_source belle_sip_source_t;

LINPHONE_BEGIN_NAMESPACE

class AbstractChatRoom;
class Address;
class Call;
class ConferenceId;
class CorePrivate;
class IdentityAddress;
class EncryptionEngine;
class ChatMessage;
class ChatRoom;
class PushNotificationMessage;

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
	friend class LocalConferenceEventHandler;
	friend class MainDb;
	friend class MainDbChatMessageKey;
	friend class MainDbEventKey;
	friend class MediaSessionPrivate;
	friend class RealTimeTextChatRoomPrivate;
	friend class RemoteConferenceEventHandler;
	friend class RemoteConferenceListEventHandler;
	friend class ServerGroupChatRoom;
	friend class ServerGroupChatRoomPrivate;
	friend class CallSessionPrivate;
	friend class ToneManager;

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
	bool isInBackground ();

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
	void soundcardActivateAudioSession (bool active);
	void soundcardEnableCallkit (bool enabled);
	LinphoneStatus terminateAllCalls ();

	// ---------------------------------------------------------------------------
	// ChatRoom.
	// ---------------------------------------------------------------------------

	std::list<std::shared_ptr<AbstractChatRoom>> getChatRooms () const;

	std::shared_ptr<AbstractChatRoom> findChatRoom (const ConferenceId &conferenceId, bool logIfNotFound = true) const;
	std::list<std::shared_ptr<AbstractChatRoom>> findChatRooms (const IdentityAddress &peerAddress) const;

	std::shared_ptr<AbstractChatRoom> findOneToOneChatRoom (
		const IdentityAddress &localAddress,
		const IdentityAddress &participantAddress,
		bool basicOnly,
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
	std::string getX3dhServerUrl () const;
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

	void pushNotificationReceived () const;
	int getUnreadChatMessageCount () const;
	int getUnreadChatMessageCount (const IdentityAddress &localAddress) const;
	int getUnreadChatMessageCountFromActiveLocals () const;
	std::shared_ptr<PushNotificationMessage> getPushNotificationMessage (const std::string &callId) const;
	std::shared_ptr<ChatRoom> getPushNotificationChatRoom (const std::string &chatRoomAddr) const;
	std::shared_ptr<ChatMessage> findChatMessageFromCallId (const std::string &callId) const;

	Address interpretUrl (const std::string &url) const;
	// Execute specified lambda later in main loop. This method can be used from any thread to execute something later on main thread.
	void doLater(const std::function<void ()> &something);

	/*
	 * Run supplied std::function as a timer. It should return true if repeated, false otherwise.
	 * The returned belle_sip_source_t must be unrefed (with belle_sip_object_unref() ).
	 * It may be unrefed before expiration, if this timer never needs to be cancelled.
	 */
	belle_sip_source_t *createTimer(const std::function<bool ()> &something, unsigned int millisecond, const std::string &name);
	/* Stop (ie cancel) and destroy a timer created by createTimer()*/

	void destroyTimer(belle_sip_source_t *timer);
private:
	Core ();

	L_DECLARE_PRIVATE(Core);
	L_DISABLE_COPY(Core);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CORE_H_
