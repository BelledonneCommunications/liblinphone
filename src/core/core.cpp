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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

#ifdef HAVE_DLOPEN
#include <dlfcn.h>
#endif

#ifndef _WIN32
#include <dirent.h>
#endif // _WIN32

#include <algorithm>
#include <iterator>

#include "bctoolbox/crypto.hh"
#include "bctoolbox/defs.h"
#include "bctoolbox/utils.hh"

#include "json/json.h"

#include "mediastreamer2/mscommon.h"

#ifdef HAVE_ADVANCED_IM
#include <xercesc/util/PlatformUtils.hpp>
#endif

#include "account/mwi/message-waiting-indication.h"
#ifdef HAVE_LIME_X3DH
#include "chat/encryption/lime-x3dh-encryption-engine.h"
#endif // HAVE_LIME_X3DH
#include "chat/encryption/lime-x3dh-server-engine.h"
#include "conference/conference-context.h"
#include "conference/conference-id-params.h"
#include "conference/conference.h"
#include "conference/handlers/client-conference-list-event-handler.h"
#include "conference/handlers/server-conference-list-event-handler.h"
#include "conference/params/media-session-params-p.h"
#include "conference/participant-info.h"
#include "conference/participant.h"
#include "conference/session/call-session-listener.h"
#include "conference/session/media-session-p.h"
#include "conference/session/media-session.h"
#include "conference/session/streams.h"
#include "core/core-listener.h"
#include "core/core-p.h"
#include "event/event.h"
#include "factory/factory.h"
#include "http/http-client.h"
#include "ldap/ldap.h"
#include "linphone/api/c-account-cbs.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/lpconfig.h"
#include "linphone/utils/algorithm.h"
#include "linphone/utils/utils.h"
#include "logger/logger.h"
#include "paths/paths.h"
#include "sal/sal_media_description.h"
#include "search/remote-contact-directory.h"
#include "vcard/carddav-params.h"

#ifdef HAVE_ADVANCED_IM
#include "xml/ekt-linphone-extension.h"
#endif // HAVE_ADVANCED_IM

// TODO: Remove me later.
#include "c-wrapper/c-wrapper.h"
#include "private.h"
#include "utils/payload-type-handler.h"

#define LINPHONE_DB "linphone.db"
#define LINPHONE_CALL_HISTORY_DB "call-history.db"
#define LINPHONE_FRIENDS_DB "friends.db"
#define LINPHONE_ZRTP_SECRETS_DB "zrtp-secrets.db"

#ifndef LINPHONE_PACKAGE_PLUGINS_DIR
#if defined(_WIN32) || defined(_WIN32_WCE)
#ifdef LINPHONE_WINDOWS_DESKTOP
#define LINPHONE_PACKAGE_PLUGINS_DIR "lib\\liblinphone\\plugins\\"
#else
#define LINPHONE_PACKAGE_PLUGINS_DIR "."
#endif
#else
#define LINPHONE_PACKAGE_PLUGINS_DIR "."
#endif
#endif

#ifndef LINPHONE_PLUGINS_EXT
#define LINPHONE_PLUGINS_EXT ".so"
#endif

#define ETAG_SIZE 8

// =============================================================================

using namespace std;
#ifdef HAVE_ADVANCED_IM
using namespace LinphonePrivate::Xsd::PublishLinphoneExtension;
#endif // HAVE_ADVANCED_IM

LINPHONE_BEGIN_NAMESPACE

const Utils::Version CorePrivate::conferenceProtocolVersion(2, 0);
const Utils::Version CorePrivate::groupChatProtocolVersion(1, 2);
const Utils::Version CorePrivate::ephemeralProtocolVersion(1, 1);

const std::string Core::limeSpec("lime");

typedef void (*init_func_t)(LinphoneCore *core);

void CorePrivate::init() {
	L_Q();

	coreStartupTask.start(q->getSharedFromThis(), 60);
	q->initPlugins();

	mainDb.reset(new MainDb(q->getSharedFromThis()));
	getToneManager(); // Forces instanciation of the ToneManager.
#ifdef HAVE_ADVANCED_IM
	clientListEventHandler = makeUnique<ClientConferenceListEventHandler>(q->getSharedFromThis());
	serverListEventHandler = makeUnique<ServerConferenceListEventHandler>(q->getSharedFromThis());
#endif

	LinphoneCore *lc = L_GET_C_BACK_PTR(q);
	if (q->limeX3dhAvailable()) {
		bool limeEnabled = linphone_config_get_bool(lc->config, "lime", "enabled", TRUE);
		if (limeEnabled) {
			q->enableLimeX3dh(true);
		}
	}

	if (linphone_factory_is_database_storage_available(linphone_factory_get()) &&
	    !!linphone_core_database_enabled(lc)) {
		AbstractDb::Backend backend;
		string uri = L_C_TO_STRING(linphone_config_get_string(linphone_core_get_config(lc), "storage", "uri", nullptr));
		if (!uri.empty())
			if (strcmp(linphone_config_get_string(linphone_core_get_config(lc), "storage", "backend", "sqlite3"),
			           "mysql") == 0) {
				backend = MainDb::Mysql;
			} else {
				backend = MainDb::Sqlite3;
				if (uri != "null") uri = Utils::quoteStringIfNotAlready(uri);
			}
		else {
			backend = AbstractDb::Sqlite3;
			string dbPath = Utils::quoteStringIfNotAlready(q->getDataPath() + "/" + LINPHONE_DB);
			lInfo() << "Using [" << dbPath << "] as default database path";
			uri = dbPath;
		}

		if (uri != "null") { // special uri "null" means don't open database. We need this for tests.
			if (backend == MainDb::Mysql && uri.find("charset=") == string::npos) {
				lInfo() << "No charset defined forcing utf8 4 bytes specially for conference subjet storage";
				uri += " charset=utf8mb4";
			}
			if (backend == MainDb::Sqlite3 &&
			    linphone_config_get_int(linphone_core_get_config(lc), "misc", "sqlite3_synchronous", 1) == 0) {
				lInfo() << "Setting sqlite3 synchronous mode to OFF.";
				uri += " synchronous=OFF";
			}
			lInfo() << "Opening linphone database " << uri << " with backend " << backend;
			uri = LinphonePrivate::Utils::localeToUtf8(uri); // `mainDb->connect` take a UTF8 string.
			auto startMs = bctbx_get_cur_time_ms();
			if (!mainDb->connect(backend, uri)) {
				ostringstream os;
				os << "Unable to open linphone database with uri " << uri << " and backend " << backend;
				throw DatabaseConnectionFailure(os.str());
			}
			auto stopMs = bctbx_get_cur_time_ms();
			auto duration = stopMs - startMs;
			if (duration >= 1000) {
				lWarning() << "Opening database took " << duration << " ms !";
			}

			loadChatRooms();
			linphone_core_friends_storage_resync_friends_lists(lc); // Load friends from mainDB if any
		} else lWarning() << "Database explicitely not requested, this Core is built with no database support.";

		// Leave this part to import the legacy call logs to MainDB
		string calHistoryDbPath = L_C_TO_STRING(
		    linphone_config_get_string(linphone_core_get_config(lc), "storage", "call_logs_db_uri", nullptr));
		if (calHistoryDbPath.empty()) calHistoryDbPath = q->getDataPath() + "/" + LINPHONE_CALL_HISTORY_DB;
		if (calHistoryDbPath != "null") {
			lInfo() << "Using [" << calHistoryDbPath << "] as legacy call history database path";
			linphone_core_set_call_logs_database_path(lc, calHistoryDbPath.c_str());
		} else lWarning() << "Call logs database explicitely not requested";

		if (lc->zrtp_secrets_cache == NULL) {
			string zrtpSecretsDbPath = L_C_TO_STRING(
			    linphone_config_get_string(linphone_core_get_config(lc), "storage", "zrtp_secrets_db_uri", nullptr));
			if (zrtpSecretsDbPath.empty()) zrtpSecretsDbPath = q->getDataPath() + "/" + LINPHONE_ZRTP_SECRETS_DB;
			if (zrtpSecretsDbPath != "null") {
				lInfo() << "Using [" << zrtpSecretsDbPath << "] as default zrtp secrets database path";
				linphone_core_set_zrtp_secrets_file(lc, zrtpSecretsDbPath.c_str());
			} else lWarning() << "ZRTP secrets database explicitely not requested";
		}

		// Leave this part to import the legacy friends to MainDB
		if (!lc->friends_db_file) {
			string friendsDbPath = L_C_TO_STRING(
			    linphone_config_get_string(linphone_core_get_config(lc), "storage", "friends_db_uri", nullptr));
			if (friendsDbPath.empty()) friendsDbPath = q->getDataPath() + "/" + LINPHONE_FRIENDS_DB;
			if (friendsDbPath != "null") {
				lInfo() << "Using [" << friendsDbPath << "] as legacy friends database path";
				linphone_core_set_friends_database_path(lc, friendsDbPath.c_str());
			} else lWarning() << "Friends database explicitely not requested";
		}
	} else {
		lInfo() << "The Core was explicitely requested not to use any database";
	}

	createConferenceCleanupTimer();

#ifdef __ANDROID__
	// On Android assume Core has been started in background,
	// otherwise first notifyEnterForeground() will do nothing.
	// If not, ActivityMonitor will tell us quickly.
	isInBackground = true;
#endif
}

bool CorePrivate::listenerAlreadyRegistered(CoreListener *listener) const {
	return (std::find(listeners.cbegin(), listeners.cend(), listener) != listeners.cend());
}

void CorePrivate::registerListener(CoreListener *listener) {
	if (!listener) {
		lError() << __func__ << " : Ignoring attempt to register a null listener";
		return;
	}
	if (listenerAlreadyRegistered(listener)) return;
	listeners.push_back(listener);
}

void CorePrivate::unregisterListener(CoreListener *listener) {
	listeners.remove(listener);
}

void CorePrivate::writeNatPolicyConfigurations() {
	L_Q();
	int index = 0;
	LinphoneCore *lc = getCCore();

	if (!linphone_core_ready(lc)) return;

	LinphoneConfig *config = linphone_core_get_config(lc);

	if (lc->nat_policy) {
		NatPolicy::toCpp(lc->nat_policy)->saveToConfig(config, index);
		++index;
	}
	for (const auto &account : q->getAccounts()) {
		auto natPolicy = account->getAccountParams()->getNatPolicy();
		if (natPolicy) {
			natPolicy->saveToConfig(config, index);
			++index;
		}
	}
	NatPolicy::clearConfigFromIndex(config, index);
}

void Core::onStopAsyncBackgroundTaskStarted() {
	L_D();
	d->stopAsyncEndEnabled = false;

	function<void()> stopAsyncEnd = [d]() {
		if (linphone_core_get_global_state(d->getCCore()) == LinphoneGlobalShutdown) {
			_linphone_core_stop_async_end(d->getCCore());
		}
	};
	function<void()> enableStopAsyncEnd = [d]() {
		lWarning() << "Background task [Stop core async end] is now expiring";
		d->stopAsyncEndEnabled = true;
	};

	d->bgTask.start(getSharedFromThis(), stopAsyncEnd, enableStopAsyncEnd,
	                linphone_config_get_int(linphone_core_get_config(getCCore()), "misc", "max_stop_async_time", 10));
}

void Core::onStopAsyncBackgroundTaskStopped() {
	L_D();
	d->bgTask.stop();
}

// Called by linphone_core_iterate() to check that aynchronous tasks are done.
// It is used to give a chance to end asynchronous tasks during core stop
// or to make sure that asynchronous tasks are finished during an aynchronous core stop.
bool CorePrivate::isShutdownDone() {
	L_Q();

	if (!calls.empty()) {
		return false;
	}

	bctbx_list_t *elem = NULL;
	for (elem = q->getCCore()->friends_lists; elem != NULL; elem = bctbx_list_next(elem)) {
		LinphoneFriendList *list = (LinphoneFriendList *)elem->data;
		if (linphone_friend_list_get_event(list)) return false;
	}

	// Sometimes (bad network for example), backgrounds for chat message (imdn, delivery ...) take too much time.
	// In this case, forece linphonecore to off. Using "stop core async end" background task to do this.
	if (stopAsyncEndEnabled) {
		return true;
	}

	for (const auto &abstractChatRoom : q->getChatRooms()) {
		const auto &chatRoom = dynamic_pointer_cast<ChatRoom>(abstractChatRoom);
		if (chatRoom && (chatRoom->getImdnHandler()->isCurrentlySendingImdnMessages() ||
		                 !chatRoom->getTransientChatMessages().empty())) {
			return false;
		}
	}

	for (const auto &account : q->getAccounts()) {
		if (account->isUnregistering()) return false;
	}

	return true;
}

void CorePrivate::deleteConferenceInfo(const std::shared_ptr<Address> &conferenceAddress) {
#ifdef HAVE_DB_STORAGE
	mainDb->deleteConferenceInfo(conferenceAddress);
#endif // HAVE_DB_STORAGE
	auto chatRoom = searchChatRoom(nullptr, nullptr, conferenceAddress, {});
	if (chatRoom) {
		chatRoom->deleteFromDb();
	}
}

void CorePrivate::createConferenceCleanupTimer() {
	L_Q();

	const auto period = q->getConferenceCleanupPeriod();
	if (period > 0) {
		auto onConferenceCleanup = [this]() -> bool {
			auto it = mConferenceById.begin();
			while (it != mConferenceById.end()) {
				auto [id, conference] = (*it);
				it++;
				const auto conferenceIsExpired = conference->isConferenceExpired();
				const auto conferenceHasNoParticipantDevices = (conference->getParticipantDevices(false).size() == 0);
				if (conferenceIsExpired && conferenceHasNoParticipantDevices) {
					lInfo() << "Automatically terminating " << *conference
					        << " because it is ended and no devices are left";
					const auto conferenceAddress = conference->getConferenceAddress();
					conference->terminate();
				}
			};
			mainDb->cleanupConferenceInfo(ms_time(NULL));
			return BELLE_SIP_CONTINUE;
		};
		mConferenceCleanupTimer =
		    q->createTimer(onConferenceCleanup, static_cast<unsigned int>(period) * 1000, "conference");
	}
}

void CorePrivate::stopConferenceCleanupTimer() {
	L_Q();
	if (mConferenceCleanupTimer) {
		q->destroyTimer(mConferenceCleanupTimer);
		mConferenceCleanupTimer = nullptr;
	}
}

