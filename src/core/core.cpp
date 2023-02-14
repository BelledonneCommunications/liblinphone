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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

#include <algorithm>
#include <iterator>

#include "bctoolbox/utils.hh"
#include <bctoolbox/defs.h>

#include <mediastreamer2/mscommon.h>

#ifdef HAVE_ADVANCED_IM
#include <xercesc/util/PlatformUtils.hpp>
#endif

#include "account/account.h"
#include "address/address.h"
#include "call/call.h"
#include "chat/encryption/encryption-engine.h"
#ifdef HAVE_LIME_X3DH
#include "chat/encryption/lime-x3dh-encryption-engine.h"
#endif
#include "chat/encryption/lime-x3dh-server-engine.h"
#ifdef HAVE_ADVANCED_IM
#include "conference/handlers/local-conference-list-event-handler.h"
#include "conference/handlers/remote-conference-list-event-handler.h"
#endif
#include "core/core-listener.h"
#include "core/core-p.h"
#include "chat/chat-room/chat-room-p.h"
#include "ldap/ldap.h"
#include "logger/logger.h"
#include "paths/paths.h"
#include "linphone/utils/utils.h"
#include "linphone/utils/algorithm.h"
#include "linphone/lpconfig.h"
#include "factory/factory.h"

#include "conference/session/media-session.h"
#include "conference/session/media-session-p.h"
#include "conference/params/media-session-params-p.h"
#include "conference/session/streams.h"
#include "conference/participant.h"
#include "conference_private.h"

#include "sal/sal_media_description.h"

// TODO: Remove me later.
#include "c-wrapper/c-wrapper.h"
#include "private.h"

#define LINPHONE_DB "linphone.db"
#define LINPHONE_CALL_HISTORY_DB "call-history.db"
#define LINPHONE_ZRTP_SECRETS_DB "zrtp-secrets.db"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

const Utils::Version CorePrivate::conferenceProtocolVersion(1, 0);
const Utils::Version CorePrivate::groupChatProtocolVersion(1, 2);
const Utils::Version CorePrivate::ephemeralProtocolVersion(1, 1);

void CorePrivate::init () {
	L_Q();

	mainDb.reset(new MainDb(q->getSharedFromThis()));
	getToneManager(); // Forces instanciation of the ToneManager.
#ifdef HAVE_ADVANCED_IM
	remoteListEventHandler = makeUnique<RemoteConferenceListEventHandler>(q->getSharedFromThis());
	localListEventHandler = makeUnique<LocalConferenceListEventHandler>(q->getSharedFromThis());
#endif

	LinphoneCore *lc = L_GET_C_BACK_PTR(q);
	if (q->limeX3dhAvailable()) {
		bool limeEnabled = linphone_config_get_bool(lc->config, "lime", "enabled", TRUE);
		if (limeEnabled) {
			q->enableLimeX3dh(true);
		}
	}

	int tmp = linphone_config_get_int(lc->config,"sip", "lime", LinphoneLimeDisabled);
	LinphoneLimeState limeState = static_cast<LinphoneLimeState>(tmp);
	if (limeState != LinphoneLimeDisabled && q->limeX3dhEnabled()) {
		bctbx_fatal("You can't have both LIME and LIME X3DH enabled at the same time !\nConflicting settings are [sip] lime and [lime] lime_server_url");
	}
	linphone_core_enable_lime(lc, limeState);

	if (linphone_factory_is_database_storage_available(linphone_factory_get())) {
		AbstractDb::Backend backend;
		string uri = L_C_TO_STRING(linphone_config_get_string(linphone_core_get_config(lc), "storage", "uri", nullptr));
		if (!uri.empty())
			if (strcmp(linphone_config_get_string(linphone_core_get_config(lc), "storage", "backend", "sqlite3"), "mysql") == 0 ) {
				backend = MainDb::Mysql;
			} else {
				backend = MainDb::Sqlite3;
				if (uri != "null")
				    uri = Utils::quoteStringIfNotAlready(uri);
			}
		else {
			backend = AbstractDb::Sqlite3;
			string dbPath = Utils::quoteStringIfNotAlready(q->getDataPath() + LINPHONE_DB);
			lInfo() << "Using [" << dbPath << "] as default database path";
			uri = dbPath;
		}

		if (uri != "null"){ //special uri "null" means don't open database. We need this for tests.
			if (backend == MainDb::Mysql && uri.find("charset=") == string::npos) {
				lInfo() << "No charset defined forcing utf8 4 bytes specially for conference subjet storage";
				uri += " charset=utf8mb4";
			}
			lInfo() << "Opening linphone database " << uri << " with backend " << backend;
			uri = LinphonePrivate::Utils::localeToUtf8(uri);// `mainDb->connect` take a UTF8 string.
			auto startMs = bctbx_get_cur_time_ms();
			if (!mainDb->connect(backend, uri)) {
				ostringstream os;
				os << "Unable to open linphone database with uri " << uri << " and backend " << backend;
				throw DatabaseConnectionFailure(os.str());
			}
			auto stopMs = bctbx_get_cur_time_ms();
			auto duration = stopMs - startMs;
			if (duration >= 1000){
				lWarning() << "Opening database took " << duration << " ms !";
			}

			loadChatRooms();
		} else lWarning() << "Database explicitely not requested, this Core is built with no database support.";

		//Leave this part to import the legacy call logs to MainDB
		string calHistoryDbPath = L_C_TO_STRING(linphone_config_get_string(linphone_core_get_config(lc), "storage", "call_logs_db_uri", nullptr));
		if (calHistoryDbPath.empty())
			calHistoryDbPath = q->getDataPath() + LINPHONE_CALL_HISTORY_DB;
		if (calHistoryDbPath != "null") {
			lInfo() << "Using [" << calHistoryDbPath << "] as legacy call history database path";
			linphone_core_set_call_logs_database_path(lc, calHistoryDbPath.c_str());
		} else lWarning() << "Call logs database explicitely not requested";

		if (lc->zrtp_secrets_cache == NULL) {
			string zrtpSecretsDbPath = L_C_TO_STRING(linphone_config_get_string(linphone_core_get_config(lc), "storage", "zrtp_secrets_db_uri", nullptr));
			if (zrtpSecretsDbPath.empty())
				zrtpSecretsDbPath = q->getDataPath() + LINPHONE_ZRTP_SECRETS_DB;
			if (zrtpSecretsDbPath != "null") {
				lInfo() << "Using [" << zrtpSecretsDbPath << "] as default zrtp secrets database path";
				linphone_core_set_zrtp_secrets_file(lc, zrtpSecretsDbPath.c_str());
			} else lWarning() << "ZRTP secrets database explicitely not requested";
		}
	}

#ifdef __ANDROID__
	// On Android assume Core has been started in background,
	// otherwise first notifyEnterForeground() will do nothing.
	// If not, ActivityMonitor will tell us quickly.
	isInBackground = true;
#endif
}

