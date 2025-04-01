/*
 * Copyright (c) 2010-2025 Belledonne Communications SARL.
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

#ifndef _L_CORE_H_
#define _L_CORE_H_

#include <functional>
#include <list>
#include <unordered_map>

#include <mediastreamer2/mssndcard.h>

#include "account/account.h"
#include "c-wrapper/internal/c-sal.h"
#include "call/audio-device/audio-device.h"
#include "call/call-log.h"
#include "conference/conference-id.h"
#ifdef HAVE_ADVANCED_IM
#include "conference/encryption/ekt-info.h"
#endif // HAVE_ADVANCED_IM
#include "event-log/event-log.h"
#include "event/event-publish.h"
#include "friend/friend-list.h"
#include "linphone/enums/c-enums.h"
#include "linphone/types.h"
#include "object/object.h"
#include "sal/event-op.h"

// =============================================================================

L_DECL_C_STRUCT(LinphoneCore);

typedef struct belle_sip_source belle_sip_source_t;

class ServerConferenceTester;

LINPHONE_BEGIN_NAMESPACE

class AbstractChatRoom;
class Account;
class Address;
class AudioDevice;
class AuthInfo;
class Call;
class CallLog;
class CallSession;
class Conference;
class ConferenceId;
class ConferenceIdParams;
class ConferenceInfo;
class Participant;
class CallSessionListener;
class ConferenceParams;
class Event;
class CorePrivate;
class EncryptionEngine;
class ChatMessage;
class ChatMessageReaction;
class ChatRoom;
class Ldap;
class PushNotificationMessage;
class SalMediaDescription;
class ConferenceScheduler;
class SalOp;
class SignalInformation;
class HttpClient;
class RemoteContactDirectory;

class LINPHONE_PUBLIC Core : public Object {
	friend class Account;
	friend class Call;
	friend class CallLog;
	friend class CallSession;
	friend class ChatMessage;
	friend class ChatMessagePrivate;
	friend class ChatMessageReaction;
	friend class ChatRoom;
	friend class Conference;
	friend class ConferenceScheduler;
	friend class SIPConferenceScheduler;
	friend class ClientChatRoom;
	friend class Imdn;
	friend class ServerConferenceEventHandler;
	friend class ServerConference;
	friend class MainDb;
	friend class MainDbEventKey;
	friend class MS2Stream;
	friend class MediaSessionPrivate;
	friend class ClientConference;
	friend class ClientConferenceEventHandler;
	friend class ClientConferenceListEventHandler;
	friend class ServerChatRoom;
	friend class CallSessionPrivate;
	friend class ToneManager;
	friend class EventLog;

	friend class ::ServerConferenceTester;

public:
	L_OVERRIDE_SHARED_FROM_THIS(Core);

	enum class ETagStatus { Error, None, AddOrUpdateETag };

	static const std::string limeSpec;

	virtual ~Core();

	// Return a new Core instance. Entry point of Linphone.
	static std::shared_ptr<Core> create(LinphoneCore *cCore);

	static std::shared_ptr<const Address>
	getConferenceFactoryAddress(const std::shared_ptr<Core> &core, const std::shared_ptr<const Address> &localAddress);
	static std::shared_ptr<const Address> getConferenceFactoryAddress(const std::shared_ptr<Core> &core,
	                                                                  const std::shared_ptr<Account> &account);
	static std::shared_ptr<const Address>
	getAudioVideoConferenceFactoryAddress(const std::shared_ptr<Core> &core,
	                                      const std::shared_ptr<const Address> &localAddress);
	static std::shared_ptr<const Address>
	getAudioVideoConferenceFactoryAddress(const std::shared_ptr<Core> &core, const std::shared_ptr<Account> &account);

	// ---------------------------------------------------------------------------
	// Application lifecycle.
	// ---------------------------------------------------------------------------

	void enterBackground();
	void enterForeground();
	bool isInBackground() const;

	// ---------------------------------------------------------------------------
	// C-Core.
	// ---------------------------------------------------------------------------

	// TODO: Remove me later.
	LinphoneCore *getCCore() const;

	// ---------------------------------------------------------------------------
	// Call.
	// ---------------------------------------------------------------------------

	bool areSoundResourcesLocked() const;
	std::shared_ptr<Call> getCallByRemoteAddress(const std::shared_ptr<const Address> &addr) const;
	std::shared_ptr<Call> getCallByCallId(const std::string &callId) const;
	const std::list<std::shared_ptr<Call>> &getCalls() const;
	unsigned int getCallCount() const;
	std::shared_ptr<Call> getCurrentCall() const;
	LinphoneStatus pauseAllCalls();
	void soundcardActivateAudioSession(bool active);
	void soundcardConfigureAudioSession();
	void soundcardEnableCallkit(bool enabled);
	void soundcardAudioRouteChanged();
	LinphoneStatus terminateAllCalls();

	// ---------------------------------------------------------------------------
	// Conference Call Event.
	// ---------------------------------------------------------------------------

	void reportConferenceCallEvent(EventLog::Type type,
	                               std::shared_ptr<CallLog> &callLog,
	                               std::shared_ptr<ConferenceInfo> confInfo);
	void reportEarlyCallFailed(LinphoneCallDir dir,
	                           const std::shared_ptr<Address> &from,
	                           const std::shared_ptr<Address> &to,
	                           LinphoneErrorInfo *ei,
	                           const std::string callId);

	// ---------------------------------------------------------------------------
	// ChatRoom.
	// ---------------------------------------------------------------------------

	std::list<std::shared_ptr<AbstractChatRoom>> getRawChatRoomList(bool includeBasic = true,
	                                                                bool includeConference = true) const;
	std::list<std::shared_ptr<AbstractChatRoom>> &getChatRooms() const;
	const bctbx_list_t *getChatRoomsCList() const;

	std::shared_ptr<AbstractChatRoom> findChatRoom(const ConferenceId &conferenceId, bool logIfNotFound = true) const;
	std::list<std::shared_ptr<AbstractChatRoom>> findChatRooms(const std::shared_ptr<Address> &peerAddress) const;

	std::shared_ptr<AbstractChatRoom> createClientChatRoom(const std::string &subject, bool fallback = true);
	std::shared_ptr<AbstractChatRoom> createClientChatRoom(const std::string &subject,
	                                                       LinphoneChatRoomCapabilitiesMask capabilities,
	                                                       bool fallback = true);
	std::shared_ptr<AbstractChatRoom> createClientChatRoom(const std::string &subject,
	                                                       const std::shared_ptr<Address> *localAddress,
	                                                       LinphoneChatRoomCapabilitiesMask capabilities,
	                                                       bool fallback = true);

	std::shared_ptr<AbstractChatRoom> getOrCreateBasicChatRoom(const ConferenceId &conferenceId);

	std::shared_ptr<AbstractChatRoom> getOrCreateBasicChatRoom(const std::shared_ptr<const Address> &localAddress,
	                                                           const std::shared_ptr<const Address> &peerAddress);

	std::shared_ptr<AbstractChatRoom> getOrCreateBasicChatRoomFromUri(const std::string &localAddressUri,
	                                                                  const std::string &peerAddressUri);

	static void deleteChatRoom(const std::shared_ptr<AbstractChatRoom> &chatRoom);

	static const std::string ephemeralVersionAsString();
	static const std::string groupChatVersionAsString();
	static const std::string conferenceVersionAsString();

	LinphoneReason onSipMessageReceived(SalOp *op, const SalMessage *sal_msg);
	LinphoneReason
	handleChatMessagesAggregation(std::shared_ptr<AbstractChatRoom> chatRoom, SalOp *op, const SalMessage *sal_msg);

	void setImdnToEverybodyThreshold(const int threshold);
	int getImdnToEverybodyThreshold() const;

	void enableEmptyChatroomsDeletion(const bool enable);
	bool emptyChatroomsDeletionEnabled() const;

	unsigned int getRemainingUploadFileCount() const;
	void incrementRemainingUploadFileCount();
	void decrementRemainingUploadFileCount();

	unsigned int getRemainingDownloadFileCount() const;
	void incrementRemainingDownloadFileCount();
	void decrementRemainingDownloadFileCount();

	// ---------------------------------------------------------------------------
	// Conference.
	// ---------------------------------------------------------------------------

	ConferenceIdParams createConferenceIdParams() const;
	void setConferenceCleanupPeriod(long seconds);
	long getConferenceCleanupPeriod() const;
	void setConferenceAvailabilityBeforeStart(long seconds);
	long getConferenceAvailabilityBeforeStart() const;
	void setConferenceExpirePeriod(long seconds);
	long getConferenceExpirePeriod() const;
	void insertConference(const std::shared_ptr<Conference> conference);
	void invalidateAccountInConferencesAndChatRooms(const std::shared_ptr<Account> &account);
	std::shared_ptr<Conference> findConference(const std::shared_ptr<const CallSession> &session,
	                                           bool logIfNotFound = true) const;
	std::shared_ptr<Conference> findConference(const ConferenceId &conferenceId, bool logIfNotFound = true) const;
	void deleteConference(const std::shared_ptr<const Conference> &conference);
	void deleteConference(const ConferenceId &conferenceId);
	std::shared_ptr<Conference> searchConference(const std::shared_ptr<ConferenceParams> &params,
	                                             const std::shared_ptr<const Address> &localAddress,
	                                             const std::shared_ptr<const Address> &remoteAddress,
	                                             const std::list<std::shared_ptr<Address>> &participants) const;
	std::shared_ptr<Conference> searchConference(const std::shared_ptr<const Address> &conferenceAddress) const;
	std::shared_ptr<Conference> searchConference(const std::string identifier) const;

	// ---------------------------------------------------------------------------
	// Paths.
	// ---------------------------------------------------------------------------

	std::string getDataPath() const;
	std::string getConfigPath() const;
	std::string getDownloadPath() const;

	// ---------------------------------------------------------------------------
	// EncryptionEngine.
	// ---------------------------------------------------------------------------

	EncryptionEngine *getEncryptionEngine() const;
	void setEncryptionEngine(EncryptionEngine *imee);
	void enableLimeX3dh(bool enable);
	void setX3dhServerUrl(const std::string &url);
	std::string getX3dhServerUrl() const;
	bool limeX3dhEnabled() const;
	bool limeX3dhAvailable() const;
	std::string getX3dhDbPath() const;

	// ---------------------------------------------------------------------------
	// Specs.
	// ---------------------------------------------------------------------------

	void addSpec(const std::string &spec);
	void addSpec(const std::string &specName, const std::string &specVersion);
	bool hasSpec(const std::string &spec) const;
	void removeSpec(const std::string &spec);
	void setSpecs(const std::string &specs);
	void setSpecs(const std::map<std::string, std::string> &specsMap);
	void setSpecs(const std::list<std::string> &specsList);
	std::string getSpecs() const;
	const std::map<std::string, std::string> &getSpecsMap() const;
	const std::list<std::string> getSpecsList() const;
	static std::pair<std::string, std::string> getSpecNameVersion(const std::string &spec);

	// ---------------------------------------------------------------------------
	// Friends.
	// ---------------------------------------------------------------------------

	void enableFriendListSubscription(bool enable);
	bool isFriendListSubscriptionEnabled() const;

	// ---------------------------------------------------------------------------
	// Audio devices.
	// ---------------------------------------------------------------------------

	std::shared_ptr<AudioDevice> findAudioDeviceMatchingMsSoundCard(MSSndCard *soundCard) const;
	std::list<std::shared_ptr<AudioDevice>> getAudioDevices() const;
	std::list<std::shared_ptr<AudioDevice>> getExtendedAudioDevices() const;

	void setInputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice);
	void setOutputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice);
	void setOutputAudioDeviceBySndCard(MSSndCard *card);
	void setInputAudioDeviceBySndCard(MSSndCard *card);
	std::shared_ptr<AudioDevice> getInputAudioDevice() const;
	std::shared_ptr<AudioDevice> getOutputAudioDevice() const;

	void setDefaultInputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice);
	void setDefaultOutputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice);
	std::shared_ptr<AudioDevice> getDefaultInputAudioDevice() const;
	std::shared_ptr<AudioDevice> getDefaultOutputAudioDevice() const;

	// ---------------------------------------------------------------------------
	// Misc.
	// ---------------------------------------------------------------------------

	void pushNotificationReceived(const std::string &callId, const std::string &payload, bool isCoreStarting);
	void healNetworkConnections();

	int getUnreadChatMessageCount() const;
	int getUnreadChatMessageCount(const std::shared_ptr<const Address> &localAddress) const;
	int getUnreadChatMessageCountFromActiveLocals() const;
	std::shared_ptr<PushNotificationMessage> getPushNotificationMessage(const std::string &callId) const;
	std::shared_ptr<ChatRoom> getPushNotificationChatRoom(const std::string &chatRoomAddr) const;
	std::shared_ptr<ChatMessage> findChatMessageFromCallId(const std::string &callId) const;

	void handleIncomingMessageWaitingIndication(std::shared_ptr<Event> event, const Content *content);

	const std::list<std::shared_ptr<RemoteContactDirectory>> &getRemoteContactDirectories();

	void addRemoteContactDirectory(std::shared_ptr<RemoteContactDirectory> remoteContactDirectory);
	void removeRemoteContactDirectory(std::shared_ptr<RemoteContactDirectory> remoteContactDirectory);

	std::shared_ptr<Address> interpretUrl(const std::string &url, bool chatOrCallUse) const;

	// Execute specified lambda later in main loop. This method can be used from any thread to execute something later
	// on main thread.
	void doLater(const std::function<void()> &something);
	// Execure specified lambda now if this method is called on the same thread as linphone_core_iterate(), otherwise do
	// the same as doLater() above.
	void performOnIterateThread(const std::function<void()> &something);

	/*
	 * Run supplied std::function as a timer. It should return true if repeated, false otherwise.
	 * The returned belle_sip_source_t must be destroyed with Core::destroyTimer().
	 */
	belle_sip_source_t *
	createTimer(const std::function<bool()> &something, unsigned int millisecond, const std::string &name);
	/* Stop (ie cancel) and destroy a timer created by createTimer()*/

	void destroyTimer(belle_sip_source_t *timer);

	void onStopAsyncBackgroundTaskStarted(); /* Using background task to ensure stop core async ended */
	void onStopAsyncBackgroundTaskStopped();
	const std::list<LinphoneMediaEncryption> getSupportedMediaEncryptions() const;

	std::shared_ptr<CallSession> createOrUpdateConferenceOnServer(const std::shared_ptr<ConferenceParams> &confParams,
	                                                              const std::list<Address> &participants,
	                                                              const std::shared_ptr<Address> &confAddr,
	                                                              std::shared_ptr<CallSessionListener> listener);

	void removeConferencePendingCreation(const std::shared_ptr<Conference> &conference);
	void addConferencePendingCreation(const std::shared_ptr<Conference> &conference);

	void removeConferenceScheduler(const std::shared_ptr<ConferenceScheduler> &scheduler);
	void addConferenceScheduler(const std::shared_ptr<ConferenceScheduler> &scheduler);

	bool canAggregateChatMessages() const;
	bool isCurrentlyAggregatingChatMessages() const;
	// ---------------------------------------------------------------------------
	// Signal informations
	// ---------------------------------------------------------------------------
	void setSignalInformation(std::shared_ptr<SignalInformation> signalInformation);
	std::shared_ptr<SignalInformation> getSignalInformation();

	const std::list<std::string> &getPluginList() const;
	bool isPluginLoaded(const std::string name) const;

	// ---------------------------------------------------------------------------
	// Publish.
	// ---------------------------------------------------------------------------

	void addOrUpdatePublishByEtag(SalPublishOp *op, std::shared_ptr<LinphonePrivate::EventPublish>);
	Core::ETagStatus eTagHandler(SalPublishOp *op, const SalBodyHandler *body);

	void notifyPublishStateChangedToAccount(const std::shared_ptr<Event> event, LinphonePublishState state);
	int sendPublish(LinphonePresenceModel *presence);

	void setLabel(const std::string &label);
	const std::string &getLabel() const;

	void setVideoCodecPriorityPolicy(LinphoneCodecPriorityPolicy policy);
	LinphoneCodecPriorityPolicy getVideoCodecPriorityPolicy() const;

	// ---------------------------------------------------------------------------
	// XML Parsing/Composing.
	// ---------------------------------------------------------------------------