// Called by _linphone_core_stop_async_start() to stop the asynchronous tasks.
// Put here the calls to stop some task with asynchronous process and check in CorePrivate::isShutdownDone() if they
// have finished.
void CorePrivate::shutdown() {
	L_Q();

	auto currentCalls = calls;
	for (auto call : currentCalls) {
		call->terminate();
	}

	enableMessageWaitingIndicationSubscription(false);
	bctbx_list_t *elem = NULL;
	for (elem = q->getCCore()->friends_lists; elem != NULL; elem = bctbx_list_next(elem)) {
		LinphoneFriendList *list = (LinphoneFriendList *)elem->data;
		linphone_friend_list_enable_subscriptions(list, FALSE);
	}
	audioDevices.clear();

	if (toneManager) toneManager->freeAudioResources();

	stopEphemeralMessageTimer();
	ephemeralMessages.clear();

	stopChatMessagesAggregationTimer();
	stopConferenceCleanupTimer();

	for (const auto &chatRoom : q->getChatRooms()) {
		for (auto &chatMessage : chatRoom->getTransientChatMessages()) {
			if (chatMessage->getState() == ChatMessage::State::FileTransferInProgress) {
				// Abort auto download file transfers
				if (chatMessage->getDirection() == ChatMessage::Direction::Incoming) {
					chatMessage->cancelFileTransfer();
				}
			}
		}
	}

	unregisterAccounts();

	static_cast<PlatformHelpers *>(getCCore()->platform_helper)->stopPushService();
	pushReceivedBackgroundTask.stop();
}

void CorePrivate::unregisterAccounts() {
	L_Q();
	LinphoneCore *lc = q->getCCore();
	if (lc->sip_network_state.global_state) {
		for (const auto &account : q->getAccounts()) {
			account->unpublish(); /* to unpublish without changing the stored flag enable_publish */
			auto params = account->getAccountParams();
			auto natPolicy = params->getNatPolicy();
			if (natPolicy) natPolicy->release();

			/* Do not unregister when push notifications are allowed, otherwise this clears tokens from the SIP
			 * server.*/
			if (params->getUnregisterAtStop() && !params->getPushNotificationAllowed() &&
			    !params->getRemotePushNotificationAllowed()) {
				account->unregister(); /* to unregister without changing the stored flag enable_register */
			}
		}
	}
}

// Called by _linphone_core_stop_async_end() just before going to globalStateOff.
// Put here the data that need to be freed before the stop.
void CorePrivate::uninit() {
	L_Q();

	// If we have an encryption engine, destroy it.
	if (imee != nullptr) {
		auto listener = dynamic_cast<CoreListener *>(q->getEncryptionEngine());
		if (listener) unregisterListener(listener);
		imee.reset();
	}

	const list<shared_ptr<AbstractChatRoom>> chatRooms = q->getChatRooms();
	shared_ptr<ChatRoom> cr;
	for (const auto &chatRoom : chatRooms) {
		cr = dynamic_pointer_cast<ChatRoom>(chatRoom);
		if (cr) {
			cr->sendPendingMessages();
			cr->getImdnHandler()->onLinphoneCoreStop();
#ifdef HAVE_ADVANCED_IM
			for (const auto &participant : cr->getParticipants()) {
				for (std::shared_ptr<ParticipantDevice> device : participant->getDevices()) {
					// to make sure no more messages are received after Core:uninit because key components like DB are
					// no longuer available. So it's no more possible to handle any singnaling messages properly.
					if (device->getSession()) device->getSession()->clearListeners();
				}
			}
#endif
			// end all file transfer background tasks before linphonecore off
			for (auto &chatMessage : cr->getTransientChatMessages()) {
				chatMessage->fileUploadEndBackgroundTask();
			}
		}
	}

	mChatRoomsById.clear();

	for (const auto &[id, conference] : mConferenceById) {
		// Terminate audio video conferences just before core is stopped
		lInfo() << "Terminating conference " << *conference << " with id " << id
		        << " because the core is shutting down";
		conference->terminate();
	}
	mConferenceById.clear();
	q->mConferencesPendingCreation.clear();
	q->mSipConferenceSchedulers.clear();

	listeners.clear();
	static_cast<PlatformHelpers *>(getCCore()->platform_helper)->stopPushService();
	pushReceivedBackgroundTask.stop();
	mRemoteContactDirectories.clear();

	q->mPublishByEtag.clear();

#ifdef HAVE_ADVANCED_IM
	clientListEventHandler.reset();
	serverListEventHandler.reset();
#endif

	Address::clearSipAddressesCache();

	// clear encrypted files plain cache directory
	std::string cacheDir(Factory::get()->getCacheDir(nullptr) + "/evfs/");
	bctbx_rmdir(cacheDir.c_str(), TRUE);

	q->uninitPlugins();

	/* The toneManager is kept until destructor, we may need it because of calls ended during linphone_core_destroy().
	 */
}

void CorePrivate::disconnectMainDb() {
	if (mainDb != nullptr) {
		mainDb->disconnect();
	}
}

void CorePrivate::stopStartupBgTask() {
	/* This is tricky. We have to wait one more iterate() cycle to ensure that the REGISTER procedures are started. */
	if (getCCore()->state == LinphoneGlobalOn) {
		doLater([this]() { coreStartupTask.stop(); });
	}
}

// -----------------------------------------------------------------------------

void CorePrivate::notifyGlobalStateChanged(LinphoneGlobalState state) {
	L_Q();
	switch (state) {
		case LinphoneGlobalOn:
			q->doLater([this] { stopStartupBgTask(); });
			break;
		case LinphoneGlobalOff:
		case LinphoneGlobalShutdown:
			coreStartupTask.stop();
			break;
		case LinphoneGlobalStartup:
		case LinphoneGlobalConfiguring:
		case LinphoneGlobalReady:
			/* nothing to do here */
			break;
	}
	auto listenersCopy = listeners; // Allow removal of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onGlobalStateChanged(state);
}

void CorePrivate::notifyNetworkReachable(bool sipNetworkReachable, bool mediaNetworkReachable) {
	auto listenersCopy = listeners; // Allow removal of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onNetworkReachable(sipNetworkReachable, mediaNetworkReachable);
}

void CorePrivate::notifyCallStateChanged(LinphoneCall *call, LinphoneCallState state, const string &message) {
	auto listenersCopy = listeners; // Allow removal of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onCallStateChanged(call, state, message);
}

void CorePrivate::notifyRegistrationStateChanged(std::shared_ptr<Account> account,
                                                 LinphoneRegistrationState state,
                                                 const string &message) {
	auto listenersCopy = listeners; // Allow removal of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onAccountRegistrationStateChanged(account, state, message);
}

void CorePrivate::notifyRegistrationStateChanged(LinphoneProxyConfig *cfg,
                                                 LinphoneRegistrationState state,
                                                 const string &message) {
	auto listenersCopy = listeners; // Allow removal of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onRegistrationStateChanged(cfg, state, message);
}

void CorePrivate::notifyEnteringBackground() {

	auto listenersCopy = listeners; // Allow removal of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onEnteringBackground();
}

void CorePrivate::notifyEnteringForeground() {
	auto listenersCopy = listeners; // Allow removal of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onEnteringForeground();
}

belle_sip_main_loop_t *CorePrivate::getMainLoop() {
	L_Q();
	return q->getCCore()->sal
	           ? belle_sip_stack_get_main_loop(static_cast<belle_sip_stack_t *>(q->getCCore()->sal->getStackImpl()))
	           : nullptr;
}

Sal *CorePrivate::getSal() {
	return getPublic()->getCCore()->sal.get();
}

LinphoneCore *CorePrivate::getCCore() const {
	return getPublic()->getCCore();
}

void CorePrivate::doLater(const std::function<void()> &something) {
	return belle_sip_main_loop_cpp_do_later(getMainLoop(), something);
}

void CorePrivate::enableFriendListsSubscription(bool enable) {
	L_Q();

	LinphoneCore *lc = L_GET_C_BACK_PTR(q);
	bctbx_list_t *elem;
	for (elem = lc->friends_lists; elem != NULL; elem = bctbx_list_next(elem)) {
		LinphoneFriendList *list = (LinphoneFriendList *)elem->data;
		linphone_friend_list_enable_subscriptions(list, enable);
	}
}

void CorePrivate::enableMessageWaitingIndicationSubscription(bool enable) {
	L_Q();

	for (auto account : q->mAccounts.mList) {
		if (enable) account->subscribeToMessageWaitingIndication();
		else account->unsubscribeFromMessageWaitingIndication();
	}
}

CorePrivate::CorePrivate() : authStack(*this) {
}

ToneManager &CorePrivate::getToneManager() {
	if (!toneManager) toneManager = makeUnique<ToneManager>(*getPublic());
	return *toneManager.get();
}

int CorePrivate::ephemeralMessageTimerExpired(void *data, BCTBX_UNUSED(unsigned int revents)) {
	CorePrivate *d = static_cast<CorePrivate *>(data);
	d->stopEphemeralMessageTimer();

	d->handleEphemeralMessages(ms_time(NULL));
	return BELLE_SIP_STOP;
}

void CorePrivate::startEphemeralMessageTimer(time_t expireTime) {
	double time = difftime(expireTime, ::ms_time(NULL));
	unsigned int timeoutValueMs = time > 0 ? (unsigned int)time * 1000 : 10;
	if (!ephemeralTimer) {
		ephemeralTimer = getPublic()->getCCore()->sal->createTimer(ephemeralMessageTimerExpired, this, timeoutValueMs,
		                                                           "ephemeral message handler");
	} else {
		belle_sip_source_set_timeout_int64(ephemeralTimer, (int64_t)timeoutValueMs);
	}
}

void CorePrivate::stopEphemeralMessageTimer() {
	if (ephemeralTimer) {
		auto core = getPublic()->getCCore();
		if (core && core->sal) core->sal->cancelTimer(ephemeralTimer);
		belle_sip_object_unref(ephemeralTimer);
		ephemeralTimer = nullptr;
	}
}

bool CorePrivate::setInputAudioDevice(const shared_ptr<AudioDevice> &audioDevice) {
	if (audioDevice && ((audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Record)) == 0)) {
		lError() << "Audio device [" << audioDevice << "] doesn't have Record capability";
		return false;
	}

	bool applied = false;
	if (static_cast<unsigned int>(calls.size()) > 0) {
		for (const auto &call : calls) {
			call->setInputAudioDevice(audioDevice);
			applied = true;
		}
	}

	for (const auto &[id, conference] : mConferenceById) {
		auto audioControlInterface = conference->getAudioControlInterface();
		if (audioControlInterface) {
			audioControlInterface->setInputDevice(audioDevice);
		}
	}

	return applied;
}

bool CorePrivate::setOutputAudioDevice(const shared_ptr<AudioDevice> &audioDevice) {
	if (audioDevice && ((audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Play)) == 0)) {
		lError() << "Audio device [" << audioDevice << "] doesn't have Play capability";
		return false;
	}

	bool applied = false;
	if (static_cast<unsigned int>(calls.size()) > 0) {
		for (const auto &call : calls) {
			call->setOutputAudioDevice(audioDevice);
			applied = true;
		}
	}

	for (const auto &[id, conference] : mConferenceById) {
		auto audioControlInterface = conference->getAudioControlInterface();
		if (audioControlInterface) {
			audioControlInterface->setOutputDevice(audioDevice);
		}
	}

	return applied;
}

void CorePrivate::updateVideoDevice() {
#ifdef VIDEO_ENABLED
	if (currentCall && currentCall->getState() == CallSession::State::StreamsRunning) {
		auto ms = currentCall->getMediaSession();
		MS2VideoControl *i = ms->getStreamsGroup().lookupMainStreamInterface<MS2VideoControl>(SalVideo);
		// VideoDevice is a camera. If we are sharing screen, the camera change is only for thumbnail.
		if (i) {
			auto vs = i->getVideoStream();
			if (vs && video_stream_local_screen_sharing_enabled(vs)) {
				auto &group = ms->getStreamsGroup();
				int idx = ms->getThumbnailStreamIdx();
				if (idx >= 0) i = dynamic_cast<MS2VideoControl *>(group.getStream(idx));
			}
			if (i) i->parametersChanged();
		}
	}
	if (getCCore()->conf_ctx) {
		/* There is a local conference.*/
		Conference *conf = Conference::toCpp(getCCore()->conf_ctx);
		VideoControlInterface *i = conf->getVideoControlInterface();
		if (i) i->parametersChanged();
	}
#endif
}

int CorePrivate::getCodecPriority(const OrtpPayloadType *pt) const {
	static constexpr int priorityBonusScore = 50;
	static const std::map<string, int> priorityMap = {{"AV1", 50}, {"H265", 40}, {"H264", 30}, {"VP8", 20}};
	auto it = priorityMap.find(pt->mime_type);

	if (it != priorityMap.end()) {
		// lInfo() << pt->mime_type << " has priority " << it->second << " with bonus: " << ((pt->flags &
		// PAYLOAD_TYPE_PRIORITY_BONUS) ? "yes" : "no");
		return it->second + ((pt->flags & PAYLOAD_TYPE_PRIORITY_BONUS) ? priorityBonusScore : 0);
	}
	return 1000; // return highest priority for unknown codecs, for convenience for whose who want to write plugins.
}

void CorePrivate::reorderVideoCodecList() {
	bctbx_list_t *videoCodecList = getCCore()->codecs_conf.video_codecs;
	bctbx_list_t *elem;
	std::list<OrtpPayloadType *> newList;

	for (elem = videoCodecList; elem != nullptr; elem = elem->next) {
		OrtpPayloadType *pt = (OrtpPayloadType *)elem->data;
		if (videoCodecPriorityPolicy == LinphoneCodecPriorityPolicyAuto) {
#if defined(__ANDROID__) || (defined(__APPLE__) && TARGET_OS_IPHONE)
			MSFilterDesc *encoderDesc = ms_factory_get_encoder(getCCore()->factory, pt->mime_type);
			if (encoderDesc && (encoderDesc->flags & MS_FILTER_IS_HW_ACCELERATED)) {
				payload_type_set_flag(pt, PAYLOAD_TYPE_PRIORITY_BONUS);
			} else {
				payload_type_unset_flag(pt, PAYLOAD_TYPE_PRIORITY_BONUS);
			}
#endif
			newList.push_back(pt);
		} else
			payload_type_unset_flag(
			    pt, PAYLOAD_TYPE_PRIORITY_BONUS); // no notion of bonus in LinphoneCodecPriorityPolicyBasic.
	}
	if (videoCodecPriorityPolicy == LinphoneCodecPriorityPolicyAuto) {
		lInfo() << "Sorting video codec list, new list is:";
		newList.sort([this](const OrtpPayloadType *pt1, const OrtpPayloadType *pt2) -> bool {
			return getCodecPriority(pt1) > getCodecPriority(pt2);
		});
		bctbx_list_free(videoCodecList);
		videoCodecList = nullptr;
		for (auto pt : newList) {
			lInfo() << pt->mime_type;
			videoCodecList = bctbx_list_append(videoCodecList, pt);
		}
		getCCore()->codecs_conf.video_codecs = videoCodecList;
	}
}

// =============================================================================

Core::Core() : Object(*new CorePrivate) {
	L_D();
	d->imee.reset();
#ifdef HAVE_ADVANCED_IM
	xercesc::XMLPlatformUtils::Initialize();
#endif
}