void CorePrivate::registerListener (CoreListener *listener) {
	listeners.push_back(listener);
}

void CorePrivate::unregisterListener (CoreListener *listener) {
	listeners.remove(listener);
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

	d->bgTask.start(getSharedFromThis(), stopAsyncEnd, enableStopAsyncEnd, linphone_config_get_int(linphone_core_get_config(getCCore()), "misc", "max_stop_async_time", 10));
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
		LinphoneFriendList *list = (LinphoneFriendList *) elem->data;
		if (list->event) {
			return false;
		}
	}


	// Sometimes (bad network for example), backgrounds for chat message (imdn, delivery ...) take too much time.
	// In this case, forece linphonecore to off. Using "stop core async end" background task to do this.
	if (stopAsyncEndEnabled) {
		return true;
	}

	for (auto it = chatRoomsById.begin(); it != chatRoomsById.end(); it++) {
		const auto &chatRoom = dynamic_pointer_cast<ChatRoom>(it->second);
		if (chatRoom && (chatRoom->getPrivate()->getImdnHandler()->isCurrentlySendingImdnMessages() 
			|| !chatRoom->getPrivate()->getTransientChatMessages().empty())) {
			return false;
		}
	}

	return true;
}

// Called by _linphone_core_stop_async_start() to stop the asynchronous tasks.
// Put here the calls to stop some task with asynchronous process and check in CorePrivate::isShutdownDone() if they have finished.
void CorePrivate::shutdown() {
	L_Q();

	auto currentCalls = calls;
	for (auto call : currentCalls) {
		call->terminate();
	}

	bctbx_list_t *elem = NULL;
	for (elem = q->getCCore()->friends_lists; elem != NULL; elem = bctbx_list_next(elem)) {
		LinphoneFriendList *list = (LinphoneFriendList *) elem->data;
		linphone_friend_list_enable_subscriptions(list,FALSE);
	}
	
	audioDevices.clear();

	if (toneManager) toneManager->freeAudioResources();

	stopEphemeralMessageTimer();
	ephemeralMessages.clear();

	stopChatMessagesAggregationTimer();

	for (auto it = chatRoomsById.begin(); it != chatRoomsById.end(); it++) {
		const auto &chatRoom = it->second;
		const auto &chatRoomPrivate = chatRoom->getPrivate();
		for (auto &chatMessage : chatRoomPrivate->getTransientChatMessages()) {
			if (chatMessage->getState() == ChatMessage::State::FileTransferInProgress) {
				// Abort auto download file transfers
				if (chatMessage->getDirection() == ChatMessage::Direction::Incoming) {
					chatMessage->cancelFileTransfer();
				}
			}
		}
	}

	pushReceivedBackgroundTask.stop();
}

// Called by _linphone_core_stop_async_end() just before going to globalStateOff.
// Put here the data that need to be freed before the stop.
void CorePrivate::uninit() {
	L_Q();

	if (q->limeX3dhEnabled()) {
		q->enableLimeX3dh(false);
	}

	const list<shared_ptr<AbstractChatRoom>> chatRooms = q->getChatRooms();
	shared_ptr<ChatRoom> cr;
	for (const auto &chatRoom : chatRooms) {
		cr = dynamic_pointer_cast<ChatRoom>(chatRoom);
		if (cr) {
			cr->getPrivate()->getImdnHandler()->onLinphoneCoreStop();
#ifdef HAVE_ADVANCED_IM
			for (const auto &participant: cr->getParticipants()) {
				for (std::shared_ptr<ParticipantDevice> device : participant->getDevices() ) {
					//to make sure no more messages are received after Core:uninit because key components like DB are no longuer available. So it's no more possible to handle any singnaling messages properly.
					if (device->getSession())
						device->getSession()->setListener(nullptr);
				}
			}
#endif
			// end all file transfer background tasks before linphonecore off
			const auto &chatRoomPrivate = cr->getPrivate();
			for (auto &chatMessage : chatRoomPrivate->getTransientChatMessages()) {
				chatMessage->fileUploadEndBackgroundTask();
			}
		}
	}

	chatRoomsById.clear();

	for (const auto &audioVideoConference : q->audioVideoConferenceById) {
		// Terminate audio video conferences just before core is stopped
		audioVideoConference.second->terminate();
	}
	q->audioVideoConferenceById.clear();

	noCreatedClientGroupChatRooms.clear();
	listeners.clear();
	pushReceivedBackgroundTask.stop();
	mLdapServers.clear();

#ifdef HAVE_ADVANCED_IM
	remoteListEventHandler.reset();
	localListEventHandler.reset();
#endif

	Address::clearSipAddressesCache();

	// clear encrypted files plain cache directory
	std::string cacheDir(Factory::get()->getCacheDir(nullptr) + "/evfs/");
	bctbx_rmdir(cacheDir.c_str(), TRUE);

	/* The toneManager is kept until destructor, we may need it because of calls ended during linphone_core_destroy(). */
}

void CorePrivate::disconnectMainDb () {
	if (mainDb != nullptr) {
		mainDb->disconnect();
	}
}

// -----------------------------------------------------------------------------

void CorePrivate::notifyGlobalStateChanged (LinphoneGlobalState state) {
	auto listenersCopy = listeners; // Allow removal of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onGlobalStateChanged(state);
}

void CorePrivate::notifyNetworkReachable (bool sipNetworkReachable, bool mediaNetworkReachable) {
	auto listenersCopy = listeners; // Allow removal of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onNetworkReachable(sipNetworkReachable, mediaNetworkReachable);
}

void CorePrivate::notifyCallStateChanged (LinphoneCall *call, LinphoneCallState state, const string &message) {
	auto listenersCopy = listeners; // Allow removal of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onCallStateChanged(call, state, message);
}

void CorePrivate::notifyRegistrationStateChanged (LinphoneProxyConfig *cfg, LinphoneRegistrationState state, const string &message) {
	auto listenersCopy = listeners; // Allow removal of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onRegistrationStateChanged(cfg, state, message);
}