#ifdef HAVE_ADVANCED_IM
	std::shared_ptr<EktInfo> createEktInfoFromXml(const std::string &xmlBody) const;
	std::string createXmlFromEktInfo(const std::shared_ptr<const EktInfo> &ei,
	                                 const std::shared_ptr<const Account> &account) const;
#endif // HAVE_ADVANCED_IM

	// ---------------------------------------------------------------------------
	// Account
	// ---------------------------------------------------------------------------
	void setAccountDeletionTimeout(unsigned int seconds);
	unsigned int getAccountDeletionTimeout() const;
	void setDefaultAccount(const std::shared_ptr<Account> &account);
	const std::shared_ptr<Account> &getDefaultAccount() const;
	void setDefaultAccountIndex(int index);
	int getDefaultAccountIndex() const;
	std::shared_ptr<Account> getAccountByIdKey(const std::string idKey) const;
	LinphoneStatus addAccount(std::shared_ptr<Account> account);
	void removeAccount(std::shared_ptr<Account> account);
	void removeDeletedAccount(const std::shared_ptr<Account> &account);
	void removeDependentAccount(const std::shared_ptr<Account> &account);
	void clearAccounts();
	void resetAccounts();
	const std::list<std::shared_ptr<Account>> &getDeletedAccounts() const;
	const bctbx_list_t *getDeletedAccountsCList() const;
	const std::list<std::shared_ptr<Account>> &getAccounts() const;
	const bctbx_list_t *getAccountsCList() const;
	std::shared_ptr<Account> lookupKnownAccount(const std::shared_ptr<const Address> uri, bool fallbackToDefault) const;
	std::shared_ptr<Account> lookupKnownAccount(const Address &uri, bool fallbackToDefault) const;
	std::shared_ptr<Account> findAccountByIdentityAddress(const std::shared_ptr<const Address> identity) const;
	std::shared_ptr<Account> findAccountByUsername(const std::string &username) const;
	void releaseAccounts();
	const bctbx_list_t *getProxyConfigList() const;

	// ---------------------------------------------------------------------------
	// AuthInfos
	// ---------------------------------------------------------------------------
	bool refreshTokens(const std::shared_ptr<AuthInfo> &ai);

	// ---------------------------------------------------------------------------
	// HTTP services
	// ---------------------------------------------------------------------------
	HttpClient &getHttpClient();
	/* Only needed because the HttpClient has a dependency on the Sal object (belle_sip_stack_t),
	 * so we have to destroy the HttpClient before the belle_sip_stack_t is destroyed.
	 * This is to be removed when C Core and C++ Core are unified */
	void stopHttpClient();

	// ---------------------------------------------------------------------------
	// Friend Lists
	// ---------------------------------------------------------------------------

	void addFriendList(const std::shared_ptr<FriendList> &list);
	void removeFriendList(const std::shared_ptr<FriendList> &list);
	void clearFriendLists();
	const std::list<std::shared_ptr<FriendList>> &getFriendLists() const;

	// ---------------------------------------------------------------------------
	// EKT plugin
	// ---------------------------------------------------------------------------

	bool isEktPluginLoaded() const;
	void setEktPluginLoaded(bool ektPluginLoaded);

