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

#ifndef _L_CORE_H_
#define _L_CORE_H_

#include <list>
#include <functional>
#include <unordered_map>

#include "object/object.h"

#include "conference/conference-id.h"
#include "event-log/event-log.h"
#include "linphone/types.h"
#include "call/audio-device/audio-device.h"
#include "call/call-log.h"

// =============================================================================

L_DECL_C_STRUCT(LinphoneCore);

typedef struct belle_sip_source belle_sip_source_t;

namespace LinphoneTest {
	class LocalConferenceTester;
}

LINPHONE_BEGIN_NAMESPACE

class AbstractChatRoom;
class Address;
class Call;
class CallSession;
class ConferenceId;
class Participant;
class ConferenceParams;
class CorePrivate;
class IdentityAddress;
class EncryptionEngine;
class ChatMessage;
class ChatRoom;
class Ldap;
class PushNotificationMessage;
class SalMediaDescription;
class ConferenceScheduler;
class SalOp;

namespace MediaConference {
	class LocalConference;
	class RemoteConference;
	class Conference;
}

class LINPHONE_PUBLIC Core : public Object {
	friend class Account;
	friend class BasicToClientGroupChatRoom;
	friend class BasicToClientGroupChatRoomPrivate;
	friend class Call;
	friend class CallSession;
	friend class ChatMessage;
	friend class ChatMessagePrivate;
	friend class ChatRoom;
	friend class ChatRoomPrivate;
	friend class Conference;
	friend class ClientGroupChatRoom;
	friend class ClientGroupChatRoomPrivate;
	friend class ClientGroupToBasicChatRoomPrivate;
	friend class Imdn;
	friend class LocalConferenceEventHandler;
	friend class LocalConference;
	friend class MainDb;
	friend class MainDbEventKey;
	friend class MS2Stream;
	friend class MediaSessionPrivate;
	friend class RemoteConferenceEventHandler;
	friend class RemoteConferenceListEventHandler;
	friend class ServerGroupChatRoom;
	friend class ServerGroupChatRoomPrivate;
	friend class CallSessionPrivate;
	friend class ToneManager;
	friend class EventLog;

	friend class MediaConference::Conference;
	friend class MediaConference::LocalConference;
	friend class MediaConference::RemoteConference;
	friend class ConferenceScheduler;

	friend class LinphoneTest::LocalConferenceTester;
public:
	L_OVERRIDE_SHARED_FROM_THIS(Core);

	~Core ();

	// Return a new Core instance. Entry point of Linphone.
	static std::shared_ptr<Core> create (LinphoneCore *cCore);

	static std::string getConferenceFactoryUri(const std::shared_ptr<Core> &core, const IdentityAddress &localAddress);
	static std::string getConferenceFactoryUri(const std::shared_ptr<Core> &core, const LinphoneAccount * account);
	static LinphoneAddress* getAudioVideoConferenceFactoryAddress(const std::shared_ptr<Core> &core, const IdentityAddress &localAddress);
	static LinphoneAddress* getAudioVideoConferenceFactoryAddress(const std::shared_ptr<Core> &core, const LinphoneAccount * account);

	// ---------------------------------------------------------------------------
	// Application lifecycle.
	// ---------------------------------------------------------------------------

	void enterBackground ();
	void enterForeground ();
	bool isInBackground () const;

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
	std::shared_ptr<Call> getCallByCallId (const std::string &callId) const;
	const std::list<std::shared_ptr<Call>> &getCalls () const;
	unsigned int getCallCount () const;
	std::shared_ptr<Call> getCurrentCall () const;
	LinphoneStatus pauseAllCalls ();
	void soundcardActivateAudioSession (bool active);
	void soundcardConfigureAudioSession ();
	void soundcardEnableCallkit (bool enabled);
	void soundcardAudioRouteChanged ();
	LinphoneStatus terminateAllCalls ();

	// ---------------------------------------------------------------------------
	// Conference Call Event.
	// ---------------------------------------------------------------------------

	void reportConferenceCallEvent (EventLog::Type type, std::shared_ptr<CallLog> &callLog, std::shared_ptr<ConferenceInfo> confInfo);
	void reportEarlyCallFailed (LinphoneCallDir dir, LinphoneAddress *from, LinphoneAddress *to, LinphoneErrorInfo *ei, const std::string callId);

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
		bool conferenceOnly,
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

	std::shared_ptr<AbstractChatRoom> getOrCreateBasicChatRoom (const ConferenceId &conferenceId);

	std::shared_ptr<AbstractChatRoom> getOrCreateBasicChatRoom (const IdentityAddress &localAddress, const IdentityAddress &peerAddress);

	std::shared_ptr<AbstractChatRoom> getOrCreateBasicChatRoomFromUri (const std::string& localAddressUri, const std::string &peerAddressUri);