void CorePrivate::notifyEnteringBackground () {
	L_Q();
	if (isInBackground)
		return;

	ms_message("Core [%p] notify enter background", q);
	isInBackground = true;

#ifdef __ANDROID__
	static_cast<PlatformHelpers *>(L_GET_C_BACK_PTR(q)->platform_helper)->updateNetworkReachability();
#endif

	auto listenersCopy = listeners; // Allow removal of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onEnteringBackground();

	if (q->isFriendListSubscriptionEnabled())
		enableFriendListsSubscription(false);

#if TARGET_OS_IPHONE
	LinphoneCore *lc = L_GET_C_BACK_PTR(q);
	/* Stop the dtmf stream in case it was started.*/
	linphone_core_stop_dtmf_stream(lc);
#endif
}

void CorePrivate::notifyEnteringForeground () {
	L_Q();
	if (!isInBackground)
		return;

	isInBackground = false;

#ifdef __ANDROID__
	static_cast<PlatformHelpers *>(L_GET_C_BACK_PTR(q)->platform_helper)->updateNetworkReachability();
#endif

	LinphoneCore *lc = L_GET_C_BACK_PTR(q);
	LinphoneProxyConfig *lpc = linphone_core_get_default_proxy_config(lc);
	if (lpc && linphone_proxy_config_get_state(lpc) == LinphoneRegistrationFailed) {
		// This is to ensure an app bring to foreground that isn't registered correctly will try to fix that and not show a red registration dot to the user
		linphone_proxy_config_refresh_register(lpc);
	}

	auto listenersCopy = listeners; // Allow removal of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onEnteringForeground();

	if (q->isFriendListSubscriptionEnabled())
		enableFriendListsSubscription(true);

}

belle_sip_main_loop_t *CorePrivate::getMainLoop(){
	L_Q();
	return belle_sip_stack_get_main_loop(static_cast<belle_sip_stack_t*>(q->getCCore()->sal->getStackImpl()));
}

Sal * CorePrivate::getSal(){
	return getPublic()->getCCore()->sal.get();
}

LinphoneCore *CorePrivate::getCCore() const {
	return getPublic()->getCCore();
}

