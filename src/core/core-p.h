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

#ifndef _L_CORE_P_H_
#define _L_CORE_P_H_

#include <stdexcept>

#include "linphone/utils/utils.h"

#include "auth-info/auth-stack.h"
#include "call/audio-device/audio-device.h"
#include "chat/chat-room/abstract-chat-room.h"
#include "conference/conference.h"
#include "conference/session/tone-manager.h"
#include "core.h"
#include "db/main-db.h"
#include "object/object-p.h"
#include "sal/call-op.h"
#include "utils/background-task.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class CoreListener;
class EncryptionEngine;
class ServerConferenceListEventHandler;
class ClientConferenceListEventHandler;

class CorePrivate : public ObjectPrivate {
	friend class AuthStack;

public:
	class DatabaseConnectionFailure : public std::runtime_error {
	public:
		DatabaseConnectionFailure(const char *what) : std::runtime_error(what) {
		}
		DatabaseConnectionFailure(const std::string &what) : std::runtime_error(what) {
		}
	};
	CorePrivate();
	void init();
	bool listenerAlreadyRegistered(CoreListener *listener) const;
	void registerListener(CoreListener *listener);
	void unregisterListener(CoreListener *listener);
	void uninit();
	void shutdown();
	void unregisterAccounts();
	bool isShutdownDone();
	void disconnectMainDb();

	void notifyGlobalStateChanged(LinphoneGlobalState state);
	void notifyNetworkReachable(bool sipNetworkReachable, bool mediaNetworkReachable);
	void notifyCallStateChanged(LinphoneCall *cfg, LinphoneCallState state, const std::string &message);
	void notifyRegistrationStateChanged(std::shared_ptr<Account> account,
	                                    LinphoneRegistrationState state,
	                                    const std::string &message);
	void notifyRegistrationStateChanged(LinphoneProxyConfig *cfg,
	                                    LinphoneRegistrationState state,
	                                    const std::string &message);
	void notifyEnteringBackground();
	void notifyEnteringForeground();

	void enableFriendListsSubscription(bool enable);
	void enableMessageWaitingIndicationSubscription(bool enable);

	int addCall(const std::shared_ptr<Call> &call);
	bool canWeAddCall() const;
	bool hasCalls() const {
		return !calls.empty();
	}
	bool inviteReplacesABrokenCall(SalCallOp *op);
	bool isAlreadyInCallWithAddress(const std::shared_ptr<Address> &addr) const;
	void iterateCalls(time_t currentRealTime, bool oneSecondElapsed) const;
	void notifySoundcardUsage(bool used);
	int removeCall(const std::shared_ptr<Call> &call);
	void setCurrentCall(const std::shared_ptr<Call> &call);
	void setVideoWindowId(bool preview, void *id);

	bool setOutputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice);
	bool setInputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice);

	void loadChatRooms();
	void handleEphemeralMessages(time_t currentTime);
	void initEphemeralMessages();
	void updateEphemeralMessages(const std::shared_ptr<ChatMessage> &message);
	void sendDeliveryNotifications();
	void insertChatRoom(const std::shared_ptr<AbstractChatRoom> &chatRoom);
	void insertChatRoomWithDb(const std::shared_ptr<AbstractChatRoom> &chatRoom, unsigned int notifyId = 0);
	std::shared_ptr<AbstractChatRoom> createBasicChatRoom(const ConferenceId &conferenceId,
	                                                      const std::shared_ptr<ConferenceParams> &params);

	ToneManager &getToneManager();

	void reloadLdapList();

	// Base
	std::shared_ptr<AbstractChatRoom> createServerChatRoom(const std::shared_ptr<Address> &conferenceFactoryUri,
	                                                       SalCallOp *op,
	                                                       const std::shared_ptr<ConferenceParams> &params);
	std::shared_ptr<AbstractChatRoom> createClientChatRoom(const std::shared_ptr<Address> &conferenceFactoryUri,
	                                                       const ConferenceId &conferenceId,
	                                                       SalCallOp *op,
	                                                       const std::shared_ptr<ConferenceParams> &params);
	std::shared_ptr<AbstractChatRoom> createClientChatRoom(const std::string &subject,
	                                                       const ConferenceId &conferenceId,
	                                                       SalCallOp *op,
	                                                       bool encrypted,
	                                                       AbstractChatRoom::EphemeralMode ephemerableMode,
	                                                       long ephemeralLifeTime);
	std::shared_ptr<AbstractChatRoom> createClientChatRoom(const std::string &subject, bool fallback, bool encrypted);

	std::shared_ptr<AbstractChatRoom> createChatRoom(const std::shared_ptr<ConferenceParams> &params,
	                                                 const std::shared_ptr<const Address> &localAddr,
	                                                 const std::list<std::shared_ptr<const Address>> &participants);
	std::shared_ptr<AbstractChatRoom> createChatRoom(const std::shared_ptr<ConferenceParams> &params,
	                                                 const std::list<std::shared_ptr<const Address>> &participants);
	std::shared_ptr<AbstractChatRoom> createChatRoom(const std::string &subject,
	                                                 const std::list<std::shared_ptr<const Address>> &participants);
	std::shared_ptr<AbstractChatRoom> createChatRoom(const std::shared_ptr<ConferenceParams> &params,
	                                                 const std::shared_ptr<Address> &localAddr,
	                                                 const std::shared_ptr<const Address> &participant);
	std::shared_ptr<AbstractChatRoom> createChatRoom(const std::shared_ptr<const Address> &participant);

	std::shared_ptr<AbstractChatRoom> searchChatRoom(const std::shared_ptr<ConferenceParams> &params,
	                                                 const std::shared_ptr<const Address> &localAddr,
	                                                 const std::shared_ptr<const Address> &remoteAddr,
	                                                 const std::list<std::shared_ptr<Address>> &participants) const;

	std::shared_ptr<const Address> getDefaultLocalAddress(const std::shared_ptr<const Address> peerAddress,
	                                                      bool withGruu) const;
	std::shared_ptr<const Address>
	getIdentityAddressWithGruu(const std::shared_ptr<const Address> &identityAddress) const;

	void updateChatRoomConferenceId(const std::shared_ptr<AbstractChatRoom> &chatRoom, ConferenceId newConferenceId);
	std::shared_ptr<AbstractChatRoom> findExhumableOneToOneChatRoom(const std::shared_ptr<Address> &localAddress,
	                                                                const std::shared_ptr<Address> &participantAddress,
	                                                                bool encrypted) const;
	std::shared_ptr<AbstractChatRoom> findExumedChatRoomFromPreviousConferenceId(const ConferenceId conferenceId) const;

	void stopChatMessagesAggregationTimer();
	void createConferenceCleanupTimer();
	void stopConferenceCleanupTimer();

	// Cancel task scheduled on the main loop
	void doLater(const std::function<void()> &something);
	belle_sip_main_loop_t *getMainLoop();
	std::unique_ptr<MainDb> mainDb;