Core::~Core() {
	lInfo() << "Destroying core: " << this;
#ifdef HAVE_ADVANCED_IM
	xercesc::XMLPlatformUtils::Terminate();
#endif
	resetAccounts();
}

shared_ptr<Core> Core::create(LinphoneCore *cCore) {
	// Do not use `make_shared` => Private constructor.
	shared_ptr<Core> core = shared_ptr<Core>(new Core);
	L_SET_CPP_PTR_FROM_C_OBJECT(cCore, core);
	return core;
}

// ---------------------------------------------------------------------------
// Application lifecycle.
// ---------------------------------------------------------------------------

void Core::enterBackground() {
	L_D();
	if (d->isInBackground) return;
	lInfo() << "Core::enterBackground()";
	d->isInBackground = true;

#ifdef __ANDROID__
	static_cast<PlatformHelpers *>(L_GET_C_BACK_PTR(this)->platform_helper)->updateNetworkReachability();
#endif
#if TARGET_OS_IPHONE
	LinphoneCore *lc = L_GET_C_BACK_PTR(this);
	/* Stop the dtmf stream in case it was started.*/
	linphone_core_stop_dtmf_stream(lc);
#endif
	if (isFriendListSubscriptionEnabled()) d->enableFriendListsSubscription(false);
	d->enableMessageWaitingIndicationSubscription(false);
	d->notifyEnteringBackground();
}

void Core::enterForeground() {
	L_D();
	if (!d->isInBackground) return;
	lInfo() << "Core::enterForeground()";
	d->isInBackground = false;

#ifdef __ANDROID__
	static_cast<PlatformHelpers *>(L_GET_C_BACK_PTR(this)->platform_helper)->updateNetworkReachability();
#endif

	for (auto account : mAccounts.mList) {
		if (account->getState() == LinphoneRegistrationFailed) {
			lInfo() << "Refreshing failed account registration for "
			        << *account->getAccountParams()->getIdentityAddress();
			account->refreshRegister();
		}
	}
	d->notifyEnteringForeground();
	if (isFriendListSubscriptionEnabled()) d->enableFriendListsSubscription(true);
	d->enableMessageWaitingIndicationSubscription(true);
}

bool Core::isInBackground() const {
	L_D();
	return d->isInBackground;
}

// ---------------------------------------------------------------------------
// C-Core.
// ---------------------------------------------------------------------------

LinphoneCore *Core::getCCore() const {
	return L_GET_C_BACK_PTR(this);
}

// -----------------------------------------------------------------------------
// Paths.
// -----------------------------------------------------------------------------

string Core::getDataPath() const {
	return Paths::getPath(Paths::Data,
	                      static_cast<PlatformHelpers *>(L_GET_C_BACK_PTR(this)->platform_helper)->getPathContext());
}

string Core::getConfigPath() const {
	return Paths::getPath(Paths::Config,
	                      static_cast<PlatformHelpers *>(L_GET_C_BACK_PTR(this)->platform_helper)->getPathContext());
}

string Core::getDownloadPath() const {
#ifdef __ANDROID__
	return getPlatformHelpers(getCCore())->getDownloadPath();
#else
	return Paths::getPath(Paths::Download,
	                      static_cast<PlatformHelpers *>(L_GET_C_BACK_PTR(this)->platform_helper)->getPathContext());
#endif
}

void Core::setEncryptionEngine(EncryptionEngine *imee) {
	L_D();
	CoreListener *listener = dynamic_cast<CoreListener *>(getEncryptionEngine());
	if (listener) {
		d->unregisterListener(listener);
	}
	d->imee.reset(imee);
}

EncryptionEngine *Core::getEncryptionEngine() const {
	L_D();
	const auto &imee = d->imee;
	if (imee) {
		return imee.get();
	}
	return nullptr;
}

void Core::enableLimeX3dh(bool enable) {
	L_D();

	if (!enable) {
		if (d->imee != nullptr) {
			CoreListener *listener = dynamic_cast<CoreListener *>(getEncryptionEngine());
			if (listener) {
				d->unregisterListener(listener);
			}
			d->imee.reset();
		}
		removeSpec(Core::limeSpec);
		return;
	}
	if (limeX3dhEnabled()) return;

	if (d->imee != nullptr) {
		lWarning() << "Enabling LIME X3DH over previous non LIME X3DH encryption engine";
		CoreListener *listener = dynamic_cast<CoreListener *>(getEncryptionEngine());
		if (listener) {
			d->unregisterListener(listener);
		}
		d->imee.reset();
	}

	if (d->imee == nullptr) {
		if (!linphone_core_conference_server_enabled(getCCore())) {
#ifdef HAVE_LIME_X3DH
			LinphoneConfig *lpconfig = linphone_core_get_config(getCCore());
			if (strcmp(linphone_config_get_string(lpconfig, "lime", "x3dh_server_url", ""), "") != 0) {
				lError() << "Setting x3dh_server_url in section lime is no longer supported. Please use setting "
				            "lime_server_url under section lime to set the URL of the LIME server globally or in the "
				            "proxy section of the RC file";
			}
			string dbAccess = getX3dhDbPath();
			lInfo() << "Using [" << dbAccess << "] as lime database path";
			/* Both Lime and MainDb use soci as database engine.
			 * However, the initialisation of backends must be done manually on platforms where soci is statically
			 * linked. Lime does not do it. Doing it twice is risky - soci unloads the previously loaded backend. A
			 * compromise is to have a single point (AbstractChatRoom::registerBackend()) where it is done safely.
			 * AbstractChatRoom::registerBackend() will do the job only once.
			 */
			AbstractDb::registerBackend(AbstractDb::Sqlite3);
			LimeX3dhEncryptionEngine *engine = new LimeX3dhEncryptionEngine(dbAccess, getSharedFromThis());
			if (engine->getEngineType() == EncryptionEngine::EngineType::LimeX3dh) {
				setEncryptionEngine(engine);
				d->registerListener(engine);
				if (!hasSpec(Core::limeSpec)) {
					addSpec(Core::limeSpec);
				}
			} else { // LimeX3DHEncryptionEngine creation failed
				delete engine;
			}
#else
			lWarning() << "Lime X3DH support is not available";
#endif
		} else {
#ifdef HAVE_ADVANCED_IM
			/* Server mode does not need the lime library dependency. */
			LimeX3dhEncryptionServerEngine *engine = new LimeX3dhEncryptionServerEngine(getSharedFromThis());
			setEncryptionEngine(engine);
			d->registerListener(engine);
#endif
		}
	}
}

std::string Core::getX3dhDbPath() const {
	LinphoneConfig *lpconfig = linphone_core_get_config(getCCore());
	string dbAccess = linphone_config_get_string(lpconfig, "lime", "x3dh_db_path", "");
	// If no path is configured (i.e., the returned string is empty),
	// set the database path to the default location within the data directory.
	if (dbAccess.empty()) {
		dbAccess = getDataPath() + "/x3dh.c25519.sqlite3";
	} // If a path is configured, check if it is set to store the database in memory.
	  // The database will be stored in memory if the path is ":memory:".
	else if (dbAccess != ":memory:") {
		// If the configured path is not an absolute path, treat it as a relative path
		// and prepend the data directory path to it.
		char *basename = bctbx_basename(dbAccess.c_str());
		if (strcmp(basename, dbAccess.c_str()) == 0) {
			dbAccess = getDataPath() + dbAccess;
		}
		bctbx_free(basename);
	}
	return dbAccess;
}

void Core::setX3dhServerUrl(const std::string &url) {
	if (!limeX3dhAvailable()) {
		return;
	}
	LinphoneConfig *lpconfig = linphone_core_get_config(getCCore());
	linphone_config_set_string(lpconfig, "lime", "lime_server_url", url.c_str());
}

std::string Core::getX3dhServerUrl() const {
	LinphoneConfig *lpconfig = linphone_core_get_config(getCCore());
	string serverUrl = linphone_config_get_string(lpconfig, "lime", "lime_server_url", "");
	return serverUrl;
}

bool Core::limeX3dhEnabled() const {
#ifdef HAVE_LIME_X3DH
	L_D();
	bool isServer = linphone_core_conference_server_enabled(getCCore());
	if (d->imee && ((!isServer && d->imee->getEngineType() == EncryptionEngine::EngineType::LimeX3dh) ||
	                (isServer && d->imee->getEngineType() == EncryptionEngine::EngineType::LimeX3dhServer)))
		return true;
#endif
	return false;
}

bool Core::limeX3dhAvailable() const {
#ifdef HAVE_LIME_X3DH
	return true;
#else
	return false;
#endif
}

// -----------------------------------------------------------------------------
// Specs.
// -----------------------------------------------------------------------------
void Core::setSpecs(const std::map<std::string, std::string> &specsMap) {
	L_D();
	d->specs = specsMap;
	const string &tmpSpecs = getSpecs();
	LinphoneConfig *lpconfig = linphone_core_get_config(getCCore());
	linphone_config_set_string(lpconfig, "sip", "linphone_specs", tmpSpecs.c_str());
	getCCore()->sal->setContactLinphoneSpecs(tmpSpecs);
}

void Core::setSpecs(const std::list<std::string> &specsList) {
	std::map<std::string, std::string> specsMap;
	for (const auto &spec : specsList) {
		const auto [name, version] = Core::getSpecNameVersion(spec);
		specsMap[name] = version;
	}
	setSpecs(specsMap);
}

std::pair<std::string, std::string> Core::getSpecNameVersion(const std::string &spec) {
	std::string specName;
	std::string specVersion;
	const auto slashPos = spec.find("/");
	if (slashPos == std::string::npos) {
		specName = spec;
	} else {
		specName = spec.substr(0, slashPos);
		specVersion = spec.substr(slashPos + 1, std::string::npos);
	}
	return std::make_pair(specName, specVersion);
}

void Core::addSpec(const std::string &specName, const std::string &specVersion) {
	L_D();
	d->specs[specName] = specVersion;
	setSpecs(d->specs);
}

void Core::addSpec(const std::string &spec) {
	const auto [name, version] = Core::getSpecNameVersion(spec);
	addSpec(name, version);
}

bool Core::hasSpec(const std::string &spec) const {
	L_D();
// https://gcc.gnu.org/bugzilla/show_bug.cgi?format=multiple&id=81767
#if __GNUC__ == 7
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif //  __GNUC__ == 7
	const auto [name, version] = Core::getSpecNameVersion(spec);
#if __GNUC__ == 7
#pragma GCC diagnostic pop
#endif //  __GNUC__ == 7
	const auto specIt = d->specs.find(name);
	return (specIt != d->specs.end());
}

void Core::removeSpec(const std::string &spec) {
	L_D();
// https://gcc.gnu.org/bugzilla/show_bug.cgi?format=multiple&id=81767
#if __GNUC__ == 7
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif //  __GNUC__ == 7
	const auto [name, version] = Core::getSpecNameVersion(spec);
#if __GNUC__ == 7
#pragma GCC diagnostic pop
#endif //  __GNUC__ == 7
	const auto specIt = d->specs.find(name);
	if (specIt != d->specs.end()) {
		d->specs.erase(specIt);
		setSpecs(d->specs);
	}
}

// Used to set specs for linphone_config
void Core::setSpecs(const std::string &pSpecs) {
	L_D();
	if (pSpecs.empty()) {
		d->specs.clear();
		setSpecs(d->specs);
	} else {
		// Assume a list of coma-separated values
		setSpecs(Utils::toList(bctoolbox::Utils::split(pSpecs, ",")));
	}
}

// Initial use of the public API of this function has been deprecated, but will still be kept as utility function for
// setSpecsList()
std::string Core::getSpecs() const {
	const std::list<std::string> specsList = getSpecsList();
	return Utils::join(Utils::toVector(specsList), ",");
}

const std::map<std::string, std::string> &Core::getSpecsMap() const {
	L_D();
	return d->specs;
}

const std::list<std::string> Core::getSpecsList() const {
	const std::map<std::string, std::string> &specsMap = getSpecsMap();

	std::list<std::string> specsList;
	for (const auto &[name, version] : specsMap) {
		std::string specNameVersion;
		specNameVersion += name;
		if (!version.empty()) {
			specNameVersion += "/";
			specNameVersion += version;
		}
		specsList.push_back(specNameVersion);
	}
	return specsList;
}

const std::string Core::conferenceVersionAsString() {
	std::ostringstream os;
	os << CorePrivate::conferenceProtocolVersion;
	return os.str();
}

// ---------------------------------------------------------------------------
// Friends.
// ---------------------------------------------------------------------------

void Core::enableFriendListSubscription(bool enable) {
	L_D();
	linphone_config_set_int(linphone_core_get_config(getCCore()), "net", "friendlist_subscription_enabled",
	                        enable ? 1 : 0);
	d->enableFriendListsSubscription(enable);
}

bool Core::isFriendListSubscriptionEnabled() const {
	return !!linphone_config_get_int(linphone_core_get_config(getCCore()), "net", "friendlist_subscription_enabled", 1);
}

// ---------------------------------------------------------------------------
// Audio devices.
// ---------------------------------------------------------------------------

void CorePrivate::computeAudioDevicesList() {
	audioDevices.clear();

	MSSndCardManager *snd_card_manager = ms_factory_get_snd_card_manager(getCCore()->factory);
	const bctbx_list_t *list = ms_snd_card_manager_get_list(snd_card_manager);

	for (const bctbx_list_t *it = list; it != nullptr; it = bctbx_list_next(it)) {
		MSSndCard *card = static_cast<MSSndCard *>(bctbx_list_get_data(it));
		audioDevices.push_back((new AudioDevice(card))->toSharedPtr());
	}
}

std::shared_ptr<AudioDevice> Core::findAudioDeviceMatchingMsSoundCard(MSSndCard *soundCard) const {
	for (const auto &audioDevice : getExtendedAudioDevices()) {
		if (audioDevice->getSoundCard() == soundCard) {
			return audioDevice;
		}
	}
	return nullptr;
}