void CorePrivate::doLater(const std::function<void ()> &something){
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

bool CorePrivate::basicToFlexisipChatroomMigrationEnabled()const{
	L_Q();
	return linphone_config_get_bool(linphone_core_get_config(q->getCCore()), "misc", "enable_basic_to_client_group_chat_room_migration", FALSE);
}

CorePrivate::CorePrivate() : authStack(*this) {
}

ToneManager & CorePrivate::getToneManager() {
	if (!toneManager) toneManager = makeUnique<ToneManager>(*getPublic());
	return *toneManager.get();
}

int CorePrivate::ephemeralMessageTimerExpired (void *data, UNUSED(unsigned int revents)) {
	CorePrivate *d = static_cast<CorePrivate *>(data);
	d->stopEphemeralMessageTimer();

	d->handleEphemeralMessages(ms_time(NULL));
	return BELLE_SIP_STOP;
}

void CorePrivate::startEphemeralMessageTimer (time_t expireTime) {
	double time = difftime(expireTime, ::ms_time(NULL));
	unsigned int timeoutValueMs = time>0 ? (unsigned int)time*1000 : 10;
	if (!ephemeralTimer) {
		ephemeralTimer = getPublic()->getCCore()->sal->createTimer(ephemeralMessageTimerExpired, this, timeoutValueMs, "ephemeral message handler");
	} else {
		belle_sip_source_set_timeout_int64(ephemeralTimer, (int64_t)timeoutValueMs);
	}
}

void CorePrivate::stopEphemeralMessageTimer () {
	if (ephemeralTimer) {
		auto core = getPublic()->getCCore();
		if (core && core->sal)
			core->sal->cancelTimer(ephemeralTimer);
		belle_sip_object_unref(ephemeralTimer);
		ephemeralTimer = nullptr;
	}
}

bool CorePrivate::setInputAudioDevice(const shared_ptr<AudioDevice> &audioDevice) {
	L_Q();
	if (audioDevice && ( (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Record)) == 0) ) {
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

	if (static_cast<unsigned int>(q->audioVideoConferenceById.size()) > 0) {
		for (const auto & audioVideoConference : q->audioVideoConferenceById) {
			audioVideoConference.second->getAudioControlInterface()->setInputDevice(audioDevice);
		}
	}

	return applied;
}

bool CorePrivate::setOutputAudioDevice(const shared_ptr<AudioDevice> &audioDevice) {
	L_Q();
	if (audioDevice && ( (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Play)) == 0) ) {
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

	if (static_cast<unsigned int>(q->audioVideoConferenceById.size()) > 0) {
		for (const auto & audioVideoConference : q->audioVideoConferenceById) {
			if (audioVideoConference.second->getAudioControlInterface()) {
				audioVideoConference.second->getAudioControlInterface()->setOutputDevice(audioDevice);
			}
		}
	}

	return applied;
}


void CorePrivate::updateVideoDevice(){
	if (currentCall && currentCall->getState() == CallSession::State::StreamsRunning){
		VideoControlInterface *i = currentCall->getMediaSession()->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
		if (i) i->parametersChanged();
	}
	if (getCCore()->conf_ctx){
		/* There is a local conference.*/
		MediaConference::Conference *conf = MediaConference::Conference::toCpp(getCCore()->conf_ctx);
		VideoControlInterface *i = conf->getVideoControlInterface();
		if (i) i->parametersChanged();
	}
}

// =============================================================================

Core::Core () : Object(*new CorePrivate) {
	L_D();
	d->imee.reset();
#ifdef HAVE_ADVANCED_IM
	xercesc::XMLPlatformUtils::Initialize();
#endif
}

Core::~Core () {
	lInfo() << "Destroying core: " << this;
#ifdef HAVE_ADVANCED_IM
	xercesc::XMLPlatformUtils::Terminate();
#endif
}

shared_ptr<Core> Core::create (LinphoneCore *cCore) {
	// Do not use `make_shared` => Private constructor.
	shared_ptr<Core> core = shared_ptr<Core>(new Core);
	L_SET_CPP_PTR_FROM_C_OBJECT(cCore, core);
	return core;
}

// ---------------------------------------------------------------------------
// Application lifecycle.
// ---------------------------------------------------------------------------

void Core::enterBackground () {
	L_D();
	d->notifyEnteringBackground();
}

void Core::enterForeground () {
	L_D();
	d->notifyEnteringForeground();
}

bool Core::isInBackground () const {
	L_D();
	return d->isInBackground;
}

// ---------------------------------------------------------------------------
// C-Core.
// ---------------------------------------------------------------------------

LinphoneCore *Core::getCCore () const {
	return L_GET_C_BACK_PTR(this);
}

// -----------------------------------------------------------------------------
// Paths.
// -----------------------------------------------------------------------------

string Core::getDataPath () const {
	return Paths::getPath(Paths::Data, static_cast<PlatformHelpers *>(L_GET_C_BACK_PTR(this)->platform_helper)->getPathContext());
}

string Core::getConfigPath () const {
	return Paths::getPath(Paths::Config, static_cast<PlatformHelpers *>(L_GET_C_BACK_PTR(this)->platform_helper)->getPathContext());
}

string Core::getDownloadPath() const {
#ifdef __ANDROID__
	return getPlatformHelpers(getCCore())->getDownloadPath();
#else
	return Paths::getPath(Paths::Download, static_cast<PlatformHelpers *>(L_GET_C_BACK_PTR(this)->platform_helper)->getPathContext());
#endif
}

void Core::setEncryptionEngine (EncryptionEngine *imee) {
	L_D();
	CoreListener *listener = dynamic_cast<CoreListener *>(getEncryptionEngine());
	if (listener) {
		d->unregisterListener(listener);
	}
	d->imee.reset(imee);
}

EncryptionEngine *Core::getEncryptionEngine () const {
	L_D();
	const auto & imee = d->imee;
	if (imee) {
		return imee.get();
	}
	return nullptr;
}

void Core::enableLimeX3dh (bool enable) {
	L_D();
	
	if (!enable) {
		if (d->imee != nullptr) {
			CoreListener *listener = dynamic_cast<CoreListener *>(getEncryptionEngine());
			if (listener) {
				d->unregisterListener(listener);
			}
			d->imee.reset();
		}
		removeSpec("lime");
		return;
	}
	if (limeX3dhEnabled())
		return;
	
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
				lError() << "Setting x3dh_server_url in section lime is no longer supported. Please use setting lime_server_url under section lime to set the URL of the LIME server globally or in the proxy section of the RC file";
			}
			string dbAccess = getX3dhDbPath();
			belle_http_provider_t *prov = linphone_core_get_http_provider(getCCore());

			LimeX3dhEncryptionEngine *engine = new LimeX3dhEncryptionEngine(dbAccess, prov, getSharedFromThis());
			setEncryptionEngine(engine);
			d->registerListener(engine);
			addSpec("lime");
#else
			lWarning() << "Lime X3DH support is not available";
#endif
		}else{
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
	if (dbAccess.empty()) {
		dbAccess = getDataPath() + "x3dh.c25519.sqlite3";
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

bool Core::limeX3dhEnabled () const {
#ifdef HAVE_LIME_X3DH
	L_D();
	bool isServer = linphone_core_conference_server_enabled(getCCore());
	if (d->imee && ((!isServer && d->imee->getEngineType() == EncryptionEngine::EngineType::LimeX3dh)
					|| (isServer && d->imee->getEngineType() == EncryptionEngine::EngineType::LimeX3dhServer)))
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
void Core::setSpecsList (const std::list<std::string> &specsList) {
	L_D();
	d->specs = specsList;
	d->specs.sort();
	d->specs.unique();
	const string &tmpSpecs = getSpecs();
	LinphoneConfig *lpconfig = linphone_core_get_config(getCCore());
	linphone_config_set_string(lpconfig, "sip", "linphone_specs", tmpSpecs.c_str());
	getCCore()->sal->setContactLinphoneSpecs(tmpSpecs);
}

void Core::addSpec (const std::string &spec) {
	L_D();
	d->specs.push_back(spec);
	setSpecsList(d->specs);
}

void Core::removeSpec(const std::string &pSpec) {
	L_D();
	d->specs.remove_if([&pSpec](const std::string &spec) {
		if (spec.compare(pSpec) == 0) return true;
		/* Check also ignoring the version number after the slash */
		istringstream istr(spec);
		string specWithoutVersion;
		if (std::getline(istr, specWithoutVersion, '/')){
			if (specWithoutVersion == pSpec) return true;
		}
		return false;
	});
	setSpecsList(d->specs);
}

const std::list<std::string> &Core::getSpecsList () const {
	L_D();
	return d->specs;
}

//Used to set specs for linphone_config
void Core::setSpecs (const std::string &pSpecs) {
	L_D();
	if (pSpecs.empty()) {
		d->specs.clear();
		setSpecsList(d->specs);
	} else {
		//Assume a list of coma-separated values
		setSpecsList(Utils::toList(bctoolbox::Utils::split(pSpecs, ",")));
	}
}

//Initial use of the public API of this function has been deprecated, but will still be kept as utility function for setSpecsList()
std::string Core::getSpecs() const {
	L_D();
	return Utils::join(Utils::toVector(d->specs), ",");
}

const std::string Core::conferenceVersionAsString() {
	std::ostringstream os;
	os << CorePrivate::conferenceProtocolVersion;
	return os.str();
}

// ---------------------------------------------------------------------------
// Friends.
// ---------------------------------------------------------------------------

void Core::enableFriendListSubscription (bool enable) {
	L_D();
	linphone_config_set_int(linphone_core_get_config(getCCore()), "net", "friendlist_subscription_enabled", enable ? 1 : 0);
	d->enableFriendListsSubscription(enable);
}

bool Core::isFriendListSubscriptionEnabled () const {
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
		audioDevices.push_back((new AudioDevice(card))->toSharedPtr() );
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
				if (!bluetoothMicFound && (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Record))) {
					lAudioDevices.push_back(audioDevice);
				} else if (!bluetoothSpeakerFound && (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Play))) {
					lAudioDevices.push_back(audioDevice);
				}

				// Do not allow to be set to false
				// Not setting flags inside if statement in order to handle the case of a bluetooth device that can record and play sound
				if (!bluetoothMicFound) bluetoothMicFound = (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Record));
				if (!bluetoothSpeakerFound) bluetoothSpeakerFound = (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Play));
				break;
			case AudioDevice::Type::Headphones:
			case AudioDevice::Type::Headset:
				if (!headsetMicFound && (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Record))) {
					lAudioDevices.push_back(audioDevice);
				} else if (!headsetSpeakerFound && (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Play))) {
					lAudioDevices.push_back(audioDevice);
				}

				// Do not allow to be set to false
				// Not setting flags inside if statement in order to handle the case of a headset/headphones device that can record and play sound
				if (!headsetMicFound) headsetMicFound = (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Record));
				if (!headsetSpeakerFound) headsetSpeakerFound = (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Play));
				break;
			case AudioDevice::Type::HearingAid:
				lAudioDevices.push_back(audioDevice);
				break;
			default:
				break;
		}
		if (micFound && speakerFound && earpieceFound && bluetoothMicFound && bluetoothSpeakerFound && headsetMicFound && headsetSpeakerFound) break;
	}
	return lAudioDevices;
}