#ifdef HAVE_ADVANCED_IM
	std::unique_ptr<ClientConferenceListEventHandler> clientListEventHandler;
	std::unique_ptr<ServerConferenceListEventHandler> serverListEventHandler;
#endif
	AuthStack &getAuthStack() {
		return authStack;
	}
	Sal *getSal();
	LinphoneCore *getCCore() const;

	void startEphemeralMessageTimer(time_t expireTime);
	void stopEphemeralMessageTimer();

	void computeAudioDevicesList();

	/* called by linphone_core_set_video_device() to update the video device in the running call or conference.*/
	void updateVideoDevice();

	/* centralized method to write down all NatPolicy used by Accounts or Core */
	void writeNatPolicyConfigurations();
	void reorderVideoCodecList();
	int getCodecPriority(const OrtpPayloadType *pt) const;
	static const Utils::Version conferenceProtocolVersion;
	static const Utils::Version groupChatProtocolVersion;
	static const Utils::Version ephemeralProtocolVersion;

private:
	void stopStartupBgTask();
	bool isInBackground = false;
	static int ephemeralMessageTimerExpired(void *data, unsigned int revents);

	std::list<CoreListener *> listeners;

	std::list<std::shared_ptr<Call>> calls;
	std::shared_ptr<Call> currentCall;

	std::unordered_map<ConferenceId, std::shared_ptr<AbstractChatRoom>, ConferenceId::WeakHash, ConferenceId::WeakEqual>
	    chatRoomsById;
	std::unordered_map<ConferenceId, std::shared_ptr<Conference>, ConferenceId::WeakHash, ConferenceId::WeakEqual>
	    conferenceById;

	std::unique_ptr<EncryptionEngine> imee;

	std::map<std::string, std::string> specs;

	std::unique_ptr<ToneManager> toneManager;

	AuthStack authStack;

	std::list<std::shared_ptr<ChatMessage>> ephemeralMessages;
	belle_sip_source_t *ephemeralTimer = nullptr;
	belle_sip_source_t *mConferenceCleanupTimer = nullptr;

	belle_sip_source_t *chatMessagesAggregationTimer = nullptr;
	BackgroundTask chatMessagesAggregationBackgroundTask{"Chat messages aggregation"};

	BackgroundTask pushReceivedBackgroundTask{"Push received background task"};
	std::string lastPushReceivedCallId = "";

	std::list<std::shared_ptr<AudioDevice>> audioDevices;
	bool stopAsyncEndEnabled = false;
	ExtraBackgroundTask bgTask{"Stop core async end"};
	BackgroundTask coreStartupTask{"Core startup until registration"};

	std::list<std::shared_ptr<Ldap>> mLdapServers; // Persistent list of LDAP servers
	std::string logLabel;
	LinphoneCodecPriorityPolicy videoCodecPriorityPolicy = LinphoneCodecPriorityPolicyAuto;
	std::unique_ptr<HttpClient> httpClient;

	L_DECLARE_PUBLIC(Core);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CORE_P_H_