	static void deleteChatRoom (const std::shared_ptr<const AbstractChatRoom> &chatRoom);

	static const std::string ephemeralVersionAsString();
	static const std::string groupChatVersionAsString();
	static const std::string conferenceVersionAsString();

	LinphoneReason onSipMessageReceived(SalOp *op, const SalMessage *sal_msg);
	LinphoneReason handleChatMessagesAggregation(std::shared_ptr<AbstractChatRoom> chatRoom, SalOp *op, const SalMessage *sal_msg);

	void enableEmptyChatroomsDeletion(const bool enable);
	bool emptyChatroomsDeletionEnabled() const;

	// ---------------------------------------------------------------------------
	// Audio Video Conference.
	// ---------------------------------------------------------------------------

	void insertAudioVideoConference (const std::shared_ptr<MediaConference::Conference> &audioVideoConference);
	std::shared_ptr<MediaConference::Conference> findAudioVideoConference (const ConferenceId &conferenceId, bool logIfNotFound = true) const;
	void deleteAudioVideoConference(const std::shared_ptr<const MediaConference::Conference> &audioVideoConference);
	std::shared_ptr<MediaConference::Conference> searchAudioVideoConference(const std::shared_ptr<ConferenceParams> &params, const ConferenceAddress &localAddress, const ConferenceAddress &remoteAddress, const std::list<IdentityAddress> &participants) const;
	std::shared_ptr<MediaConference::Conference> searchAudioVideoConference(const ConferenceAddress &conferenceAddress) const;

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
	std::string getX3dhDbPath () const;

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

	void pushNotificationReceived (const std::string& callId, const std::string& payload, bool isCoreStarting);
	int getUnreadChatMessageCount () const;
	int getUnreadChatMessageCount (const IdentityAddress &localAddress) const;
	int getUnreadChatMessageCountFromActiveLocals () const;
	std::shared_ptr<PushNotificationMessage> getPushNotificationMessage (const std::string &callId) const;
	std::shared_ptr<ChatRoom> getPushNotificationChatRoom (const std::string &chatRoomAddr) const;
	std::shared_ptr<ChatMessage> findChatMessageFromCallId (const std::string &callId) const;

	// ---------------------------------------------------------------------------
	// Ldap.
	// ---------------------------------------------------------------------------

	const std::list<std::shared_ptr<Ldap>> & getLdapList();
	std::list<std::shared_ptr<Ldap>>::iterator getLdapIterator(int id);
	
	void addLdap(std::shared_ptr<Ldap> ldap);
	void removeLdap(std::shared_ptr<Ldap> ldap);

	Address interpretUrl (const std::string &url, bool chatOrCallUse) const;

	// Execute specified lambda later in main loop. This method can be used from any thread to execute something later on main thread.
	void doLater(const std::function<void ()> &something);
	// Execure specified lambda now if this method is called on the same thread as linphone_core_iterate(), otherwise do the same as doLater() above.
	void performOnIterateThread(const std::function<void ()> &something);

	/*
	 * Run supplied std::function as a timer. It should return true if repeated, false otherwise.
	 * The returned belle_sip_source_t must be unrefed (with belle_sip_object_unref() ).
	 * It may be unrefed before expiration, if this timer never needs to be cancelled.
	 */
	belle_sip_source_t *createTimer(const std::function<bool ()> &something, unsigned int millisecond, const std::string &name);
	/* Stop (ie cancel) and destroy a timer created by createTimer()*/

	void destroyTimer(belle_sip_source_t *timer);

	bool incompatibleSecurity(const std::shared_ptr<SalMediaDescription> &md) const;
	void onStopAsyncBackgroundTaskStarted(); /* Using background task to ensure stop core async ended */
	void onStopAsyncBackgroundTaskStopped();
	const std::list<LinphoneMediaEncryption> getSupportedMediaEncryptions() const;

	std::shared_ptr<CallSession> createConferenceOnServer(const std::shared_ptr<ConferenceParams> &confParams, const Address &localAddr, const std::list<IdentityAddress> &participants);
	std::shared_ptr<CallSession> createOrUpdateConferenceOnServer(const std::shared_ptr<ConferenceParams> &confParams, const Address &localAddr, const std::list<IdentityAddress> &participants, const ConferenceAddress &confAddr);

	bool isCurrentlyAggregatingChatMessages ();
private:
	Core ();

	bool deleteEmptyChatrooms = true;

	std::unordered_map<ConferenceId, std::shared_ptr<MediaConference::Conference>> audioVideoConferenceById;
	const ConferenceId prepareConfereceIdForSearch(const ConferenceId & conferenceId) const;

	L_DECLARE_PRIVATE(Core);
	L_DISABLE_COPY(Core);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CORE_H_