list<std::shared_ptr<AudioDevice>> Core::getExtendedAudioDevices() const {
	L_D();
	return d->audioDevices; 
}

void Core::setInputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice) {
	L_D();
	if(getCCore()->use_files) {
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
	if(getCCore()->use_files) {
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
	if (getCCore()->conf_ctx){
		/* There is a local conference.*/
		MediaConference::Conference *conf = MediaConference::Conference::toCpp(getCCore()->conf_ctx);
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
	if (getCCore()->conf_ctx){
		/* There is a local conference.*/
		MediaConference::Conference *conf = MediaConference::Conference::toCpp(getCCore()->conf_ctx);
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

void Core::setOutputAudioDeviceBySndCard(MSSndCard *card){
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
			lInfo() << "[ " << __func__ << " ] on device matching playback capture card: " << audioDevice->getDeviceName();
			d->setOutputAudioDevice(audioDevice);
			return;
		}
	} else { // No cards available : remove the device. This will allow to restart it if a new one is detected.
		lInfo() << "[ " << __func__ << " ] remove output device";
		d->setOutputAudioDevice(nullptr);
	}
	if(card)// Having no device when a card is requested is an error
		lError() << "[ " << __func__ << " ] Unable to find suitable output audio device";
}

void Core::setInputAudioDeviceBySndCard(MSSndCard *card){
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
			lInfo() << "[ " << __func__ << " ] on device matching default capture card: " << audioDevice->getDeviceName();
			d->setInputAudioDevice(audioDevice);
			return;
		}
	}else {// No cards available : remove the device. This will allow to restart it if a new one is detected.
		lInfo() << "[ " << __func__ << " ] remove input device";
		d->setInputAudioDevice(nullptr);
	}
	if (card) // Having no device when a card is requested is an error
		lError() << "[ " << __func__ << " ] Unable to find suitable input audio device";
}



std::shared_ptr<AudioDevice> Core::getDefaultInputAudioDevice() const {
	if(getCCore()->use_files)
		return nullptr;
	else{
		MSSndCard *card = getCCore()->sound_conf.capt_sndcard;
		return findAudioDeviceMatchingMsSoundCard(card);
	}
}

std::shared_ptr<AudioDevice> Core::getDefaultOutputAudioDevice() const {
	if(getCCore()->use_files)
		return nullptr;
	else{
		MSSndCard *card = getCCore()->sound_conf.play_sndcard;
		return findAudioDeviceMatchingMsSoundCard(card);
	}
}

// -----------------------------------------------------------------------------
// Misc.
// -----------------------------------------------------------------------------

/*
 * pushNotificationReceived() is a critical piece of code.
 * When receiving a push notification, we must be absolutely sure that our connections to the SIP servers is up, running and reliable.
 * If not, we must start or restart them.
 */
void Core::pushNotificationReceived (const string& callId, const string& payload, bool isCoreStarting) {
	L_D();

	lInfo() << "Push notification received for Call-ID [" << callId << "]";
	// Stop any previous background task we might already have
	d->pushReceivedBackgroundTask.stop();

	bool found = false;
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
			d->lastPushReceivedCallId = callId;
			// Start a background task for 20 seconds to ensure we have time to process the push
			d->pushReceivedBackgroundTask.start(getSharedFromThis(), 20);
		}
	}

	LinphoneCore *lc = getCCore();
	linphone_core_notify_push_notification_received(lc, payload.c_str());

	if (isCoreStarting) {
		lInfo() << "Core is starting, skipping network tasks that ensures sockets are alive";
		return;
	}
	if (found) {
		lInfo() << "Call-ID was found, skipping network tasks that ensures sockets are alive";
		return;
	}

#ifdef __ANDROID__
	if (linphone_core_wifi_only_enabled(lc)) {
		// If WiFi only policy enabled, check that we are on a WiFi network, otherwise don't handle the push
		bool_t isWifiOnlyCompliant = static_cast<PlatformHelpers *>(lc->platform_helper)->isActiveNetworkWifiOnlyCompliant();
		if (!isWifiOnlyCompliant) {
			lError() << "Android Platform Helpers says current network isn't compliant with WiFi only policy, aborting push notification processing!";
			d->pushReceivedBackgroundTask.stop();
 			return;
		}
	}
#endif

	// We can assume network should be reachable when a push notification is received.
	// If the app was put in DOZE mode, internal network reachability will have been disabled and thus may prevent registration.
	linphone_core_set_network_reachable_internal(lc, TRUE);

	const bctbx_list_t *proxies = linphone_core_get_proxy_config_list(lc);
	bctbx_list_t *it = (bctbx_list_t *)proxies;

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
	while (it) {
		LinphoneProxyConfig *proxy = (LinphoneProxyConfig *) bctbx_list_get_data(it);
		LinphoneRegistrationState state = linphone_proxy_config_get_state(proxy);
		if (state == LinphoneRegistrationFailed) {
			lInfo() << "Proxy config [" << proxy << "] is in failed state, refreshing REGISTER";
			if (linphone_proxy_config_register_enabled(proxy) && linphone_proxy_config_get_expires(proxy) > 0) {
				linphone_proxy_config_refresh_register(proxy);
			}
		} else if (state == LinphoneRegistrationOk) {
			// Send a keep-alive to ensure the socket isn't broken
			sendKeepAlive = true;
		}
		it = bctbx_list_next(it);
	}
	/* Send a "\r\n" keepalive. If the socket is broken, it will generate an error. */
	if (sendKeepAlive) {
		lInfo() << "Sending keep-alive to ensure sockets aren't broken";
		getCCore()->sal->sendKeepAlive();
		linphone_core_iterate(lc); //Let the socket error be caught.
		linphone_core_iterate(lc); // Let the socket error be notified to the refreshers, to restart a connection if needed.
	}
	/*
	 * Despite all the things done so far, there can still be the case where some connections are "ready" but in fact stalled,
	 * due to crappy firewalls not notifying reset connections. Eliminate them.
	 */
	if (d->calls.empty()){
		/* We do this only if there is no running call. Indeed, breaking the connection on which the call was made is problematic, as we are going to loose
		 * the local contact of the dialog. As of today, liblinphone doesn't trigger a reINVITE upon loss of underlying connection.
		 * It does this only upon network manager decisions (linphone_core_set_network_reachable_internal()).
		 * This looks an acceptable compromise, as the socket is not expected to be zombified while keepalives are regularly sent while the app is in foreground.
		 */
		lc->sal->cleanUnreliableConnections();
	}
	linphone_core_iterate(lc); // Let the disconnections be notified to the refreshers.
}

int Core::getUnreadChatMessageCount () const {
	L_D();
	return d->mainDb->getUnreadChatMessageCount();
}