list<shared_ptr<AudioDevice>> Core::getAudioDevices() const {
	std::list<shared_ptr<AudioDevice>> lAudioDevices;
	bool micFound = false, speakerFound = false, earpieceFound = false;
	bool bluetoothMicFound = false, bluetoothSpeakerFound = false;
	bool headsetMicFound = false, headsetSpeakerFound = false;
	bool hearingAidFound = false;

	for (const auto &audioDevice : getExtendedAudioDevices()) {
		switch (audioDevice->getType()) {
			case AudioDevice::Type::Microphone:
				if (!micFound) {
					micFound = true;
					lAudioDevices.push_back(audioDevice);
				}
				break;
			case AudioDevice::Type::Earpiece:
				if (!earpieceFound) {
					earpieceFound = true;
					lAudioDevices.push_back(audioDevice);
				}
				break;
			case AudioDevice::Type::Speaker:
				if (!speakerFound) {
					speakerFound = true;
					lAudioDevices.push_back(audioDevice);
				}
				break;
			case AudioDevice::Type::Bluetooth:
				if (!bluetoothMicFound &&
				    (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Record))) {
					lAudioDevices.push_back(audioDevice);
				} else if (!bluetoothSpeakerFound &&
				           (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Play))) {
					lAudioDevices.push_back(audioDevice);
				}

				// Do not allow to be set to false
				// Not setting flags inside if statement in order to handle the case of a bluetooth device that can
				// record and play sound
				if (!bluetoothMicFound)
					bluetoothMicFound =
					    (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Record));
				if (!bluetoothSpeakerFound)
					bluetoothSpeakerFound =
					    (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Play));
				break;
			case AudioDevice::Type::Headphones:
			case AudioDevice::Type::Headset:
				if (!headsetMicFound &&
				    (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Record))) {
					lAudioDevices.push_back(audioDevice);
				} else if (!headsetSpeakerFound &&
				           (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Play))) {
					lAudioDevices.push_back(audioDevice);
				}

				// Do not allow to be set to false
				// Not setting flags inside if statement in order to handle the case of a headset/headphones device that
				// can record and play sound
				if (!headsetMicFound)
					headsetMicFound =
					    (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Record));
				if (!headsetSpeakerFound)
					headsetSpeakerFound =
					    (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Play));
				break;
			case AudioDevice::Type::HearingAid:
				if (!hearingAidFound) {
					hearingAidFound = true;
					lAudioDevices.push_back(audioDevice);
				}
				break;
			default:
				break;
		}
		if (micFound && speakerFound && earpieceFound && bluetoothMicFound && bluetoothSpeakerFound &&
		    headsetMicFound && headsetSpeakerFound && hearingAidFound)
			break;
	}
	return lAudioDevices;
}

list<std::shared_ptr<AudioDevice>> Core::getExtendedAudioDevices() const {
	L_D();
	return d->audioDevices;
}

void Core::setInputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice) {
	L_D();
	if (getCCore()->use_files) {
		lInfo() << "Trying to change input audio device on core while use_files mode is on : do nothing";
		return;
	}
	bool success = d->setInputAudioDevice(audioDevice);

	if (success) {
		linphone_core_notify_audio_device_changed(L_GET_C_BACK_PTR(getSharedFromThis()), audioDevice->toC());
	}
}

void Core::setOutputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice) {
	L_D();
	if (getCCore()->use_files) {
		lInfo() << "Trying to change output audio device on core while use_files mode is on : do nothing";
		return;
	}
	bool success = d->setOutputAudioDevice(audioDevice);

	if (success) {
		linphone_core_notify_audio_device_changed(L_GET_C_BACK_PTR(getSharedFromThis()), audioDevice->toC());
	}
}

std::shared_ptr<AudioDevice> Core::getInputAudioDevice() const {
	// If the core in a local conference, then get the audio device of the audio control interface
	if (getCCore()->conf_ctx) {
		/* There is a local conference.*/
		Conference *conf = Conference::toCpp(getCCore()->conf_ctx);
		AudioControlInterface *i = conf->getAudioControlInterface();
		return i ? i->getInputDevice() : nullptr;
	} else {
		shared_ptr<LinphonePrivate::Call> call = getCurrentCall();
		if (call) {
			return call->getInputAudioDevice();
		}
		for (const auto &call : getCalls()) {
			return call->getInputAudioDevice();
		}
	}

	return nullptr;
}

std::shared_ptr<AudioDevice> Core::getOutputAudioDevice() const {
	// If the core in a local conference, then get the audio device of the audio control interface
	if (getCCore()->conf_ctx) {
		/* There is a local conference.*/
		Conference *conf = Conference::toCpp(getCCore()->conf_ctx);
		AudioControlInterface *i = conf->getAudioControlInterface();
		return i ? i->getOutputDevice() : nullptr;
	} else {
		shared_ptr<LinphonePrivate::Call> call = getCurrentCall();
		if (call) {
			return call->getOutputAudioDevice();
		}
		for (const auto &call : getCalls()) {
			return call->getOutputAudioDevice();
		}
	}

	return nullptr;
}

void Core::setDefaultInputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice) {
	if ((audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Record)) == 0) {
		lError() << "Audio device [" << audioDevice << "] doesn't have Record capability";
		return;
	}
	linphone_core_set_capture_device(getCCore(), audioDevice->getId().c_str());
}

void Core::setDefaultOutputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice) {
	if ((audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Play)) == 0) {
		lError() << "Audio device [" << audioDevice << "] doesn't have Play capability";
		return;
	}
	linphone_core_set_playback_device(getCCore(), audioDevice->getId().c_str());
}

void Core::setOutputAudioDeviceBySndCard(MSSndCard *card) {
	L_D();

	if (card) {
		auto audioDevice = findAudioDeviceMatchingMsSoundCard(card);
		if (audioDevice) {
			lInfo() << "[ " << __func__ << " ] on device: " << audioDevice->getDeviceName();
			d->setOutputAudioDevice(audioDevice);
			return;
		}
	}
	auto defaultAudioDevice = getDefaultOutputAudioDevice();
	if (defaultAudioDevice) {
		lInfo() << "[ " << __func__ << " ] on default device: " << defaultAudioDevice->getDeviceName();
		d->setOutputAudioDevice(defaultAudioDevice);
		return;
	}
	MSSndCardManager *snd_card_manager = ms_factory_get_snd_card_manager(getCCore()->factory);
	MSSndCard *defaultCard = ms_snd_card_manager_get_default_playback_card(snd_card_manager);
	if (defaultCard) {
		auto audioDevice = findAudioDeviceMatchingMsSoundCard(defaultCard);
		if (audioDevice) {
			lInfo() << "[ " << __func__
			        << " ] on device matching playback capture card: " << audioDevice->getDeviceName();
			d->setOutputAudioDevice(audioDevice);
			return;
		}
	} else { // No cards available : remove the device. This will allow to restart it if a new one is detected.
		lInfo() << "[ " << __func__ << " ] remove output device";
		d->setOutputAudioDevice(nullptr);
	}
	if (card) // Having no device when a card is requested is an error
		lError() << "[ " << __func__ << " ] Unable to find suitable output audio device";
}

void Core::setInputAudioDeviceBySndCard(MSSndCard *card) {
	L_D();

	if (card) {
		auto audioDevice = findAudioDeviceMatchingMsSoundCard(card);
		if (audioDevice) {
			lInfo() << "[ " << __func__ << " ] on device: " << audioDevice->getDeviceName();
			d->setInputAudioDevice(audioDevice);
			return;
		}
	}
	auto defaultAudioDevice = getDefaultInputAudioDevice();
	if (defaultAudioDevice) {
		lInfo() << "[ " << __func__ << " ] on default device: " << defaultAudioDevice->getDeviceName();
		d->setInputAudioDevice(defaultAudioDevice);
		return;
	}
	MSSndCardManager *snd_card_manager = ms_factory_get_snd_card_manager(getCCore()->factory);
	MSSndCard *defaultCard = ms_snd_card_manager_get_default_capture_card(snd_card_manager);
	if (defaultCard) {
		auto audioDevice = findAudioDeviceMatchingMsSoundCard(defaultCard);
		if (audioDevice) {
			lInfo() << "[ " << __func__
			        << " ] on device matching default capture card: " << audioDevice->getDeviceName();
			d->setInputAudioDevice(audioDevice);
			return;
		}
	} else { // No cards available : remove the device. This will allow to restart it if a new one is detected.
		lInfo() << "[ " << __func__ << " ] remove input device";
		d->setInputAudioDevice(nullptr);
	}
	if (card) // Having no device when a card is requested is an error
		lError() << "[ " << __func__ << " ] Unable to find suitable input audio device";
}

std::shared_ptr<AudioDevice> Core::getDefaultInputAudioDevice() const {
	if (getCCore()->use_files) return nullptr;
	else {
		MSSndCard *card = getCCore()->sound_conf.capt_sndcard;
		return findAudioDeviceMatchingMsSoundCard(card);
	}
}

std::shared_ptr<AudioDevice> Core::getDefaultOutputAudioDevice() const {
	if (getCCore()->use_files) return nullptr;
	else {
		MSSndCard *card = getCCore()->sound_conf.play_sndcard;
		return findAudioDeviceMatchingMsSoundCard(card);
	}
}

// -----------------------------------------------------------------------------
// Misc.
// -----------------------------------------------------------------------------

/*
 * pushNotificationReceived() is a critical piece of code.
 * When receiving a push notification, we must be absolutely sure that our connections to the SIP servers is up, running
 * and reliable. If not, we must start or restart them.
 */
void Core::pushNotificationReceived(const string &callId, const string &payload, bool isCoreStarting) {
	L_D();
	lInfo() << "Push notification received for Call-ID [" << callId << "]";
	// (Re) Start a background task for 20 seconds to ensure we have time to process the push
	if (d->pushReceivedBackgroundTask.hasStarted()) {
		d->pushReceivedBackgroundTask.restart();
	} else {
		d->pushReceivedBackgroundTask.start(getSharedFromThis(), 20);
	}

	LinphoneCore *lc = getCCore();
	bool found = false;
	bool keepBackgroundTaskUntilTimeout = false;
	if (!callId.empty()) {
		for (const auto &call : d->calls) {
			auto callLog = call->getLog();
			if (callLog) {
				const auto &id = callLog->getCallId();
				if (id == callId) {
					lInfo() << "Call with matching Call-ID found, no need for a background task";
					found = true;
					break;
				}
			}
		}

		auto chatMessage = findChatMessageFromCallId(callId);
		if (chatMessage) {
			lInfo() << "Chat message with matching Call-ID found, no need for a background task";
			found = true;
		}

		if (!found) {
			lInfo() << "Call/Chat message with Call-ID [" << callId << "] not found";
			keepBackgroundTaskUntilTimeout = true;

			d->lastPushReceivedCallId = callId;
			static_cast<PlatformHelpers *>(lc->platform_helper)->startPushService();
		}
	} else {
		lWarning() << "No given Call-ID, can't know if a background task is necessary or not...";
	}

	linphone_core_notify_push_notification_received(lc, payload.c_str());

	if (isCoreStarting) {
		lInfo() << "Core is starting, skipping network tasks that ensures sockets are alive";
		return;
	}
	if (found) {
		lInfo() << "Call-ID was found, skipping network tasks that ensures sockets are alive";
		return;
	}

	healNetworkConnections();

	if (!keepBackgroundTaskUntilTimeout) {
		lInfo() << "Stopping push notification background task if it exists, doesn't seem necessary anymore";
		d->pushReceivedBackgroundTask.stop();
	}
}

void Core::healNetworkConnections() {
	L_D();
	LinphoneCore *lc = getCCore();

#ifdef __ANDROID__
	if (linphone_core_wifi_only_enabled(lc)) {
		// If WiFi only policy enabled, check that we are on a WiFi network, otherwise don't handle the push
		bool_t isWifiOnlyCompliant =
		    static_cast<PlatformHelpers *>(lc->platform_helper)->isActiveNetworkWifiOnlyCompliant();
		if (!isWifiOnlyCompliant) {
			lError() << "Android Platform Helpers says current network isn't compliant with WiFi only policy, aborting "
			            "push notification processing!";
			d->pushReceivedBackgroundTask.stop();
			return;
		}
	}
#endif

	// We can assume network should be reachable when a push notification is received.
	// If the app was put in DOZE mode, internal network reachability will have been disabled and thus may prevent
	// registration.
	linphone_core_set_network_reachable_internal(lc, TRUE);

	const auto accounts = getAccounts();

	/*
	 * The following is a bit hacky. But sometimes 3 lines of code are better than
	 * a heavy refactoring.
	 * pushNotificationReceived() is a critical piece of code where any action to reconnect to the
	 * SIP server must be taken immediately, which, thanks to belle-sip transactions automatically
	 * starts a background task that prevents iOS and Android systems to suspend the process.
	 */
	linphone_core_iterate(lc); // First iterate to handle disconnection errors on sockets
	linphone_core_iterate(lc); // Second iterate required by belle-sip to notify about disconnections
	linphone_core_iterate(lc); // Third iterate required by refresher to restart a connection/registration if needed.

	/*
	 * If if any of the connections is already pending a register retry, the following code will request an immediate
	 * attempt to connect and register.
	 */
	bool sendKeepAlive = false;
	for (const auto &account : accounts) {
		const auto &params = account->getAccountParams();
		if (params->getForceRegisterOnPushNotification()) {
			lInfo() << "Account [" << account
			        << "] is configured to force a REGISTER when a push is received, doing it now";
			account->refreshRegister();
			continue;
		}

		LinphoneRegistrationState state = account->getState();
		if (state == LinphoneRegistrationFailed) {
			lInfo() << "Account [" << account << "] is in failed state, refreshing REGISTER";
			if (params->getRegisterEnabled() && (params->getExpires() > 0)) {
				account->refreshRegister();
			}
		} else if (state == LinphoneRegistrationOk) {
			// Send a keep-alive to ensure the socket isn't broken
			sendKeepAlive = true;
		}
	}
	/* Send a "\r\n" keepalive. If the socket is broken, it will generate an error. */
	if (sendKeepAlive) {
		lInfo() << "Sending keep-alive to ensure sockets aren't broken";
		getCCore()->sal->sendKeepAlive();
		linphone_core_iterate(lc); // Let the socket error be caught.
		linphone_core_iterate(
		    lc); // Let the socket error be notified to the refreshers, to restart a connection if needed.
	}
	/*
	 * Despite all the things done so far, there can still be the case where some connections are "ready" but in fact
	 * stalled, due to crappy firewalls not notifying reset connections. Eliminate them.
	 */
	if (d->calls.empty()) {
		/* We do this only if there is no running call. Indeed, breaking the connection on which the call was made is
		 * problematic, as we are going to loose the local contact of the dialog. As of today, liblinphone doesn't
		 * trigger a reINVITE upon loss of underlying connection. It does this only upon network manager decisions
		 * (linphone_core_set_network_reachable_internal()). This looks an acceptable compromise, as the socket is not
		 * expected to be zombified while keepalives are regularly sent while the app is in foreground.
		 */
		lc->sal->cleanUnreliableConnections();
	}
	linphone_core_iterate(lc); // Let the disconnections be notified to the refreshers.
}

int Core::getUnreadChatMessageCount() const {
	L_D();
	if (d->mainDb && d->mainDb->isInitialized()) {
		return d->mainDb->getUnreadChatMessageCount();
	}
	return -1;
}

int Core::getUnreadChatMessageCount(const std::shared_ptr<const Address> &localAddress) const {
	int count = 0;
	for (const auto &chatRoom : getChatRooms()) {
		if (localAddress->weakEqual(*chatRoom->getLocalAddress())) {
			if (!chatRoom->getIsMuted()) {
				count += chatRoom->getUnreadChatMessageCount();
			}
		}
	}
	return count;
}

