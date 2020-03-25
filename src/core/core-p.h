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

#ifndef _L_CORE_P_H_
#define _L_CORE_P_H_

#include <stdexcept>

#include "chat/chat-room/abstract-chat-room.h"
#include "core.h"
#include "db/main-db.h"
#include "object/object-p.h"
#include "sal/call-op.h"
#include "auth-info/auth-stack.h"
#include "conference/session/tone-manager.h"
#include "utils/background-task.h"
#include "call/audio-device/audio-device.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class CoreListener;
class EncryptionEngine;
class LocalConferenceListEventHandler;
class RemoteConferenceListEventHandler;

class CorePrivate : public ObjectPrivate {
public:
	class DatabaseConnectionFailure : public std::runtime_error {
	public:
		DatabaseConnectionFailure(const char *what) : std::runtime_error(what) {}
		DatabaseConnectionFailure(const std::string &what) : std::runtime_error(what) {}
	};
	CorePrivate();
	void init ();
	void registerListener (CoreListener *listener);
	void unregisterListener (CoreListener *listener);
	void uninit ();
	void shutdown();
	void stop();
	bool asyncStopDone();

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
	void setVideoWindowId (bool preview, void *id);

	bool setOutputAudioDevice(AudioDevice *audioDevice);
	bool setInputAudioDevice(AudioDevice *audioDevice);

	void loadChatRooms ();
	void handleEphemeralMessages (time_t currentTime);
	void initEphemeralMessages ();
	void updateEphemeralMessages (const std::shared_ptr<ChatMessage> &message);
	void sendDeliveryNotifications ();
	void insertChatRoom (const std::shared_ptr<AbstractChatRoom> &chatRoom);
	void insertChatRoomWithDb (const std::shared_ptr<AbstractChatRoom> &chatRoom, unsigned int notifyId = 0);
	std::shared_ptr<AbstractChatRoom> createBasicChatRoom (const ConferenceId &conferenceId, AbstractChatRoom::CapabilitiesMask capabilities, const std::shared_ptr<ChatRoomParams> &params);

	std::shared_ptr<ToneManager> getToneManager();

	//Base
	std::shared_ptr<AbstractChatRoom> createClientGroupChatRoom (
		const std::string &subject,
		const IdentityAddress &conferenceFactoryUri,
		const ConferenceId &conferenceId,
		const Content &content,
		AbstractChatRoom::CapabilitiesMask capabilities,
		const std::shared_ptr<ChatRoomParams> &params,
		bool fallback
	);
	std::shared_ptr<AbstractChatRoom> createClientGroupChatRoom (const std::string &subject,
								     const IdentityAddress &conferenceFactoryUri,
								     const ConferenceId &conferenceId,
								     const Content &content,
								     bool encrypted);
	std::shared_ptr<AbstractChatRoom> createClientGroupChatRoom (const std::string &subject,
								     const ConferenceId &conferenceId,
								     const Content &content,
								     bool encrypted);
	std::shared_ptr<AbstractChatRoom> createClientGroupChatRoom(const std::string &subject, bool fallback, bool encrypted);
	std::shared_ptr<AbstractChatRoom> createClientGroupChatRoom(const std::string &subject, const Address *localAddress, AbstractChatRoom::CapabilitiesMask capabilities, bool fallback);

	std::shared_ptr<AbstractChatRoom> createChatRoom(const std::shared_ptr<ChatRoomParams> &params,
							 const IdentityAddress &localAddr,
							 const std::string &subject,
							 const std::list<IdentityAddress> &participants);
	std::shared_ptr<AbstractChatRoom> createChatRoom(const std::shared_ptr<ChatRoomParams> &params,
							 const std::string &subject,
							 const std::list<IdentityAddress> &participants);
	std::shared_ptr<AbstractChatRoom> createChatRoom(const std::string &subject,
							 const std::list<IdentityAddress> &participants);
	std::shared_ptr<AbstractChatRoom> createChatRoom(const std::shared_ptr<ChatRoomParams> &params, const IdentityAddress &localAddr, const IdentityAddress &participant);
	std::shared_ptr<AbstractChatRoom> createChatRoom(const IdentityAddress &participant);
	
	void replaceChatRoom (const std::shared_ptr<AbstractChatRoom> &replacedChatRoom, const std::shared_ptr<AbstractChatRoom> &newChatRoom);
	void doLater(const std::function<void ()> &something);
	belle_sip_main_loop_t *getMainLoop();
	bool basicToFlexisipChatroomMigrationEnabled()const;
	std::unique_ptr<MainDb> mainDb;
#ifdef HAVE_ADVANCED_IM
	std::unique_ptr<RemoteConferenceListEventHandler> remoteListEventHandler;
	std::unique_ptr<LocalConferenceListEventHandler> localListEventHandler;
#endif
	AuthStack &getAuthStack(){
		return authStack;
	}
	Sal * getSal();
	LinphoneCore *getCCore();

	void startEphemeralMessageTimer (time_t expireTime);
	void stopEphemeralMessageTimer ();

	void computeAudioDevicesList ();

private:
	bool isInBackground = false;
	bool isFriendListSubscriptionEnabled = false;
	static int ephemeralMessageTimerExpired (void *data, unsigned int revents);

	std::list<CoreListener *> listeners;

	std::list<std::shared_ptr<Call>> calls;
	std::shared_ptr<Call> currentCall;

	std::unordered_map<ConferenceId, std::shared_ptr<AbstractChatRoom>> chatRoomsById;

	std::unique_ptr<EncryptionEngine> imee;

	std::list<std::string> specs;

	std::shared_ptr<ToneManager> toneManager;

	// This is to keep a ref on a clientGroupChatRoom while it is being created
	// Otherwise the chatRoom will be freed() before it is inserted
	std::unordered_map<const AbstractChatRoom *, std::shared_ptr<const AbstractChatRoom>> noCreatedClientGroupChatRooms;
	AuthStack authStack;

	std::list<std::shared_ptr<ChatMessage>> ephemeralMessages;
	belle_sip_source_t *timer = nullptr;

	std::list<AudioDevice *> audioDevices;
	L_DECLARE_PUBLIC(Core);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CORE_P_H_