int Core::getUnreadChatMessageCount (const IdentityAddress &localAddress) const {
	L_D();
	int count = 0;
	auto addressToCompare = localAddress.asAddress();
	for (auto it = d->chatRoomsById.begin(); it != d->chatRoomsById.end(); it++) {
		const auto &chatRoom = it->second;
		if (addressToCompare.weakEqual(chatRoom->getLocalAddress().asAddress()))
			count += chatRoom->getUnreadChatMessageCount();
	}
	return count;
}

int Core::getUnreadChatMessageCountFromActiveLocals () const {
	L_D();

	int count = 0;
	for (auto it = d->chatRoomsById.begin(); it != d->chatRoomsById.end(); it++) {
		const auto &chatRoom = it->second;
		for (auto it = linphone_core_get_proxy_config_list(getCCore()); it != NULL; it = it->next) {
			LinphoneProxyConfig *cfg = (LinphoneProxyConfig *)it->data;
			const LinphoneAddress *identityAddr = linphone_proxy_config_get_identity_address(cfg);
			if (L_GET_CPP_PTR_FROM_C_OBJECT(identityAddr)->weakEqual(chatRoom->getLocalAddress().asAddress())) {
				count += chatRoom->getUnreadChatMessageCount();
			}
		}
	}
	return count;
}

std::shared_ptr<PushNotificationMessage> Core::getPushNotificationMessage (const std::string &callId) const {
	std::shared_ptr<PushNotificationMessage> msg = getPlatformHelpers(getCCore())->getSharedCoreHelpers()->getPushNotificationMessage(callId);
	if (linphone_core_get_global_state(getCCore()) == LinphoneGlobalOn && getPlatformHelpers(getCCore())->getSharedCoreHelpers()->isCoreStopRequired()) {
		lInfo() << "[SHARED] Executor Shared Core is beeing stopped by Main Shared Core";
		linphone_core_stop(getCCore());
#if TARGET_OS_IPHONE
		if (!msg && callId != "dummy_call_id") {
			ms_message("[push] Executor Core for callId[%s] has been stopped but couldn't get the msg in time. Trying to get the msg received by the Main Core that just started", callId.c_str());
			auto platformHelpers = LinphonePrivate::createIosPlatformHelpers(getCCore()->cppPtr, NULL);
			msg = platformHelpers->getSharedCoreHelpers()->getPushNotificationMessage(callId);
			delete platformHelpers;
		}
#endif
	}
	return msg;
}

std::shared_ptr<ChatRoom> Core::getPushNotificationChatRoom (const std::string &chatRoomAddr) const {
	std::shared_ptr<ChatRoom> chatRoom = getPlatformHelpers(getCCore())->getSharedCoreHelpers()->getPushNotificationChatRoom(chatRoomAddr);
	return chatRoom;
}

std::shared_ptr<ChatMessage> Core::findChatMessageFromCallId (const std::string &callId) const {
	L_D();
	std::list<std::shared_ptr<ChatMessage>> chatMessages = d->mainDb->findChatMessagesFromCallId(callId);
	return chatMessages.empty() ? nullptr : chatMessages.front();
}

// -----------------------------------------------------------------------------
// Ldap.
// -----------------------------------------------------------------------------

const std::list<std::shared_ptr<Ldap>> & Core::getLdapList() {
	return getPrivate()->mLdapServers;
}

std::list<std::shared_ptr<Ldap>>::iterator Core::getLdapIterator(int index){
	return std::find_if( getPrivate()->mLdapServers.begin(), getPrivate()->mLdapServers.end(), [index](std::shared_ptr<Ldap> a){
		return a->getIndex() == index;
	});
}

void CorePrivate::reloadLdapList() {
	std::list<std::shared_ptr<Ldap>> ldapList;
	auto lpConfig = linphone_core_get_config(getCCore());
	bctbx_list_t * bcSections = linphone_config_get_sections_names_list(lpConfig);
	// Loop on all sections and load configuration. If this is not a LDAP configuration, the model is discarded.
	for(auto itSections = bcSections; itSections; itSections=itSections->next) {
		std::string section = static_cast<char *>(itSections->data);
		std::shared_ptr<Ldap> ldap = Ldap::create(getPublic()->getSharedFromThis(), section);
		if( ldap)
			ldapList.push_back(ldap);
	}
	if( bcSections)
		bctbx_list_free(bcSections);
	ldapList.sort([](std::shared_ptr<Ldap> a, std::shared_ptr<Ldap> b) {
		return a->getIndex() < b->getIndex();
	});
	mLdapServers = ldapList;
}

void Core::addLdap(std::shared_ptr<Ldap> ldap) {
	if(ldap->getLdapParams()) {
		ldap->writeToConfigFile();
		auto itLdapStored = getLdapIterator(ldap->getIndex());
		if(itLdapStored == getPrivate()->mLdapServers.end()) {	// New
			getPrivate()->mLdapServers.push_back(ldap);
			getPrivate()->mLdapServers.sort([](std::shared_ptr<Ldap> a, std::shared_ptr<Ldap> b) {
				return a->getIndex() < b->getIndex();
			});
		}else{	// Update
			*itLdapStored = ldap;
		}
	}
}

void Core::removeLdap(std::shared_ptr<Ldap> ldap){
	auto itLdapStored = getLdapIterator(ldap->getIndex());
	if(itLdapStored != getPrivate()->mLdapServers.end()){
		getPrivate()->mLdapServers.erase(itLdapStored);
		ldap->removeFromConfigFile();
	}
}
// -----------------------------------------------------------------------------

Address Core::interpretUrl (const std::string &url, bool chatOrCallUse) const {
	bool applyPrefix = true;
	if (chatOrCallUse) {
		LinphoneAccount *account = linphone_core_get_default_account(getCCore());
		if (account) {
			const LinphoneAccountParams *params = linphone_account_get_params(account);
			applyPrefix = linphone_account_params_get_use_international_prefix_for_calls_and_chats(params);
		}
	}

	LinphoneAddress *cAddress = linphone_core_interpret_url_2(getCCore(), url.c_str(), applyPrefix);
	if (!cAddress) return Address();

	char *str = linphone_address_as_string(cAddress);
	linphone_address_unref(cAddress);

	Address address(str);
	bctbx_free(str);

	return address;
}

void Core::doLater(const std::function<void ()> &something){
	getPrivate()->doLater(something);
}

void Core::performOnIterateThread(const std::function<void ()> &something){
	unsigned long currentThreadId = bctbx_thread_self();
	if (currentThreadId == getCCore()->iterate_thread_id) {
		something();
	} else {
		doLater(something);
	}
}