private:
	Core();
	void updateChatRoomList() const;

	bool deleteEmptyChatrooms = true;
	int mImdnToEverybodyThreshold = 5;
	std::shared_ptr<SignalInformation> mSignalInformation = nullptr;
	const ConferenceId prepareConfereceIdForSearch(const ConferenceId &conferenceId) const;
	void clearProxyConfigList() const;

	std::shared_ptr<Account> guessLocalAccountFromMalformedMessage(const std::shared_ptr<Address> &localAddress,
	                                                               const std::shared_ptr<Address> &peerAddress);

	std::list<std::string> plugins;
#if defined(_WIN32) && !defined(_WIN32_WCE)
	std::list<HINSTANCE> loadedPlugins;
#elif defined(HAVE_DLOPEN)
	std::list<void *> loadedPlugins;
#endif

	void initPlugins();
	void uninitPlugins();
	int loadPlugins(const std::string &dir);
	bool_t dlopenPlugin(const std::string &plugin_path, const std::string plugin_name);

	std::list<std::shared_ptr<Conference>> mConferencesPendingCreation;
	std::list<std::shared_ptr<ConferenceScheduler>> mSipConferenceSchedulers;
	std::map<std::string, std::shared_ptr<LinphonePrivate::EventPublish>> mPublishByEtag;

	mutable ListHolder<AbstractChatRoom> mChatRooms;
	mutable ListHolder<Account> mAccounts;
	mutable ListHolder<Account> mDeletedAccounts;
	std::shared_ptr<Account> mDefaultAccount;

	unsigned int mRemainingDownloadFileCount = 0;
	unsigned int mRemainingUploadFileCount = 0;
	unsigned int mAccountDeletionTimeout = 32;

	mutable bctbx_list_t *mCachedProxyConfigs = NULL;

	bool mEktPluginLoaded = false;

	L_DECLARE_PRIVATE(Core);
	L_DISABLE_COPY(Core);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CORE_H_
