/*
 * core-p.h
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

#ifndef _L_CORE_P_H_
#define _L_CORE_P_H_

#include "chat/chat-room/abstract-chat-room.h"
#include "core.h"
#include "db/main-db.h"
#include "object/object-p.h"
#include "sal/call-op.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class CoreListener;
class EncryptionEngine;
class LocalConferenceListEventHandler;
class RemoteConferenceListEventHandler;

class CorePrivate : public ObjectPrivate {
public:
	void init ();
	void registerListener (CoreListener *listener);
	void unregisterListener (CoreListener *listener);
	void uninit ();

	void notifyGlobalStateChanged (LinphoneGlobalState state);
	void notifyNetworkReachable (bool sipNetworkReachable, bool mediaNetworkReachable);
	void notifyRegistrationStateChanged (LinphoneProxyConfig *cfg, LinphoneRegistrationState state, const std::string &message);
	void notifyEnteringBackground ();
	void notifyEnteringForeground ();

	void enableFriendListsSubscription (bool enable);

	int addCall (const std::shared_ptr<Call> &call);
	bool canWeAddCall () const;
	bool hasCalls () const { return !calls.empty(); }
	bool inviteReplacesABrokenCall (SalCallOp *op);
	bool isAlreadyInCallWithAddress (const Address &addr) const;
	void iterateCalls (time_t currentRealTime, bool oneSecondElapsed) const;
	void notifySoundcardUsage (bool used);
	int removeCall (const std::shared_ptr<Call> &call);
	void setCurrentCall (const std::shared_ptr<Call> &call) { currentCall = call; }
	void unsetVideoWindowId (bool preview, void *id);

	void parameterizeEqualizer (AudioStream *stream);
	void postConfigureAudioStream (AudioStream *stream, bool muted);
	void setPlaybackGainDb (AudioStream *stream, float gain);

	void loadChatRooms ();
	void insertChatRoom (const std::shared_ptr<AbstractChatRoom> &chatRoom);
	void insertChatRoomWithDb (const std::shared_ptr<AbstractChatRoom> &chatRoom, unsigned int notifyId = 0);
	std::shared_ptr<AbstractChatRoom> createBasicChatRoom (const ConferenceId &conferenceId, AbstractChatRoom::CapabilitiesMask capabilities);

	std::shared_ptr<AbstractChatRoom> createClientGroupChatRoom (
		const std::string &subject,
		const ConferenceId &conferenceId,
		const Content &content,
		AbstractChatRoom::CapabilitiesMask capabilities
	);

	std::shared_ptr<AbstractChatRoom> createClientGroupChatRoom(const std::string &subject, bool fallback);
	std::shared_ptr<AbstractChatRoom> createClientGroupChatRoom(const std::string &subject, AbstractChatRoom::CapabilitiesMask capabilities, bool fallback);
	std::shared_ptr<AbstractChatRoom> createClientGroupChatRoom(const std::string &subject, const Address *localAddress, AbstractChatRoom::CapabilitiesMask capabilities, bool fallback);

	std::shared_ptr<AbstractChatRoom> createChatRoom(const ChatRoomParams *params,
							 const std::string &subject,
							 const std::list<Address> &participants);
	std::shared_ptr<AbstractChatRoom> createChatRoom(const std::string &subject,
							 const std::list<Address> &participants);
	std::shared_ptr<AbstractChatRoom> createChatRoom(const ChatRoomParams *params, const IdentityAddress &participant);
	std::shared_ptr<AbstractChatRoom> createChatRoom(const IdentityAddress &participant);

	void replaceChatRoom (const std::shared_ptr<AbstractChatRoom> &replacedChatRoom, const std::shared_ptr<AbstractChatRoom> &newChatRoom);
	void doLater(const std::function<void ()> &something);
	belle_sip_main_loop_t *getMainLoop();

	std::unique_ptr<MainDb> mainDb;
	std::unique_ptr<RemoteConferenceListEventHandler> remoteListEventHandler;
	std::unique_ptr<LocalConferenceListEventHandler> localListEventHandler;

private:
	bool isInBackground = false;
	bool isFriendListSubscriptionEnabled = false;

	std::list<CoreListener *> listeners;

	std::list<std::shared_ptr<Call>> calls;
	std::shared_ptr<Call> currentCall;

	std::list<std::shared_ptr<AbstractChatRoom>> chatRooms;

	std::unordered_map<ConferenceId, std::shared_ptr<AbstractChatRoom>> chatRoomsById;

	std::unique_ptr<EncryptionEngine> imee;

	std::list<std::string> specs;

	// Ugly cache to deal with C code.
	std::unordered_map<const AbstractChatRoom *, std::shared_ptr<const AbstractChatRoom>> noCreatedClientGroupChatRooms;

	L_DECLARE_PUBLIC(Core);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CORE_P_H_