belle_sip_source_t *Core::createTimer(const std::function<bool ()> &something, unsigned int milliseconds, const string &name){
	return belle_sip_main_loop_create_cpp_timeout_2(getPrivate()->getMainLoop(), something, (unsigned)milliseconds, name.c_str());
}

/* Stop and destroy a timer created by createTimer()*/
void Core::destroyTimer(belle_sip_source_t *timer){
	belle_sip_source_cancel(timer);
	belle_sip_object_unref(timer);
}

const ConferenceId Core::prepareConfereceIdForSearch(const ConferenceId & conferenceId) const {
	Address peerAddress = conferenceId.getPeerAddress().asAddress();
	peerAddress.removeUriParam("gr");
	Address localAddress = conferenceId.getLocalAddress().asAddress();
	localAddress.removeUriParam("gr");
	ConferenceId prunedConferenceId = ConferenceId(ConferenceAddress(peerAddress), ConferenceAddress(localAddress));
	return prunedConferenceId;
}

std::shared_ptr<MediaConference::Conference> Core::findAudioVideoConference (const ConferenceId &conferenceId, bool logIfNotFound) const {

	ConferenceId prunedConferenceId = prepareConfereceIdForSearch(conferenceId);

	auto it = audioVideoConferenceById.find(prunedConferenceId);
	if (it != audioVideoConferenceById.cend()) {
		lInfo() << "Found audio video conference in RAM with conference ID " << conferenceId << ".";
		return it->second;
	}

	if (logIfNotFound)
		lInfo() << "Unable to find audio video conference with conference ID " << conferenceId << " in RAM.";
	return nullptr;
}

void Core::insertAudioVideoConference (const shared_ptr<MediaConference::Conference> &audioVideoConference) {
	L_ASSERT(audioVideoConference);

	const ConferenceId &conferenceId = audioVideoConference->getConferenceId();

	ConferenceId prunedConferenceId = prepareConfereceIdForSearch(conferenceId);
	auto conf = findAudioVideoConference(prunedConferenceId);

	// Conference does not exist or yes but with the same pointer!
	L_ASSERT(conf == nullptr || conf == audioVideoConference);
	if (conf == nullptr) {
		lInfo() << "Insert audio video conference " << audioVideoConference << " in RAM with conference ID " << conferenceId << ".";
		audioVideoConferenceById[prunedConferenceId] = audioVideoConference;
	}
}

void Core::deleteAudioVideoConference(const shared_ptr<const MediaConference::Conference> &audioVideoConference) {
	const ConferenceId &conferenceId = audioVideoConference->getConferenceId();
	ConferenceId prunedConferenceId = prepareConfereceIdForSearch(conferenceId);

	auto it = audioVideoConferenceById.find(prunedConferenceId);

	if (it != audioVideoConferenceById.cend()) {
		lInfo() << "Delete audio video conference in RAM with conference ID " << conferenceId << ".";
		audioVideoConferenceById.erase(it);
	}

}

shared_ptr<MediaConference::Conference> Core::searchAudioVideoConference(const shared_ptr<ConferenceParams> &params, const ConferenceAddress &localAddress, const ConferenceAddress &remoteAddress, const std::list<IdentityAddress> &participants) const {

	const auto it = std::find_if (audioVideoConferenceById.begin(), audioVideoConferenceById.end(), [&] (const auto & p) {
		// p is of type std::pair<ConferenceId, std::shared_ptr<MediaConference::Conference>
		const auto &audioVideoConference = p.second;
		const ConferenceId &conferenceId = audioVideoConference->getConferenceId();
		const auto &curLocalAddress = conferenceId.getLocalAddress();
		if (localAddress.getAddressWithoutGruu() != curLocalAddress.getAddressWithoutGruu())
			return false;
		const auto &curRemoteAddress = conferenceId.getPeerAddress();
		if (remoteAddress.isValid() && remoteAddress.getAddressWithoutGruu() != curRemoteAddress.getAddressWithoutGruu())
			return false;

		// Check parameters only if pointer provided as argument is not null
		if (params) {
			const ConferenceParams confParams = audioVideoConference->getCurrentParams();
			if (!params->getSubject().empty() && (params->getSubject().compare(confParams.getSubject()) != 0))
				return false;
			if (params->chatEnabled() != confParams.chatEnabled())
				return false;
			if (params->audioEnabled() != confParams.audioEnabled())
				return false;
			if (params->videoEnabled() != confParams.videoEnabled())
				return false;
			if (params->localParticipantEnabled() != confParams.localParticipantEnabled())
				return false;
		}

		// Check participants only if list provided as argument is not empty
		bool participantListMatch = true;
		if (participants.empty() == false) {
			const std::list<std::shared_ptr<Participant>> & confParticipants = audioVideoConference->getParticipants ();
			participantListMatch = equal(participants.cbegin(), participants.cend(), confParticipants.cbegin(), confParticipants.cend(), [] (const auto & p1, const auto & p2) {
				return (p2->getAddress().getAddressWithoutGruu() == p1.getAddressWithoutGruu());
			});
		}
		return participantListMatch;
	});

	shared_ptr<MediaConference::Conference> conference = nullptr;
	if (it != audioVideoConferenceById.cend()) {
		conference = it->second;
	}

	return conference;
}

shared_ptr<MediaConference::Conference> Core::searchAudioVideoConference(const ConferenceAddress &conferenceAddress) const {

	if (!conferenceAddress.isValid()) return nullptr;
	const auto it = std::find_if (audioVideoConferenceById.begin(), audioVideoConferenceById.end(), [&] (const auto & p) {
		// p is of type std::pair<ConferenceId, std::shared_ptr<MediaConference::Conference>
		const auto &audioVideoConference = p.second;
		const auto curConferenceAddress = audioVideoConference->getConferenceAddress();
		return (conferenceAddress == curConferenceAddress);
	});

	shared_ptr<MediaConference::Conference> conference = nullptr;
	if (it != audioVideoConferenceById.cend()) {
		conference = it->second;
	}

	return conference;
}

shared_ptr<CallSession> Core::createConferenceOnServer(const shared_ptr<ConferenceParams> &confParams, const Address &localAddr, const std::list<IdentityAddress> &participants) {
	return createOrUpdateConferenceOnServer(confParams, localAddr, participants, ConferenceAddress());
}