int Core::getUnreadChatMessageCountFromActiveLocals() const {
	int count = 0;
	for (const auto &chatRoom : getChatRooms()) {
		for (const auto &account : getAccounts()) {
			auto identityAddress = account->getAccountParams()->getIdentityAddress();
			if (identityAddress->weakEqual(*chatRoom->getLocalAddress())) {
				if (!chatRoom->getIsMuted()) {
					count += chatRoom->getUnreadChatMessageCount();
				}
			}
		}
	}
	return count;
}

std::shared_ptr<PushNotificationMessage> Core::getPushNotificationMessage(const std::string &callId) const {
	std::shared_ptr<PushNotificationMessage> msg =
	    getPlatformHelpers(getCCore())->getSharedCoreHelpers()->getPushNotificationMessage(callId);
	if (linphone_core_get_global_state(getCCore()) == LinphoneGlobalOn &&
	    getPlatformHelpers(getCCore())->getSharedCoreHelpers()->isCoreStopRequired()) {
		lInfo() << "[SHARED] Executor Shared Core is beeing stopped by Main Shared Core";
		linphone_core_stop(getCCore());
#if TARGET_OS_IPHONE
		if (!msg && callId != "dummy_call_id") {
			ms_message("[push] Executor Core for callId[%s] has been stopped but couldn't get the msg in time. Trying "
			           "to get the msg received by the Main Core that just started",
			           callId.c_str());
			auto platformHelpers = LinphonePrivate::createIosPlatformHelpers(getCCore()->cppPtr, NULL);
			msg = platformHelpers->getSharedCoreHelpers()->getPushNotificationMessage(callId);
			delete platformHelpers;
		}
#endif
	}
	return msg;
}

std::shared_ptr<ChatRoom> Core::getPushNotificationChatRoom(const std::string &chatRoomAddr) const {
	std::shared_ptr<ChatRoom> chatRoom =
	    getPlatformHelpers(getCCore())->getSharedCoreHelpers()->getPushNotificationChatRoom(chatRoomAddr);
	return chatRoom;
}

std::shared_ptr<ChatMessage> Core::findChatMessageFromCallId(const std::string &callId) const {
	L_D();
	std::list<std::shared_ptr<ChatMessage>> chatMessages = d->mainDb->findChatMessagesFromCallId(callId);
	return chatMessages.empty() ? nullptr : chatMessages.front();
}

void Core::handleIncomingMessageWaitingIndication(std::shared_ptr<Event> event, const Content *content) {
	shared_ptr<const Address> accountAddr = nullptr;

	if (!content) {
		lWarning() << "MWI NOTIFY without body, doing nothing";
		return;
	}

	// Try to get the account that has subscribed for this event.
	for (auto account : getAccounts()) {
		if (account->getMwiEvent() != event) continue;
		accountAddr = account->getAccountParams()->getIdentityAddress();
	}
	if (!accountAddr) {
		lWarning() << "Received NOTIFY for an unknown MWI subscribe event, maybe an out-of-dialog notify. "
		           << "Will try to find the account for the address contained in the body of the notification.";
	}

	shared_ptr<Mwi::MessageWaitingIndication> mwi = Mwi::MessageWaitingIndication::parse(*content);
	if (mwi) {
		if (!accountAddr) accountAddr = mwi->getAccountAddress();
		auto account = findAccountByIdentityAddress(accountAddr);
		if (account) {
			LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Account, account,
			                                  linphone_account_cbs_get_message_waiting_indication_changed, mwi->toC());
		}
		linphone_core_notify_message_waiting_indication_changed(event->getCore()->getCCore(), event->toC(), mwi->toC());
	} else {
		lWarning() << "Wrongly formatted MWI notification";
	}
}

// -----------------------------------------------------------------------------
// Remote Contact Directories.
// -----------------------------------------------------------------------------

const list<shared_ptr<RemoteContactDirectory>> &Core::getRemoteContactDirectories() {
	return getPrivate()->mRemoteContactDirectories;
}

void Core::addRemoteContactDirectory(shared_ptr<RemoteContactDirectory> remoteContactDirectory) {
	remoteContactDirectory->writeToConfigFile();
	getPrivate()->mRemoteContactDirectories.push_back(remoteContactDirectory);
}

void Core::removeRemoteContactDirectory(shared_ptr<RemoteContactDirectory> remoteContactDirectory) {
	remoteContactDirectory->removeFromConfigFile();
	getPrivate()->mRemoteContactDirectories.remove(remoteContactDirectory);
}

void CorePrivate::reloadRemoteContactDirectories() {
	auto core = getPublic()->getSharedFromThis();
	auto lpConfig = linphone_core_get_config(getCCore());
	bctbx_list_t *bcSections = linphone_config_get_sections_names_list(lpConfig);

	string carddavSection = "carddav_";
	string ldapSection = "ldap_";
	list<shared_ptr<RemoteContactDirectory>> paramsList;
	for (auto itSections = bcSections; itSections; itSections = itSections->next) {
		string section = static_cast<char *>(itSections->data);
		if (section.rfind(carddavSection, 0) == 0) {
			string indexStr = section.substr(carddavSection.size());
			if (!indexStr.empty()) {
				try {
					int index = stoi(indexStr);
					auto params = CardDavParams::create(core, index);
					auto remoteContactDirectory = RemoteContactDirectory::create(params);
					paramsList.push_back(remoteContactDirectory);
				} catch (const invalid_argument &) {
				} catch (const out_of_range &) {
				}
			}
		} else if (section.rfind(ldapSection, 0) == 0) {
			string indexStr = section.substr(ldapSection.size());
			if (!indexStr.empty()) {
				try {
					int index = stoi(indexStr);
					auto params = LdapParams::create(core, index);
					auto remoteContactDirectory = RemoteContactDirectory::create(params);
					paramsList.push_back(remoteContactDirectory);
				} catch (const invalid_argument &) {
				} catch (const out_of_range &) {
				}
			}
		}
	}

	if (bcSections) bctbx_list_free(bcSections);
	mRemoteContactDirectories = paramsList;
}

// -----------------------------------------------------------------------------
// Signal Infomrations
// -----------------------------------------------------------------------------

void Core::setSignalInformation(std::shared_ptr<SignalInformation> signalInformation) {
	mSignalInformation = signalInformation;
}
std::shared_ptr<SignalInformation> Core::getSignalInformation() {
	return mSignalInformation;
}

// -----------------------------------------------------------------------------

std::shared_ptr<Address> Core::interpretUrl(const std::string &url, bool chatOrCallUse) const {
	bool applyPrefix = true;
	if (chatOrCallUse) {
		LinphoneAccount *account = linphone_core_get_default_account(getCCore());
		if (account) {
			const LinphoneAccountParams *params = linphone_account_get_params(account);
			applyPrefix = linphone_account_params_get_use_international_prefix_for_calls_and_chats(params);
		}
	}

	LinphoneAddress *cAddress = linphone_core_interpret_url_2(getCCore(), url.c_str(), applyPrefix);
	if (!cAddress) return nullptr;

	std::shared_ptr<Address> address = Address::getSharedFromThis(cAddress);
	linphone_address_unref(cAddress);

	return address;
}

void Core::doLater(const std::function<void()> &something) {
	getPrivate()->doLater(something);
}

void Core::performOnIterateThread(const std::function<void()> &something) {
	unsigned long currentThreadId = bctbx_thread_self();
	if (currentThreadId == getCCore()->iterate_thread_id) {
		something();
	} else {
		doLater(something);
	}
}

belle_sip_source_t *
Core::createTimer(const std::function<bool()> &something, unsigned int milliseconds, const string &name) {
	const auto mainLoop = getPrivate()->getMainLoop();
	return mainLoop
	           ? belle_sip_main_loop_create_cpp_timeout_2(mainLoop, something, (unsigned)milliseconds, name.c_str())
	           : nullptr;
}

/* Stop and destroy a timer created by createTimer()*/
void Core::destroyTimer(belle_sip_source_t *timer) {
	belle_sip_source_cancel(timer);
	belle_sip_object_unref(timer);
}

void Core::invalidateAccountInConferencesAndChatRooms(const std::shared_ptr<Account> &account) {
	L_D();
	for (const auto &[id, conference] : d->mConferenceById) {
		if (account == conference->getAccount()) {
			conference->invalidateAccount();
		}
	}

	for (const auto &[id, chatRoom] : d->mChatRoomsById) {
		if (account == chatRoom->getAccount()) {
			chatRoom->invalidateAccount();
		}
	}
}

void Core::setConferenceCleanupPeriod(long seconds) {
	L_D();
	auto oldPeriod = getConferenceCleanupPeriod();
	LinphoneConfig *config = linphone_core_get_config(getCCore());
	linphone_config_set_int64(config, "misc", "conference_cleanup_period", seconds);
	lInfo() << "Set conference clean window to " << seconds << " seconds";
	if ((oldPeriod <= 0) && (seconds > 0)) {
		// Create cleanup time if not already done
		d->createConferenceCleanupTimer();
	} else if ((oldPeriod > 0) && (seconds <= 0)) {
		// Stop time if the new period is 0 or lower
		d->stopConferenceCleanupTimer();
	} else {
		// Update timer
		d->stopConferenceCleanupTimer();
		d->createConferenceCleanupTimer();
	}
}

long Core::getConferenceCleanupPeriod() const {
	return (long)linphone_config_get_int64(linphone_core_get_config(getCCore()), "misc", "conference_cleanup_period",
	                                       -1);
}

void Core::setConferenceAvailabilityBeforeStart(long seconds) {
	LinphoneConfig *config = linphone_core_get_config(getCCore());
	linphone_config_set_int64(config, "misc", "conference_available_before_period", seconds);
	lInfo() << "Set conference avaibility before start to " << seconds << " seconds";
}

long Core::getConferenceAvailabilityBeforeStart() const {
	return (long)linphone_config_get_int64(linphone_core_get_config(getCCore()), "misc",
	                                       "conference_available_before_period", -1);
}

void Core::setConferenceExpirePeriod(long seconds) {
	LinphoneConfig *config = linphone_core_get_config(getCCore());
	linphone_config_set_int64(config, "misc", "conference_expire_period", seconds);
	lInfo() << "Set conference expire period to " << seconds << " seconds";
}

long Core::getConferenceExpirePeriod() const {
	// Default to 30 minutes
	return (long)linphone_config_get_int64(linphone_core_get_config(getCCore()), "misc", "conference_expire_period",
	                                       1800);
}

void Core::setAccountDeletionTimeout(unsigned int seconds) {
	if (seconds == 0) {
		lError() << "Unable to disable automatic deletion of accounts";
	}
	mAccountDeletionTimeout = seconds;
}

unsigned int Core::getAccountDeletionTimeout() const {
	return mAccountDeletionTimeout;
}

std::shared_ptr<Conference> Core::findConference(const std::shared_ptr<const CallSession> &session,
                                                 bool logIfNotFound) const {

	L_D();
	for (const auto &[id, conference] : d->mConferenceById) {
		const auto &conferenceSession = conference->getMainSession();
		if (conferenceSession && (session == conferenceSession)) {
			return conference;
		}
	}

	auto op = session->getPrivate()->getOp();
	if (op) {
		shared_ptr<Call> call = getCallByCallId(op->getCallId());
		if (call) {
			return call->getConference();
		}
	}

	if (logIfNotFound) {
		lInfo() << "Unable to find the conference session " << session << " (local address "
		        << *session->getLocalAddress() << " remote address "
		        << (session->getRemoteAddress() ? session->getRemoteAddress()->toString() : "Unknown")
		        << ") belongs to";
	}
	return nullptr;
}

std::shared_ptr<Conference> Core::findConference(const ConferenceId &conferenceId, bool logIfNotFound) const {
	L_D();
	try {
		auto conference = d->mConferenceById.at(conferenceId);
		lInfo() << "Found " << *conference << " in RAM with conference ID " << conferenceId << ".";
		return conference;
	} catch (const out_of_range &) {
		if (logIfNotFound) {
			lInfo() << "Unable to find conference with conference ID " << conferenceId << " in RAM.";
		}
	}
	return nullptr;
}

void Core::insertConference(const shared_ptr<Conference> conference) {
	L_D();

	L_ASSERT(conference);

	const ConferenceId &conferenceId = conference->getConferenceId();
	if (!conferenceId.isValid()) {
		lInfo() << "Attempting to insert " << *conference << " with invalid conference ID " << conferenceId;
		return;
	}

	bool isStartup = (linphone_core_get_global_state(getCCore()) == LinphoneGlobalStartup);
	std::shared_ptr<Conference> conf;
	if (!isStartup) {
		if (conference->getCurrentParams()->chatEnabled()) {
			// Handling of chat room exhume
			const auto &chatRoom = findChatRoom(conferenceId, false);
			if (chatRoom) {
				conf = chatRoom->getConference();
			}
		} else {
			conf = findConference(conferenceId, false);
		}
		// When starting the LinphoneCore, it may happen to have 2 audio video conferences or chat room that have the
		// same conference ID apart from the GRUU which is not taken into the account for the comparison. In such a
		// scenario, it is allowed to replace the pointer towards the audio video conference in the core map. Method
		// addChatRoomToList will take care of setting the right pointer before exiting
		L_ASSERT(conf == nullptr || conf == conference);
	}
	if ((conf == nullptr) || (conf != conference)) {
		lInfo() << "Insert " << *conference << " in RAM with conference ID " << conferenceId << ".";
		d->mConferenceById.insert_or_assign(conferenceId, conference);
	}
}

void Core::deleteConference(const ConferenceId &conferenceId) {
	L_D();
	auto it = d->mConferenceById.find(conferenceId);
	if (it != d->mConferenceById.cend()) {
		lInfo() << "Delete " << *(it->second) << " in RAM with conference ID " << conferenceId << ".";
		d->mConferenceById.erase(it);
	}
}

void Core::deleteConference(const shared_ptr<const Conference> &conference) {
	const ConferenceId &conferenceId = conference->getConferenceId();
	deleteConference(conferenceId);
}

std::shared_ptr<Conference> Core::searchConference(const std::shared_ptr<ConferenceParams> &params,
                                                   const std::shared_ptr<const Address> &localAddress,
                                                   const std::shared_ptr<const Address> &remoteAddress,
                                                   const std::list<std::shared_ptr<Address>> &participants) const {
	L_D();
	ConferenceContext referenceConferenceContext(params, localAddress, remoteAddress, participants);
	const auto it = std::find_if(
	    d->mConferenceById.begin(), d->mConferenceById.end(), [&referenceConferenceContext](const auto &p) {
		    // p is of type std::pair<ConferenceId, std::shared_ptr<Conference>
		    const auto &conference = p.second;
		    const ConferenceId &conferenceId = conference->getConferenceId();
		    ConferenceContext conferenceContext(conference->getCurrentParams(), conferenceId.getLocalAddress(),
		                                        conferenceId.getPeerAddress(), conference->getParticipantAddresses());
		    return (referenceConferenceContext == conferenceContext);
	    });

	std::shared_ptr<Conference> conference;
	if (it != d->mConferenceById.cend()) {
		conference = it->second;
	}

	return conference;
}