shared_ptr<CallSession> Core::createOrUpdateConferenceOnServer(const std::shared_ptr<ConferenceParams> &confParams, const Address &localAddr, const std::list<IdentityAddress> &participants, const ConferenceAddress &confAddr) {
	L_D()
	if (!confParams) {
		lWarning() << "Trying to create conference with null parameters";
		return nullptr;
	}

	LinphoneCore *lc = L_GET_C_BACK_PTR(this);
	auto params = linphone_core_create_call_params(lc, nullptr);

	Address conferenceFactoryUri;
	if (confAddr == ConferenceAddress()) {
		LinphoneAddress *factoryUri = Core::getAudioVideoConferenceFactoryAddress(getSharedFromThis(), localAddr);
		if (factoryUri == nullptr) {
			lWarning() << "Not creating conference: no conference factory uri for local address [" << localAddr << "]";
			return nullptr;
		}
		auto conferenceFactoryUriStr = linphone_address_as_string_uri_only(factoryUri);
		linphone_address_unref(factoryUri);
		conferenceFactoryUri = Address(conferenceFactoryUriStr);
		ms_free(conferenceFactoryUriStr);
	} else {
		conferenceFactoryUri = confAddr.asAddress();
	}

	ConferenceId conferenceId = ConferenceId(IdentityAddress(), localAddr);
	if (!localAddr.hasUriParam("gr")) {
		lWarning() << "Local identity address [" << localAddr << "] doesn't have a gruu, let's try to find it";
		IdentityAddress localAddrWithGruu = d->getIdentityAddressWithGruu(localAddr);
		if (localAddrWithGruu.isValid()) {
			lInfo() << "Found matching contact address [" << localAddrWithGruu << "] to use instead";
			conferenceId = ConferenceId(IdentityAddress(), localAddrWithGruu);
		} else {
			lError() << "Failed to find matching contact address with gruu for identity address [" << localAddr << "], client group chat room creation will fail!";
		}
	}

	// Participant with the focus call is admin
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->addCustomContactParameter("admin", Utils::toString(true));
	auto addressesList(participants);

	addressesList.sort();
	addressesList.unique();

	if (!addressesList.empty()) {
		Content content;
		content.setBodyFromUtf8(Utils::getResourceLists(addressesList));
		content.setContentType(ContentType::ResourceLists);
		content.setContentDisposition(ContentDisposition::RecipientList);
		if (linphone_core_content_encoding_supported(lc, "deflate")) {
			content.setContentEncoding("deflate");
		}

		L_GET_CPP_PTR_FROM_C_OBJECT(params)->addCustomContent(content);
	}
	linphone_call_params_set_start_time(params, confParams->getStartTime());
	linphone_call_params_set_end_time(params, confParams->getEndTime());
	linphone_call_params_enable_video(params, confParams->videoEnabled());
	linphone_call_params_set_description(params, L_STRING_TO_C(confParams->getDescription()));
	linphone_call_params_set_conference_creation(params, TRUE);

	auto participant = Participant::create(nullptr, localAddr);
	auto session = dynamic_pointer_cast<MediaSession>(participant->createSession(getSharedFromThis(), L_GET_CPP_PTR_FROM_C_OBJECT(params), (confParams->audioEnabled() || confParams->videoEnabled()), nullptr));

	if (!session) {
		lWarning() << "Cannot create conference with subject [" << confParams->getSubject() <<"]";
		return nullptr;
	}

	linphone_call_params_unref(params);

	Address meCleanedAddress(localAddr);
	meCleanedAddress.removeUriParam("gr"); // Remove gr parameter for INVITE.
	session->configure(LinphoneCallOutgoing, nullptr, nullptr, meCleanedAddress, conferenceFactoryUri);
	session->enableToneIndications(false);
	LinphoneProxyConfig *destProxy = session->getDestProxy();
	const LinphoneNatPolicy *natPolicy = nullptr;
	if (destProxy){
		natPolicy = linphone_proxy_config_get_nat_policy(destProxy);
	}
	if (!natPolicy){
		natPolicy = linphone_core_get_nat_policy(getCCore());
	}
	if (natPolicy){
		LinphoneNatPolicy *newNatPolicy = linphone_nat_policy_clone(natPolicy);
		// remove stun server asynchronous gathering, we don't actually need it and it looses some time.
		linphone_nat_policy_enable_stun(newNatPolicy, false);
		session->setNatPolicy(newNatPolicy);
		linphone_nat_policy_unref(newNatPolicy);
	}
	session->initiateOutgoing();
	session->startInvite(nullptr, confParams->getSubject(), nullptr);
	return session;
}

bool Core::incompatibleSecurity(const std::shared_ptr<SalMediaDescription> &md) const {
	LinphoneCore *lc = L_GET_C_BACK_PTR(this);
	return linphone_core_is_media_encryption_mandatory(lc) && linphone_core_get_media_encryption(lc)==LinphoneMediaEncryptionSRTP && !md->hasSrtp();
}

const std::list<LinphoneMediaEncryption> Core::getSupportedMediaEncryptions() const {
	LinphoneCore *lc = L_GET_C_BACK_PTR(this);
	std::list<LinphoneMediaEncryption> encEnumList;
	const auto encList = linphone_core_get_supported_media_encryptions(lc);
	for(const bctbx_list_t * enc = encList;enc!=NULL;enc=enc->next){
		encEnumList.push_back(static_cast<LinphoneMediaEncryption>(LINPHONE_PTR_TO_INT(bctbx_list_get_data(enc))));
	}
	return encEnumList;
}

LinphoneAddress* Core::getAudioVideoConferenceFactoryAddress(const std::shared_ptr<Core> &core, const IdentityAddress &localAddress) {
	Address addr(localAddress.asAddress());
	LinphoneAccount *account = linphone_core_lookup_known_account(core->getCCore(), L_GET_C_BACK_PTR(&addr));

	if (!account) {
		lWarning() << "No account found for local address: [" << localAddress.asString() << "]";
		return nullptr;
	}else
		return getAudioVideoConferenceFactoryAddress(core, account);
}

LinphoneAddress* Core::getAudioVideoConferenceFactoryAddress(const std::shared_ptr<Core> &core, const LinphoneAccount * account) {
	const LinphoneAddress *address = Account::toCpp(account)->getAccountParams()->getAudioVideoConferenceFactoryAddress();
	if (address == nullptr) {
		string conferenceFactoryUri = getConferenceFactoryUri(core, account);
		lWarning() << "Audio/video conference factory is null, fallback to default conference factory URI [" << conferenceFactoryUri << "]";
		if (conferenceFactoryUri.empty()) return nullptr;
		return linphone_address_new(conferenceFactoryUri.c_str());
	}
	char * address_uri = linphone_address_as_string_uri_only(address);
	LinphoneAddress * factory_address = linphone_address_new(address_uri);
	ms_free(address_uri);
	return factory_address;
}

LINPHONE_END_NAMESPACE