std::shared_ptr<Conference> Core::searchConference(const std::string identifier) const {
	auto [localAddress, peerAddress] = ConferenceId::parseIdentifier(identifier);
	if (!localAddress || !localAddress->isValid() || !peerAddress || !peerAddress->isValid()) {
		return nullptr;
	}
	return searchConference(nullptr, localAddress, peerAddress, {});
}

shared_ptr<Conference> Core::searchConference(const std::shared_ptr<const Address> &conferenceAddress) const {
	L_D();

	if (!conferenceAddress || !conferenceAddress->isValid()) return nullptr;
	const auto it = std::find_if(d->mConferenceById.begin(), d->mConferenceById.end(), [&](const auto &p) {
		// p is of type std::pair<ConferenceId, std::shared_ptr<Conference>
		const auto &conference = p.second;
		const auto curConferenceAddress = conference->getConferenceAddress();
		return (*conferenceAddress == *curConferenceAddress);
	});

	shared_ptr<Conference> conference = nullptr;
	if (it != d->mConferenceById.cend()) {
		conference = it->second;
	}

	return conference;
}

void Core::removeConferencePendingCreation(const std::shared_ptr<Conference> &conference) {
	mConferencesPendingCreation.remove(conference);
}

void Core::addConferencePendingCreation(const std::shared_ptr<Conference> &conference) {
	auto it = std::find(mConferencesPendingCreation.begin(), mConferencesPendingCreation.end(), conference);
	if (it == mConferencesPendingCreation.end()) {
		mConferencesPendingCreation.push_back(conference);
	}
}

void Core::removeConferenceScheduler(const std::shared_ptr<ConferenceScheduler> &scheduler) {
	mSipConferenceSchedulers.remove(scheduler);
}

void Core::addConferenceScheduler(const std::shared_ptr<ConferenceScheduler> &scheduler) {
	auto it = std::find(mSipConferenceSchedulers.begin(), mSipConferenceSchedulers.end(), scheduler);
	if (it == mSipConferenceSchedulers.end()) {
		mSipConferenceSchedulers.push_back(scheduler);
	}
}

shared_ptr<CallSession> Core::createOrUpdateConferenceOnServer(const std::shared_ptr<ConferenceParams> &confParams,
                                                               const std::list<Address> &participants,
                                                               const std::shared_ptr<Address> &confAddr,
                                                               std::shared_ptr<CallSessionListener> listener) {
	if (!confParams) {
		lWarning() << "Trying to create or update conference with null parameters";
		return nullptr;
	}

	MediaSessionParams params;
	params.initDefault(getSharedFromThis(), LinphoneCallOutgoing);

	auto account = confParams->getAccount();
	if (!account) {
		account = getDefaultAccount();
	}
	if (!account) {
		lWarning() << "Unable to create or update a conference without an account associated";
		return nullptr;
	}
	params.setAccount(account);

	std::shared_ptr<Address> conferenceFactoryUri;
	bool mediaEnabled = (confParams->audioEnabled() || confParams->videoEnabled());
	if (confAddr) {
		conferenceFactoryUri = confAddr;
	} else {
		std::shared_ptr<const Address> conferenceFactoryUriRef;
		if (mediaEnabled) {
			conferenceFactoryUriRef = Core::getAudioVideoConferenceFactoryAddress(getSharedFromThis(), account);
		} else {
			conferenceFactoryUriRef = account->getAccountParams()->getConferenceFactoryAddress();
		}
		if (!conferenceFactoryUriRef || !conferenceFactoryUriRef->isValid()) {
			lWarning() << "Not creating or updating conference: no conference factory uri for account " << *account;
			return nullptr;
		}
		conferenceFactoryUri = conferenceFactoryUriRef->clone()->toSharedPtr();
		conferenceFactoryUri->setUriParam(Conference::SecurityModeParameter,
		                                  ConferenceParams::getSecurityLevelAttribute(confParams->getSecurityLevel()));
	}

	if (!!linphone_core_get_add_admin_information_to_contact(getCCore())) {
		params.addCustomContactParameter(Conference::AdminParameter, Utils::toString(true));
	}

	if (confParams->chatEnabled()) {
		if (!mediaEnabled) {
			params.addCustomContactParameter(Conference::TextParameter);
		}
		params.addCustomHeader("Require", "recipient-list-invite");
		params.addCustomHeader("One-To-One-Chat-Room", Utils::btos(!confParams->isGroup()));
		params.addCustomHeader("End-To-End-Encrypted", Utils::btos(confParams->getChatParams()->isEncrypted()));
		params.addCustomHeader("Ephemerable", Utils::btos(confParams->getChatParams()->getEphemeralMode() ==
		                                                  AbstractChatRoom::EphemeralMode::AdminManaged));
		params.addCustomHeader("Ephemeral-Life-Time", to_string(confParams->getChatParams()->getEphemeralLifetime()));
	}

	if (!participants.empty()) {
		auto addresses = participants;
		addresses.sort([](const auto &addr1, const auto &addr2) { return addr1 < addr2; });
		addresses.unique([](const auto &addr1, const auto &addr2) { return addr1.weakEqual(addr2); });
		auto resourceList = Content::create();
		resourceList->setBodyFromUtf8(Utils::getResourceLists(addresses));
		resourceList->setContentType(ContentType::ResourceLists);
		resourceList->setContentDisposition(ContentDisposition::RecipientList);
		LinphoneCore *lc = L_GET_C_BACK_PTR(this);
		if (linphone_core_content_encoding_supported(lc, "deflate")) {
			resourceList->setContentEncoding("deflate");
		}
		params.addCustomContent(resourceList);
	}
	params.enableAudio(confParams->audioEnabled());
	params.enableVideo(confParams->videoEnabled());
	params.enableRealtimeText(false);
	params.getPrivate()->setStartTime(confParams->getStartTime());
	params.getPrivate()->setEndTime(confParams->getEndTime());
	params.getPrivate()->setDescription(confParams->getDescription());
	params.getPrivate()->setConferenceCreation(true);
	params.getPrivate()->disableRinging(true);
	params.getPrivate()->enableToneIndications(false);

	const auto &localAddr = account->getAccountParams()->getIdentityAddress();
	auto participant = Participant::create(nullptr, localAddr);
	auto session = participant->createSession(getSharedFromThis(), &params, true);
	session->addListener(listener);
	bool isMediaSession = (dynamic_pointer_cast<MediaSession>(session) != nullptr);

	if (!session) {
		lWarning() << "Cannot create conference with subject [" << confParams->getSubject() << "]";
		return nullptr;
	}

	std::shared_ptr<Address> meCleanedAddress = Address::create(localAddr->getUriWithoutGruu());
	session->configure(LinphoneCallOutgoing, nullptr, nullptr, meCleanedAddress, conferenceFactoryUri);
	const auto destAccount = session->getPrivate()->getDestAccount();
	std::shared_ptr<NatPolicy> natPolicy = nullptr;
	if (destAccount) {
		const auto accountParams = destAccount->getAccountParams();
		natPolicy = accountParams->getNatPolicy();
	}
	if (!natPolicy) {
		natPolicy = NatPolicy::getSharedFromThis(linphone_core_get_nat_policy(getCCore()));
	}
	if (natPolicy && isMediaSession) {
		auto newNatPolicy = natPolicy->clone()->toSharedPtr();
		// remove stun server asynchronous gathering, we don't actually need it and it looses some time.
		newNatPolicy->enableStun(false);
		dynamic_pointer_cast<MediaSession>(session)->setNatPolicy(newNatPolicy);
	}
	session->initiateOutgoing();
	if (!isMediaSession) {
		session->getPrivate()->createOp();
	}
	session->startInvite(nullptr, confParams->getUtf8Subject(), nullptr);
	return session;
}

const std::list<LinphoneMediaEncryption> Core::getSupportedMediaEncryptions() const {
	LinphoneCore *lc = L_GET_C_BACK_PTR(this);
	std::list<LinphoneMediaEncryption> encEnumList;
	const auto encList = linphone_core_get_supported_media_encryptions(lc);
	for (const bctbx_list_t *enc = encList; enc != NULL; enc = enc->next) {
		encEnumList.push_back(static_cast<LinphoneMediaEncryption>(LINPHONE_PTR_TO_INT(bctbx_list_get_data(enc))));
	}
	return encEnumList;
}

std::shared_ptr<const Address>
Core::getAudioVideoConferenceFactoryAddress(const std::shared_ptr<Core> &core,
                                            const std::shared_ptr<const Address> &localAddress) {
	auto account = core->lookupKnownAccount(localAddress, true);
	if (!account) {
		// lWarning() << "No account found for local address: [" << *localAddress << "]";
		return nullptr;
	} else return getAudioVideoConferenceFactoryAddress(core, account);
}

std::shared_ptr<const Address> Core::getAudioVideoConferenceFactoryAddress(const std::shared_ptr<Core> &core,
                                                                           const std::shared_ptr<Account> &account) {
	if (!account) {
		lError() << "Unable to retrieve the audio video conference factory address from a null account";
		return nullptr;
	}
	const auto &params = account->getAccountParams();
	if (!params) {
		lError() << *account
		         << " doesn't have a valid set of parameters, hence it is not possible to retrieve the audio video "
		            "conference factory address";
		return nullptr;
	}
	auto address = params->getAudioVideoConferenceFactoryAddress();
	if (address == nullptr) {
		const auto &conferenceFactoryUri = getConferenceFactoryAddress(core, account);
		lWarning() << "Audio/video conference factory is null, fallback to default conference factory URI ["
		           << *conferenceFactoryUri << "]";
		if (!conferenceFactoryUri || !conferenceFactoryUri->isValid()) return nullptr;
		return conferenceFactoryUri;
	}
	return address;
}

void Core::initPlugins() {
	std::string pluginDir = Factory::get()->getLiblinphonePluginsDir();

	if (pluginDir.empty()) {
#ifdef __APPLE__
		pluginDir = getPlatformHelpers(getCCore())->getPluginsDir();
#else
#ifdef LINPHONE_PACKAGE_PLUGINS_DIR
		pluginDir = LINPHONE_PACKAGE_PLUGINS_DIR;
#else
		pluginDir = "";
#endif
#endif
	}

	if (!pluginDir.empty()) {
		lInfo() << "Loading linphone core plugins from " << pluginDir;
		loadPlugins(pluginDir);
	}
}

int Core::loadPlugins(BCTBX_UNUSED(const std::string &dir)) {
#ifdef __IOS__
	lInfo() << "Loading of plugins is not supported in iOS systems";
	return -1;
#endif // __IOS__

	int num = 0;
#if defined(_WIN32) && !defined(_WIN32_WCE)
	WIN32_FIND_DATA FileData;
	HANDLE hSearch;
	std::string szDirPath;
	BOOL fFinished = FALSE;
	// Start searching for .dll files in the current directory.
	szDirPath += dir;
	szDirPath.append("\\liblinphone_*.dll");
#ifdef UNICODE
	std::wstring wszDirPath(szDirPath.begin(), szDirPath.end());
	hSearch = FindFirstFileExW(wszDirPath.c_str(), FindExInfoStandard, &FileData, FindExSearchNameMatch, NULL, 0);
#else
	hSearch = FindFirstFileExA(szDirPath.c_str(), FindExInfoStandard, &FileData, FindExSearchNameMatch, NULL, 0);
#endif
	if (hSearch == INVALID_HANDLE_VALUE) {
		lInfo() << "no plugin (*.dll) found in [" << szDirPath << "] [" << (int)GetLastError() << "].";
		return 0;
	}

	while (!fFinished) {
		/* load library */
#ifdef MS2_WINDOWS_DESKTOP
		UINT em = 0;
#endif
		HINSTANCE os_handle;
		std::string szPluginFile(dir);
		szPluginFile.append("\\");
#ifdef UNICODE
		wchar_t wszPluginFile[2048];
		char filename[512];
		wcstombs(filename, FileData.cFileName, sizeof(filename));
		szPluginFile += std::string(filename);
		mbstowcs(wszPluginFile, szPluginFile.c_str(), sizeof(wszPluginFile));
#else
		szPluginFile += std::string(FileData.cFileName);
#endif
#if defined(MS2_WINDOWS_DESKTOP) && !defined(MS2_WINDOWS_UWP)

#ifdef UNICODE
		os_handle = LoadLibraryExW(wszPluginFile, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
#else
		os_handle = LoadLibraryExA(szPluginFile.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
#endif
		if (os_handle == NULL) {
			lError() << "Fail to load plugin " << szPluginFile << " with altered search path: error "
			         << (int)GetLastError();
#ifdef UNICODE
			os_handle = LoadLibraryExW(wszPluginFile, NULL, 0);
#else
			os_handle = LoadLibraryExA(szPluginFile.c_str(), NULL, 0);
#endif
		}
#else
		os_handle = LoadPackagedLibrary(wszPluginFile, 0);
#endif
		if (os_handle == NULL) lError() << "Fail to load plugin " << szPluginFile << ": error " << (int)GetLastError();
		else {
			init_func_t initroutine;
			std::string szPluginName;
#ifdef UNICODE
			szPluginName = std::string(filename);
#else
			szPluginName = std::string(FileData.cFileName);
#endif
			/*on mingw, dll names might be libsomething-3.dll. We must skip the -X.dll stuff*/
			auto minus = szPluginName.find('-');
			if (minus != std::string::npos) szPluginName = szPluginName.substr(0, minus);
			else szPluginName = szPluginName.substr(0, szPluginName.size() - 4); /*remove .dll*/
			std::string szMethodName(szPluginName);
			szMethodName.append("_init");
			initroutine = (init_func_t)GetProcAddress(os_handle, szMethodName.c_str());
			if (initroutine != NULL) {
				LinphoneCore *lc = L_GET_C_BACK_PTR(this);
				initroutine(lc);
				lInfo() << "Plugin loaded (" << szPluginFile << ")";
				// Add this new loaded plugin to the list (useful for FreeLibrary at the end)
				loadedPlugins.push_back(os_handle);
				plugins.push_back(szPluginName);
				num++;
			} else {
				lWarning() << "Could not locate init routine of plugin " << szPluginFile << ". Should be "
				           << szMethodName;
			}
		}
		if (!FindNextFile(hSearch, &FileData)) {
			if (GetLastError() == ERROR_NO_MORE_FILES) {
				fFinished = TRUE;
			} else {
				lError() << "couldn't find next plugin dll.";
				fFinished = TRUE;
			}
		}
	}
	/* Close the search handle. */
	FindClose(hSearch);

#elif defined(HAVE_DLOPEN)
	DIR *ds;
	std::list<std::string> loaded_plugins;
	struct dirent *de;
	char *ext;
	ds = opendir(dir.c_str());
	if (ds == NULL) {
		lInfo() << "Cannot open directory " << dir << ": " << strerror(errno);
		return -1;
	}
	while ((de = readdir(ds)) != NULL) {
		if (
#ifndef __QNX__
		    (de->d_type == DT_REG || de->d_type == DT_UNKNOWN || de->d_type == DT_LNK) &&
#endif
		    (strstr(de->d_name, "liblinphone_") == de->d_name) &&
		    ((ext = strstr(de->d_name, LINPHONE_PLUGINS_EXT)) != NULL)) {
			std::string plugin_file_name(de->d_name);
			if (std::find(loaded_plugins.cbegin(), loaded_plugins.cend(), plugin_file_name) != loaded_plugins.cend())
				continue;
			if (dlopenPlugin(dir, de->d_name)) {
				loaded_plugins.push_back(plugin_file_name);
				num++;
			}
		}
	}
	closedir(ds);
#else
	lWarning() << "no loadable plugin support: plugins cannot be loaded.";
	num = -1;
#endif
	return num;
}

bool_t Core::dlopenPlugin(BCTBX_UNUSED(const std::string &plugin_path),
                          BCTBX_UNUSED(const std::string plugin_file_name)) {
	bool_t plugin_loaded = FALSE;
#if defined(HAVE_DLOPEN)
	void *handle = NULL;
	void *initroutine = NULL;
	std::string initroutine_name = plugin_file_name;
	std::string plugin_name;
	std::string fullpath;

	if (!plugin_path.empty()) {
		fullpath += plugin_path;
		fullpath.append("/");
	}
	fullpath += plugin_file_name;
	lInfo() << "Loading plugin " << fullpath << " ...";

	if ((handle = dlopen(fullpath.c_str(), RTLD_NOW)) == NULL) {
		lWarning() << "Fail to load plugin " << fullpath << ": " << dlerror();
		if (handle) {
			dlclose(handle);
		}
	} else {
		plugin_name = plugin_file_name;
		auto ext = plugin_name.find(LINPHONE_PLUGINS_EXT);
		if (ext != std::string::npos) plugin_name = plugin_name.substr(0, ext);
		if (!plugin_name.empty()) {
			initroutine_name = plugin_name + std::string("_init");
			initroutine = dlsym(handle, initroutine_name.c_str());
		}
	}

#ifdef __APPLE__
	if (initroutine == NULL) {
		/* on macosx: library name are libxxxx.1.2.3.dylib */
		/* -> MUST remove the .1.2.3 */
		plugin_name = plugin_file_name;
		auto dot = plugin_name.find('.');
		if (dot != std::string::npos) plugin_name = plugin_name.substr(0, dot);
		if (!plugin_name.empty()) {
			initroutine_name = plugin_name + std::string("_init");
			initroutine = dlsym(handle, initroutine_name.c_str());
		}
	}
#endif

	if (initroutine != NULL) {
		init_func_t func = (init_func_t)initroutine;
		LinphoneCore *lc = L_GET_C_BACK_PTR(this);
		func(lc);
		lInfo() << "Plugin " << plugin_name << " loaded (file " << fullpath << ")";
		plugins.push_back(plugin_name);
		loadedPlugins.push_back(handle);
		plugin_loaded = TRUE;
	} else {
		lInfo() << "Could not locate init routine " << initroutine_name << " of plugin " << plugin_file_name;
		if (handle) {
			dlclose(handle);
		}
	}
#endif
	return plugin_loaded;
}

void Core::uninitPlugins() {
	for (auto &handle : loadedPlugins) {
#if defined(_WIN32)
		FreeLibrary(handle);
#elif defined(HAVE_DLOPEN)
		dlclose(handle);
#endif
	}
	loadedPlugins.clear();
	plugins.clear();
}

const std::list<std::string> &Core::getPluginList() const {
	return plugins;
}

bool Core::isPluginLoaded(const std::string name) const {
	return (std::find(plugins.cbegin(), plugins.cend(), name) != plugins.cend());
}

void Core::addOrUpdatePublishByEtag(SalPublishOp *op, shared_ptr<LinphonePrivate::EventPublish> event) {
	char generatedETag_char[ETAG_SIZE] = {0};
	do {
		belle_sip_random_token(generatedETag_char, sizeof(generatedETag_char));
	} while (mPublishByEtag.find(generatedETag_char) != mPublishByEtag.end());
	mPublishByEtag.erase(op->getETag());
	mPublishByEtag.insert_or_assign(generatedETag_char, event);
	op->setETag(generatedETag_char);
}

Core::ETagStatus Core::eTagHandler(SalPublishOp *op, const SalBodyHandler *body) {
	string eTag(op->getETag());

	if (!eTag.empty()) {
		auto it = mPublishByEtag.find(eTag);
		if (it == mPublishByEtag.end()) {
			lWarning() << "Unknown eTag [" << eTag << "]";
			op->replyMessage(SalReasonConditionalRequestFailed);
			op->release();
			return Core::ETagStatus::Error;
		}
	}

	if (body) {
		if (eTag.empty()) {
			return Core::ETagStatus::AddOrUpdateETag;
		} else {
			auto it = mPublishByEtag.find(eTag);
			if (it != mPublishByEtag.end()) {
				return Core::ETagStatus::AddOrUpdateETag;
			}
		}
	} else {
		auto it = mPublishByEtag.find(eTag);
		if (op->getExpires() == 0) {
			if (it != mPublishByEtag.end()) mPublishByEtag.erase(it);
			// else already expired
		} else {
			if (it != mPublishByEtag.end()) {
				return Core::ETagStatus::AddOrUpdateETag;
			} else {
				lWarning() << "Unknown eTag [" << eTag << "]";
				op->replyMessage(SalReasonUnknown);
				op->release();
				return Core::ETagStatus::Error;
			}
		}
	}
	return Core::ETagStatus::None;
}

void Core::setLabel(const std::string &label) {
	L_D();
	d->logLabel = label;
}

const std::string &Core::getLabel() const {
	L_D();
	return d->logLabel;
}

void Core::setVideoCodecPriorityPolicy(LinphoneCodecPriorityPolicy policy) {
	L_D();
	if (linphone_core_ready(getCCore())) {
		linphone_config_set_int(linphone_core_get_config(getCCore()), "video", "codec_priority_policy", (int)policy);
	}
	d->videoCodecPriorityPolicy = policy;
	d->reorderVideoCodecList();
}

LinphoneCodecPriorityPolicy Core::getVideoCodecPriorityPolicy() const {
	L_D();
	return d->videoCodecPriorityPolicy;
}

void Core::setDefaultAccount(const std::shared_ptr<Account> &account) {
	/* check if this account is in our list */
	if (account) {
		const auto &accounts = getAccounts();
		const auto accountIt = std::find(accounts.cbegin(), accounts.cend(), account);
		if (accountIt == accounts.cend()) {
			lWarning() << "Bad account address: it is not in the list !";
			mDefaultAccount = nullptr;
			linphone_core_notify_default_account_changed(getCCore(), NULL);
			return;
		}
	}

	if (mDefaultAccount == account) {
		lInfo() << "Account " << account << " is already the default one, skipping.";
		return;
	}

	mDefaultAccount = account;
	if (linphone_core_ready(getCCore())) {
		linphone_config_set_int(linphone_core_get_config(getCCore()), "sip", "default_proxy", getDefaultAccountIndex());
		/* Invalidate phone numbers in friends maps when default account changes because the new one may have a
		 * different dial prefix */
		linphone_core_invalidate_friends_maps(getCCore());
	}

	linphone_core_notify_default_account_changed(getCCore(), account ? account->toC() : nullptr);
}

const std::shared_ptr<Account> &Core::getDefaultAccount() const {
	return mDefaultAccount;
}

void Core::setDefaultAccountIndex(int index) {
	std::shared_ptr<Account> defaultAccount = nullptr;
	if (index < 0) {
		lInfo() << "Resetting default account as a negative index (" << index << ") has been provided";
	} else {
		const auto &accounts = getAccounts();
		if (static_cast<size_t>(index) < accounts.size()) {
			auto accountIt = accounts.cbegin();
			std::advance(accountIt, index);
			defaultAccount = *accountIt;
		} else {
			lInfo() << "Do not changing default account because the index request (" << index
			        << ") is larger than the number of accounts available " << accounts.size();
			defaultAccount = getDefaultAccount();
		}
	}
	setDefaultAccount(defaultAccount);
}

int Core::getDefaultAccountIndex() const {
	int pos = -1;
	const auto &accounts = getAccounts();
	const auto accountIt = std::find(accounts.cbegin(), accounts.cend(), getDefaultAccount());
	if (accountIt != accounts.cend()) {
		return static_cast<int>(std::distance(accounts.cbegin(), accountIt));
	}
	return pos;
}

void Core::removeAccount(std::shared_ptr<Account> account) {
	/* check this account is in the list before doing more*/
	auto &accounts = mAccounts.mList;
	const auto accountIt = std::find(accounts.cbegin(), accounts.cend(), account);
	if (accountIt == accounts.cend()) {
		lError() << "Account [ " << account << "] is not known by Core [" << this << "] (programming error?)";
		return;
	}

	auto accountParams = account->getAccountParams();
	auto coreState = linphone_core_get_global_state(getCCore());
	/* we also need to update the accounts list */
	accounts.erase(accountIt);
	removeDependentAccount(account);
	/* When the core is ON, add to the list of destroyed accounts, so that the possible unREGISTER request can succeed
	 * authentication. If not ON, no unregistration is made in this case. This might happen during a Core restart.
	 * Also there is not need to trigger the deletion timer if the account hasn't registered.
	 * The if statement is also entered if a REGISTER is sent and the account is removed while it is still in the state
	 * LinphoneRegistrationProgress (i.e. before receiving the 200Ok response). In such a scenario, in fact an account
	 * waits for the 200Ok response and then it sends another REGISTER message with the Expires header set to 0. */
	if (accountParams->getRegisterEnabled() && (coreState == LinphoneGlobalOn)) {
		mDeletedAccounts.mList.push_back(account);
		account->triggerDeletion();
	} else {
		account->releaseOps();
		account->setConfig(nullptr);
	}

	invalidateAccountInConferencesAndChatRooms(account);

	if (getDefaultAccount() == account) {
		setDefaultAccount(nullptr);
	}

	linphone_core_notify_account_removed(getCCore(), account->toC());

	if (coreState == LinphoneGlobalOn) {
		// Update the associated linphone specs on the core
		auto newAccountParams = accountParams->clone()->toSharedPtr();
		newAccountParams->setRegisterEnabled(false);
		newAccountParams->setConferenceFactoryAddress(nullptr);
		account->setAccountParams(newAccountParams);
	}
	auto state = account->getState();
	switch (state) {
		case LinphoneRegistrationNone:
		case LinphoneRegistrationOk:
		case LinphoneRegistrationRefreshing:
		case LinphoneRegistrationProgress:
			// iterate will do the job.
			break;
		case LinphoneRegistrationFailed:
		case LinphoneRegistrationCleared:
			account->setState(LinphoneRegistrationNone, "Registration disabled");
			break;
	}
	Account::writeAllToConfigFile(getSharedFromThis());
}

void Core::removeDeletedAccount(const std::shared_ptr<Account> &account) {
	auto &accounts = mDeletedAccounts.mList;
	const auto accountIt = std::find(accounts.cbegin(), accounts.cend(), account);
	if (accountIt == accounts.cend()) {
		lError() << "Account [ " << account << "] is not in the list of deleted accounts";
		return;
	}
	const auto &params = account->getAccountParams();
	lInfo() << "Account for [" << *params->getServerAddress() << "] is definitely removed from core.";
	account->releaseOps();
	// Setting the proxy config associated to an account to NULL will cause its destruction and therefore losing the
	// reference held by it to the Account object. In this way, the memory occupied by the account to be deleted is
	// properly once it is erased by the account list
	account->setConfig(nullptr);
	/* we also need to update the accounts list */
	accounts.erase(accountIt);
}

void Core::removeDependentAccount(const std::shared_ptr<Account> &account) {
	const auto &params = account->getAccountParams();
	const auto &accountIdKey = params->getIdKey();
	auto &accounts = mAccounts.mList;
	for (const auto &accountInList : accounts) {
		if ((accountInList != account) && (accountInList->getDependency() == account)) {
			lInfo() << "Updating dependent account [" << accountInList
			        << "] caused by removal of 'master' account idkey[" << accountIdKey << "]";
			accountInList->setDependency(NULL);
			account->setNeedToRegister(account->getAccountParams()->getRegisterEnabled());
			accountInList->update();
		}
	}
}

LinphoneStatus Core::addAccount(std::shared_ptr<Account> account) {
	if (!account || !account->check()) {
		return -1;
	}
	const auto &accounts = getAccounts();
	if (std::find(accounts.cbegin(), accounts.cend(), account) != accounts.cend()) {
		lWarning() << "Account already entered, ignored.";
		return 0;
	}
	account->cancelDeletion(); // in case this account had been previously be removed from the Core.
	mAccounts.mList.push_back(account);

	// If there is no back pointer to a proxy config then create a proxy config that will depend on this account
	// to ensure backward compatibility when using only proxy configs
	LinphoneProxyConfig *cfg = account->getConfig();
	if (cfg) {
		account->setConfig(cfg);
	} else {
		cfg = belle_sip_object_new(LinphoneProxyConfig);
		cfg->account = linphone_account_ref(account->toC());
		account->setConfig(cfg);
		linphone_proxy_config_unref(cfg);
	}

	account->apply(getCCore());

	linphone_core_notify_account_added(getCCore(), account->toC());

	return 0;
}

void Core::resetAccounts() {
	// The 2 for-loops below will break a circular dependency as the proxy config has a reference towards the account
	// and the account keeps a reference of the proxy config. When the core is shutting down, awe need to break this
	// circular referencing from all actual and deleted accounts
	for (const auto &account : getAccounts()) {
		account->setConfig(nullptr);
	}

	for (const auto &account : getDeletedAccounts()) {
		account->setConfig(nullptr);
	}
}

void Core::clearAccounts() {
	auto accountList = mAccounts.mList;
	for (auto &account : accountList) {
		removeAccount(account);
	}
	Account::writeAllToConfigFile(getSharedFromThis());
}

std::shared_ptr<Account> Core::getAccountByIdKey(const std::string idKey) const {
	if (idKey.empty()) return nullptr;
	for (const auto &account : getAccounts()) {
		const auto &params = account->getAccountParams();
		const auto &accountIdKey = params->getIdKey();
		if (!accountIdKey.empty() && (accountIdKey.compare(idKey) == 0)) return account;
	}
	return nullptr;
}

const std::list<std::shared_ptr<Account>> &Core::getDeletedAccounts() const {
	return mDeletedAccounts.mList;
}

const bctbx_list_t *Core::getDeletedAccountsCList() const {
	return mDeletedAccounts.getCList();
}

void Core::clearProxyConfigList() const {
	if (mCachedProxyConfigs) {
		bctbx_list_free_with_data(mCachedProxyConfigs, (void (*)(void *))linphone_proxy_config_unref);
		mCachedProxyConfigs = NULL;
	}
}

void Core::releaseAccounts() {
	clearProxyConfigList();
	for (const auto &account : getAccounts()) {
		account->releaseOps();
	}

	for (const auto &account : getDeletedAccounts()) {
		account->releaseOps();
		account->cancelDeletion();
	}
}

const bctbx_list_t *Core::getProxyConfigList() const {
	clearProxyConfigList();
	for (const auto &account : getAccounts()) {
		mCachedProxyConfigs =
		    bctbx_list_append(mCachedProxyConfigs, (void *)linphone_proxy_config_ref(account->getConfig()));
	}
	return mCachedProxyConfigs;
}

const std::list<std::shared_ptr<Account>> &Core::getAccounts() const {
	return mAccounts.mList;
}

const bctbx_list_t *Core::getAccountsCList() const {
	return mAccounts.getCList();
}

std::shared_ptr<Account> Core::lookupKnownAccount(const std::shared_ptr<const Address> uri,
                                                  bool fallbackToDefault) const {
	if (uri) {
		return lookupKnownAccount(*uri, fallbackToDefault);
	}
	return nullptr;
}

std::shared_ptr<Account> Core::lookupKnownAccount(const Address &uri, bool fallbackToDefault) const {
	std::shared_ptr<Account> foundAccount = NULL;
	std::shared_ptr<Account> foundRegAccount = NULL;
	std::shared_ptr<Account> foundNoRegAccount = NULL;
	std::shared_ptr<Account> foundAccountDomainMatch = NULL;
	std::shared_ptr<Account> foundRegAccountDomainMatch = NULL;
	std::shared_ptr<Account> foundNoRegAccountDomainMatch = NULL;
	std::shared_ptr<Account> defaultAccount = getDefaultAccount();

	if (!uri.isValid()) {
		lError() << "Cannot look for account for NULL uri, returning default";
		return defaultAccount;
	}
	const std::string uriDomain = uri.getDomain();
	if (uriDomain.empty()) {
		lInfo() << "Cannot look for account for uri [" << uri << "] that has no domain set, returning default";
		return defaultAccount;
	}

	/*return default account if it is matching the destination uri*/
	if (defaultAccount) {
		const auto &defaultAccountParams = defaultAccount->getAccountParams();
		const auto &identityAddress = defaultAccountParams->getIdentityAddress();
		if (uri.weakEqual(*identityAddress)) {
			foundAccount = defaultAccount;
			goto end;
		}
		const auto &domain = defaultAccountParams->getDomain();
		if (!domain.empty() && (domain == uriDomain)) {
			foundAccountDomainMatch = defaultAccount;
		}
	}

	/*otherwise return first registered, then first registering matching, otherwise first matching */
	for (const auto &account : getAccounts()) {
		const auto &accountParams = account->getAccountParams();
		const auto &identityAddress = accountParams->getIdentityAddress();
		const auto registered = (account->getState() == LinphoneRegistrationOk);
		const auto canRegister = accountParams->getRegisterEnabled();
		if (uri.weakEqual(*identityAddress)) {
			if (registered) {
				foundAccount = account;
				break;
			} else if (!foundRegAccount && canRegister) {
				foundRegAccount = account;
			} else if (!foundNoRegAccount) {
				foundNoRegAccount = account;
			}
		}
		const auto &domain = accountParams->getDomain();
		if (!domain.empty() && (domain == uriDomain)) {
			if (!foundAccountDomainMatch && registered) {
				foundAccountDomainMatch = account;
			} else if (!foundRegAccountDomainMatch && canRegister) {
				foundRegAccountDomainMatch = account;
			} else if (!foundNoRegAccountDomainMatch) {
				foundNoRegAccountDomainMatch = account;
			}
		}
	}
end:
	// ============ Choose the the most appropriate account =====================
	// Check first if there is an account whose identity address matches the uri passed as argument to this
	// function. Then try to guess an account based on the same domain as the address passed as argument to this
	// function

	// Account matched by identity address comparison
	if (!foundAccount && foundRegAccount) foundAccount = foundRegAccount;
	else if (!foundAccount && foundNoRegAccount) foundAccount = foundNoRegAccount;

	// Default account fallback
	if (foundAccount && foundAccount != defaultAccount) {
		ms_debug("Overriding default account setting for this call/message/subscribe operation.");
	} else if (fallbackToDefault && !foundAccount) {
		foundAccount = defaultAccount; /*when no matching account is found, use the default account*/
	}

	// Account matched by identity address domain comparison
	if (!foundAccount && foundAccountDomainMatch) foundAccount = foundAccountDomainMatch;
	else if (!foundAccount && foundRegAccountDomainMatch) foundAccount = foundRegAccountDomainMatch;
	else if (!foundAccount && foundNoRegAccountDomainMatch) foundAccount = foundNoRegAccountDomainMatch;
	return foundAccount;
}

std::shared_ptr<Account> Core::findAccountByIdentityAddress(const std::shared_ptr<const Address> identity) const {
	std::shared_ptr<Account> found = nullptr;
	if (!identity) return found;

	const auto accounts = mAccounts.mList;
	for (const auto &account : accounts) {
		const auto &params = account->getAccountParams();
		const auto &address = params->getIdentityAddress();
		if (address && identity->weakEqual(address)) {
			found = account;
			break;
		}
	}
	return found;
}

std::shared_ptr<Account> Core::findAccountByUsername(const std::string &username) const {
	std::shared_ptr<Account> found = nullptr;
	if (username.empty()) return found;

	auto &accounts = mAccounts.mList;
	for (const auto &account : accounts) {
		const auto &params = account->getAccountParams();
		const auto &address = params->getIdentityAddress();
		if (address && address->getUsername() == username) {
			found = account;
			break;
		}
	}
	return found;
}

void Core::notifyPublishStateChangedToAccount(const std::shared_ptr<Event> event, LinphonePublishState state) {
	auto &accounts = mAccounts.mList;
	auto accountIt = std::find_if(accounts.begin(), accounts.end(), [&event](const auto &account) {
		auto publishEvent = account->getPresencePublishEvent();
		return (publishEvent && (publishEvent == event));
	});

	if (accountIt != accounts.end()) {
		(*accountIt)->notifyPublishStateChanged(state);
	}
}

int Core::sendPublish(LinphonePresenceModel *presence) {
	auto &accounts = mAccounts.mList;
	for (const auto &account : accounts) {
		const auto &params = account->getAccountParams();
		if (params->getPublishEnabled()) {
			account->setPresenceModel(presence);
			account->sendPublish();
		}
	}
	return 0;
}

bool Core::refreshTokens(const std::shared_ptr<AuthInfo> &ai) {
	if (ai->getTokenEndpointUri().empty()) {
		if (ai->getAuthorizationServer().empty()) {
			lWarning() << "Core::refreshTokens(): no token endpoint uri and no authorization server uri set.";
			return false;
		}
		ai->setTokenEndpointUri(ai->getAuthorizationServer() + "/token");
		lWarning()
		    << "Core::refreshTokens(): token endpoint uri guessed from authorization server base uri - not reliable.";
	}
	if (ai->getRefreshToken() == nullptr) {
		lWarning() << "Core::refreshTokens(): no refresh token is set.";
		return false;
	}
	std::string form = "grant_type=refresh_token&refresh_token=" + ai->getRefreshToken()->getToken();
	if (!ai->getClientId().empty()) {
		form += "&client_id=" + ai->getClientId();
	}
	try {
		getHttpClient()
		    .createRequest("POST", ai->getTokenEndpointUri())
		    .setBody(Content(ContentType("application", "x-www-form-urlencoded"), form))
		    .execute([this, ai](const HttpResponse &response) {
			    if (response.getHttpStatusCode() == 200) {
				    JsonDocument doc(response.getBody());
				    const Json::Value &root = doc.getRoot();
				    if (!root.isNull()) {
					    auto bearerToken = (new BearerToken(root["access_token"].asString(),
					                                        time(NULL) + (time_t)root["expires_in"].asInt64()))
					                           ->toSharedPtr();
					    ai->setAccessToken(bearerToken);
					    string refreshToken = root["refresh_token"].asString();
					    if (!refreshToken.empty()) {
						    // FIXME: how to get the expiration of the refresh token ?
						    auto bearerRefreshToken = (new BearerToken(refreshToken, time(NULL)))->toSharedPtr();
						    ai->setRefreshToken(bearerRefreshToken);
					    }
					    /* this will resubmit all pending authentications */
					    linphone_core_add_auth_info(getCCore(), ai->toC());
					    return;
				    }
			    }
			    lError() << "Token refreshing failed, invoking authentication_requested...";
			    linphone_core_notify_authentication_requested(getCCore(), ai->toC(), LinphoneAuthBearer);
		    });
	} catch (const std::exception &e) {
		lError() << "Cannot refresh access token: " << e.what();
		return false;
	}
	return true;
}

HttpClient &Core::getHttpClient() {
	L_D();
	if (!d->httpClient) {
		d->httpClient.reset(new HttpClient(getSharedFromThis()));
	}
	return *d->httpClient;
}

void Core::stopHttpClient() {
	L_D();
	d->httpClient.reset();
}

#ifdef HAVE_ADVANCED_IM
shared_ptr<EktInfo> Core::createEktInfoFromXml(const std::string &xmlBody) const {
	istringstream data(xmlBody);
	unique_ptr<CryptoType> crypto;
	auto ei = (new EktInfo())->toSharedPtr();

	try {
		crypto = parseCrypto(data, Xsd::XmlSchema::Flags::dont_validate);
	} catch (const exception &) {
		lError() << "Core::createEktInfoFromXml : Error while parsing crypto XML";
		return ei;
	}

	if (const auto &sSpi = crypto->getSspi()) {
		ei->setSSpi(static_cast<uint16_t>(sSpi));
	} else {
		lError() << "Core::createEktInfoFromXml : Missing sSPI";
		return ei;
	}

	auto &cSpi = crypto->getCspi();
	if (cSpi.present()) {
		ei->setCSpi(bctoolbox::decodeBase64(cSpi.get()));
	}

	auto &ciphers = crypto->getCiphers();
	if (ciphers.present()) {
		for (auto &cipher : ciphers->getEncryptedekt())
			ei->addCipher(cipher.getTo(), bctoolbox::decodeBase64(cipher));
	}

	auto &from = crypto->getFrom();
	if (from) ei->setFrom(*interpretUrl(from.get(), false));

	return ei;
}

string Core::createXmlFromEktInfo(const shared_ptr<const EktInfo> &ei, const shared_ptr<const Account> &account) const {
	stringstream xmlBody;
	shared_ptr<Address> addr = nullptr;
	if (account) {
		addr = account->getContactAddress();
	} else if (getDefaultAccount()) {
		lWarning() << __func__ << " : The account passed as an argument is null, using the default account";
		addr = getDefaultAccount()->getContactAddress();
	} else {
		lError() << __func__ << " : No valid account found, return an empty XML body";
		return xmlBody.str();
	}

	auto crypto = CryptoType(ei->getSSpi(), addr->asStringUriOnly());

	if (ei->getFrom()) crypto.setFrom(ei->getFrom()->asStringUriOnly());

	if (!ei->getCSpi().empty()) crypto.setCspi(bctoolbox::encodeBase64(ei->getCSpi()));

	auto dict = ei->getCiphers();
	map<string, Variant> cipherMap;
	if (dict != nullptr) cipherMap = dict->getProperties();
	if (!cipherMap.empty()) {
		CiphersType ciphers;
		crypto.setCiphers(ciphers);
		for (const auto &cipher : cipherMap) {
			vector<uint8_t> cipherVec(ei->getCiphers()->getBuffer(cipher.first)->getContent().begin(),
			                          ei->getCiphers()->getBuffer(cipher.first)->getContent().end());
			auto ekt = EncryptedektType(bctoolbox::encodeBase64(cipherVec), cipher.first);
			crypto.getCiphers()->getEncryptedekt().push_back(ekt);
		}
	}

	Xsd::XmlSchema::NamespaceInfomap map;
	map[""].name = "linphone:xml:ns:ekt-linphone-extension";
	serializeCrypto(xmlBody, crypto, map);
	return xmlBody.str();
}
#endif // HAVE_ADVANCED_IM

ConferenceIdParams Core::createConferenceIdParams() const {
	return ConferenceIdParams(getSharedFromThis());
}

void Core::addFriendList(const shared_ptr<FriendList> &list) {
	L_D();

	d->friendLists.push_back(list);
}

void Core::removeFriendList(const shared_ptr<FriendList> &list) {
	L_D();

	d->friendLists.remove(list);
}

void Core::clearFriendLists() {
	L_D();

	d->friendLists.clear();
}

const list<shared_ptr<FriendList>> &Core::getFriendLists() const {
	L_D();

	return d->friendLists;
}

bool Core::isEktPluginLoaded() const {
	return mEktPluginLoaded;
}

void Core::setEktPluginLoaded(bool ektPluginLoaded) {
	mEktPluginLoaded = ektPluginLoaded;
}

std::shared_ptr<Account> Core::guessLocalAccountFromMalformedMessage(const std::shared_ptr<Address> &localAddress,
                                                                     const std::shared_ptr<Address> &peerAddress) {
	if (!localAddress) return nullptr;

	auto account = findAccountByIdentityAddress(localAddress);
	if (!account) {
		string toUser = localAddress->getUsername();
		if (!toUser.empty() && peerAddress && Utils::isIp(localAddress->getDomain())) {
			Address localAddressWithPeerDomain(*localAddress);
			localAddressWithPeerDomain.setDomain(peerAddress->getDomain());
			localAddressWithPeerDomain.setPort(0);
			account = lookupKnownAccount(localAddressWithPeerDomain, false);
			if (account) {
				// We have a match for the FROM domain and the TO username.
				// We may face an IPBPX that sets the TO domain to our IP address, which is
				// a terribly stupid idea.
				lWarning() << "Detecting TO header [" << localAddress->asStringUriOnly()
				           << "] probably ill-choosen, but an account that matches the username on "
				              "the FROM ["
				           << peerAddress->asStringUriOnly() << "] domain was found: ["
				           << account->getAccountParams()->getIdentityAddress()->asStringUriOnly() << "]";
				return account;
			} else {
				account = findAccountByUsername(toUser);
				if (account) {
					lWarning() << "Detecting TO header [" << localAddress->asStringUriOnly()
					           << "] probably ill-choosen, but an account that matches the username "
					              "was found: ["
					           << account->getAccountParams()->getIdentityAddress()->asStringUriOnly() << "]";
					return account;
				} else {
					lWarning() << "Failed to find an account matching TO header [" << localAddress->asStringUriOnly()
					           << "], even by using FROM header [" << peerAddress->asStringUriOnly() << "] domain";
				}
			}
		}
	}

	return nullptr;
}

LINPHONE_END_NAMESPACE
