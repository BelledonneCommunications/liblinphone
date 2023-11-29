/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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
#include "chat/chat-room/chat-room-p.h"
#include "core/core-listener.h"
#include "core/core-p.h"
#include "factory/factory.h"
#include "ldap/ldap.h"
#include "linphone/lpconfig.h"
#include "linphone/utils/algorithm.h"
#include "linphone/utils/utils.h"
#include "logger/logger.h"
#include "paths/paths.h"

#include "conference.h"
#include "conference/params/media-session-params-p.h"
#include "conference/participant.h"
#include "conference/session/media-session-p.h"
#include "conference/session/media-session.h"
#include "conference/session/streams.h"

#include "sal/sal_media_description.h"

// TODO: Remove me later.
#include "c-wrapper/c-wrapper.h"
#include "private.h"
#include <utils/payload-type-handler.h>

#define LINPHONE_DB "linphone.db"
#define LINPHONE_CALL_HISTORY_DB "call-history.db"
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

LINPHONE_BEGIN_NAMESPACE

const Utils::Version CorePrivate::conferenceProtocolVersion(1, 0);
const Utils::Version CorePrivate::groupChatProtocolVersion(1, 2);
const Utils::Version CorePrivate::ephemeralProtocolVersion(1, 1);

const std::string Core::limeSpec("lime");

typedef void (*init_func_t)(LinphoneCore *core);

void CorePrivate::init() {
	L_Q();

	q->initPlugins();

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

	if (linphone_factory_is_database_storage_available(linphone_factory_get())) {
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
			string dbPath = Utils::quoteStringIfNotAlready(q->getDataPath() + LINPHONE_DB);
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
		} else lWarning() << "Database explicitely not requested, this Core is built with no database support.";

		// Leave this part to import the legacy call logs to MainDB
		string calHistoryDbPath = L_C_TO_STRING(
		    linphone_config_get_string(linphone_core_get_config(lc), "storage", "call_logs_db_uri", nullptr));
		if (calHistoryDbPath.empty()) calHistoryDbPath = q->getDataPath() + LINPHONE_CALL_HISTORY_DB;
		if (calHistoryDbPath != "null") {
			lInfo() << "Using [" << calHistoryDbPath << "] as legacy call history database path";
			linphone_core_set_call_logs_database_path(lc, calHistoryDbPath.c_str());
		} else lWarning() << "Call logs database explicitely not requested";

		if (lc->zrtp_secrets_cache == NULL) {
			string zrtpSecretsDbPath = L_C_TO_STRING(
			    linphone_config_get_string(linphone_core_get_config(lc), "storage", "zrtp_secrets_db_uri", nullptr));
			if (zrtpSecretsDbPath.empty()) zrtpSecretsDbPath = q->getDataPath() + LINPHONE_ZRTP_SECRETS_DB;
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

void CorePrivate::registerListener(CoreListener *listener) {
	listeners.push_back(listener);
}

void CorePrivate::unregisterListener(CoreListener *listener) {
	listeners.remove(listener);
}

void CorePrivate::writeNatPolicyConfigurations() {
	int index = 0;
	LinphoneCore *lc = getCCore();

	if (!linphone_core_ready(lc)) return;

	LinphoneConfig *config = linphone_core_get_config(lc);

	if (lc->nat_policy) {
		NatPolicy::toCpp(lc->nat_policy)->saveToConfig(config, index);
		++index;
	}
	const bctbx_list_t *elem;
	for (elem = linphone_core_get_account_list(lc); elem != nullptr; elem = elem->next) {
		LinphoneAccount *account = (LinphoneAccount *)elem->data;
		auto natPolicy = Account::toCpp(account)->getAccountParams()->getNatPolicy();
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
		if (chatRoom && (chatRoom->getPrivate()->getImdnHandler()->isCurrentlySendingImdnMessages() ||
		                 !chatRoom->getPrivate()->getTransientChatMessages().empty())) {
			return false;
		}
	}

	return true;
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
	static_cast<PlatformHelpers *>(getCCore()->platform_helper)->stopPushService();
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
			cr->getPrivate()->getImdnHandler()->onLinphoneCoreStop();
#ifdef HAVE_ADVANCED_IM
			for (const auto &participant : cr->getParticipants()) {
				for (std::shared_ptr<ParticipantDevice> device : participant->getDevices()) {
					// to make sure no more messages are received after Core:uninit because key components like DB are
					// no longuer available. So it's no more possible to handle any singnaling messages properly.
					if (device->getSession()) device->getSession()->setListener(nullptr);
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
	static_cast<PlatformHelpers *>(getCCore()->platform_helper)->stopPushService();
	mLdapServers.clear();

	q->mPublishByEtag.clear();

#ifdef HAVE_ADVANCED_IM
	remoteListEventHandler.reset();
	localListEventHandler.reset();
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

// -----------------------------------------------------------------------------

void CorePrivate::notifyGlobalStateChanged(LinphoneGlobalState state) {
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
	L_Q();
	if (isInBackground) return;

	ms_message("Core [%p] notify enter background", q);
	isInBackground = true;

#ifdef __ANDROID__
	static_cast<PlatformHelpers *>(L_GET_C_BACK_PTR(q)->platform_helper)->updateNetworkReachability();
#endif

	auto listenersCopy = listeners; // Allow removal of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onEnteringBackground();

	if (q->isFriendListSubscriptionEnabled()) enableFriendListsSubscription(false);

#if TARGET_OS_IPHONE
	LinphoneCore *lc = L_GET_C_BACK_PTR(q);
	/* Stop the dtmf stream in case it was started.*/
	linphone_core_stop_dtmf_stream(lc);
#endif
}

void CorePrivate::notifyEnteringForeground() {
	L_Q();
	if (!isInBackground) return;

	isInBackground = false;

#ifdef __ANDROID__
	static_cast<PlatformHelpers *>(L_GET_C_BACK_PTR(q)->platform_helper)->updateNetworkReachability();
#endif

	LinphoneCore *lc = L_GET_C_BACK_PTR(q);
	LinphoneProxyConfig *lpc = linphone_core_get_default_proxy_config(lc);
	if (lpc && linphone_proxy_config_get_state(lpc) == LinphoneRegistrationFailed) {
		// This is to ensure an app bring to foreground that isn't registered correctly will try to fix that and not
		// show a red registration dot to the user
		linphone_proxy_config_refresh_register(lpc);
	}

	auto listenersCopy = listeners; // Allow removal of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onEnteringForeground();

	if (q->isFriendListSubscriptionEnabled()) enableFriendListsSubscription(true);
}

belle_sip_main_loop_t *CorePrivate::getMainLoop() {
	L_Q();
	return belle_sip_stack_get_main_loop(static_cast<belle_sip_stack_t *>(q->getCCore()->sal->getStackImpl()));
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

bool CorePrivate::basicToFlexisipChatroomMigrationEnabled() const {
	L_Q();
	return linphone_config_get_bool(linphone_core_get_config(q->getCCore()), "misc",
	                                "enable_basic_to_client_group_chat_room_migration", FALSE);
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
	L_Q();
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

	if (static_cast<unsigned int>(q->audioVideoConferenceById.size()) > 0) {
		for (const auto &audioVideoConference : q->audioVideoConferenceById) {
			audioVideoConference.second->getAudioControlInterface()->setInputDevice(audioDevice);
		}
	}

	return applied;
}

bool CorePrivate::setOutputAudioDevice(const shared_ptr<AudioDevice> &audioDevice) {
	L_Q();
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

	if (static_cast<unsigned int>(q->audioVideoConferenceById.size()) > 0) {
		for (const auto &audioVideoConference : q->audioVideoConferenceById) {
			if (audioVideoConference.second->getAudioControlInterface()) {
				audioVideoConference.second->getAudioControlInterface()->setOutputDevice(audioDevice);
			}
		}
	}

	return applied;
}

void CorePrivate::updateVideoDevice() {
	if (currentCall && currentCall->getState() == CallSession::State::StreamsRunning) {
		VideoControlInterface *i =
		    currentCall->getMediaSession()->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(
		        SalVideo);
		if (i) i->parametersChanged();
	}
	if (getCCore()->conf_ctx) {
		/* There is a local conference.*/
		MediaConference::Conference *conf = MediaConference::Conference::toCpp(getCCore()->conf_ctx);
		VideoControlInterface *i = conf->getVideoControlInterface();
		if (i) i->parametersChanged();
	}
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
	d->notifyEnteringBackground();
}

void Core::enterForeground() {
	L_D();
	d->notifyEnteringForeground();
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
			belle_http_provider_t *prov = linphone_core_get_http_provider(getCCore());
			/* Both Lime and MainDb use soci as database engine.
			 * However, the initialisation of backends must be done manually on platforms where soci is statically
			 * linked. Lime does not do it. Doing it twice is risky - soci unloads the previously loaded backend. A
			 * compromise is to have a single point (AbstractChatRoom::registerBackend()) where it is done safely.
			 * AbstractChatRoom::registerBackend() will do the job only once.
			 */
			AbstractDb::registerBackend(AbstractDb::Sqlite3);
			LimeX3dhEncryptionEngine *engine = new LimeX3dhEncryptionEngine(dbAccess, prov, getSharedFromThis());
			setEncryptionEngine(engine);
			d->registerListener(engine);
			if (!hasSpec(Core::limeSpec)) {
				addSpec(Core::limeSpec);
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
	if (getCCore()->conf_ctx) {
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
	// Stop any previous background task we might already have
	d->pushReceivedBackgroundTask.stop();

	LinphoneCore *lc = getCCore();
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
			static_cast<PlatformHelpers *>(lc->platform_helper)->startPushService();
			// Start a background task for 20 seconds to ensure we have time to process the push
			d->pushReceivedBackgroundTask.start(getSharedFromThis(), 20);
		}
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

	const bctbx_list_t *accounts = linphone_core_get_account_list(lc);
	bctbx_list_t *it = (bctbx_list_t *)accounts;

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
		LinphoneAccount *account = (LinphoneAccount *)bctbx_list_get_data(it);
		const LinphoneAccountParams *params = linphone_account_get_params(account);
		if (AccountParams::toCpp(params)->getForceRegisterOnPushNotification()) {
			lInfo() << "Account [" << account
			        << "] is configured to force a REGISTER when a push is received, doing it now";
			linphone_account_refresh_register(account);
			it = bctbx_list_next(it);
			continue;
		}

		LinphoneRegistrationState state = linphone_account_get_state(account);
		if (state == LinphoneRegistrationFailed) {
			lInfo() << "Account [" << account << "] is in failed state, refreshing REGISTER";
			if (linphone_account_params_register_enabled(params) && linphone_account_params_get_expires(params) > 0) {
				linphone_account_refresh_register(account);
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
	return d->mainDb->getUnreadChatMessageCount();
}

int Core::getUnreadChatMessageCount(const std::shared_ptr<Address> &localAddress) const {
	L_D();
	int count = 0;
	for (auto it = d->chatRoomsById.begin(); it != d->chatRoomsById.end(); it++) {
		const auto &chatRoom = it->second;
		if (localAddress->weakEqual(*chatRoom->getLocalAddress())) {
			if (!chatRoom->getIsMuted()) {
				count += chatRoom->getUnreadChatMessageCount();
			}
		}
	}
	return count;
}

int Core::getUnreadChatMessageCountFromActiveLocals() const {
	L_D();

	int count = 0;
	for (auto it = d->chatRoomsById.begin(); it != d->chatRoomsById.end(); it++) {
		const auto &chatRoom = it->second;
		for (auto it = linphone_core_get_proxy_config_list(getCCore()); it != NULL; it = it->next) {
			LinphoneProxyConfig *cfg = (LinphoneProxyConfig *)it->data;
			const LinphoneAddress *identityAddr = linphone_proxy_config_get_identity_address(cfg);
			if (Address::toCpp(identityAddr)->weakEqual(*chatRoom->getLocalAddress())) {
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

// -----------------------------------------------------------------------------
// Ldap.
// -----------------------------------------------------------------------------

const std::list<std::shared_ptr<Ldap>> &Core::getLdapList() {
	return getPrivate()->mLdapServers;
}

std::list<std::shared_ptr<Ldap>>::iterator Core::getLdapIterator(int index) {
	return std::find_if(getPrivate()->mLdapServers.begin(), getPrivate()->mLdapServers.end(),
	                    [index](std::shared_ptr<Ldap> a) { return a->getIndex() == index; });
}

void CorePrivate::reloadLdapList() {
	std::list<std::shared_ptr<Ldap>> ldapList;
	auto lpConfig = linphone_core_get_config(getCCore());
	bctbx_list_t *bcSections = linphone_config_get_sections_names_list(lpConfig);
	// Loop on all sections and load configuration. If this is not a LDAP configuration, the model is discarded.
	for (auto itSections = bcSections; itSections; itSections = itSections->next) {
		std::string section = static_cast<char *>(itSections->data);
		std::shared_ptr<Ldap> ldap = Ldap::create(getPublic()->getSharedFromThis(), section);
		if (ldap) ldapList.push_back(ldap);
	}
	if (bcSections) bctbx_list_free(bcSections);
	ldapList.sort([](std::shared_ptr<Ldap> a, std::shared_ptr<Ldap> b) { return a->getIndex() < b->getIndex(); });
	mLdapServers = ldapList;
}

void Core::addLdap(std::shared_ptr<Ldap> ldap) {
	if (ldap->getLdapParams()) {
		ldap->writeToConfigFile();
		auto itLdapStored = getLdapIterator(ldap->getIndex());
		if (itLdapStored == getPrivate()->mLdapServers.end()) { // New
			getPrivate()->mLdapServers.push_back(ldap);
			getPrivate()->mLdapServers.sort(
			    [](std::shared_ptr<Ldap> a, std::shared_ptr<Ldap> b) { return a->getIndex() < b->getIndex(); });
		} else { // Update
			*itLdapStored = ldap;
		}
	}
}

void Core::removeLdap(std::shared_ptr<Ldap> ldap) {
	auto itLdapStored = getLdapIterator(ldap->getIndex());
	if (itLdapStored != getPrivate()->mLdapServers.end()) {
		getPrivate()->mLdapServers.erase(itLdapStored);
		ldap->removeFromConfigFile();
	}
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

	std::shared_ptr<Address> address = Address::toCpp(cAddress)->getSharedFromThis();
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
	return belle_sip_main_loop_create_cpp_timeout_2(getPrivate()->getMainLoop(), something, (unsigned)milliseconds,
	                                                name.c_str());
}

/* Stop and destroy a timer created by createTimer()*/
void Core::destroyTimer(belle_sip_source_t *timer) {
	belle_sip_source_cancel(timer);
	belle_sip_object_unref(timer);
}

const ConferenceId Core::prepareConfereceIdForSearch(const ConferenceId &conferenceId) const {
	std::shared_ptr<Address> peerAddress = nullptr;
	const auto &rawPeerAddress = conferenceId.getPeerAddress();
	if (rawPeerAddress) {
		peerAddress = Address::create(rawPeerAddress->getUriWithoutGruu());
		peerAddress->removeUriParam(Conference::SecurityModeParameter);
	}
	std::shared_ptr<Address> localAddress = nullptr;
	const auto &rawLocalAddress = conferenceId.getLocalAddress();
	if (rawLocalAddress) {
		localAddress = Address::create(rawLocalAddress->getUriWithoutGruu());
		localAddress->removeUriParam(Conference::SecurityModeParameter);
	}
	ConferenceId prunedConferenceId = ConferenceId(peerAddress, localAddress);
	return prunedConferenceId;
}

std::shared_ptr<MediaConference::Conference> Core::findAudioVideoConference(const ConferenceId &conferenceId,
                                                                            bool logIfNotFound) const {
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

void Core::insertAudioVideoConference(const shared_ptr<MediaConference::Conference> audioVideoConference) {
	L_ASSERT(audioVideoConference);

	const ConferenceId &conferenceId = audioVideoConference->getConferenceId();

	ConferenceId prunedConferenceId = prepareConfereceIdForSearch(conferenceId);
	auto conf = findAudioVideoConference(prunedConferenceId);

	// Conference does not exist or yes but with the same pointer!
	L_ASSERT(conf == nullptr || conf == audioVideoConference);
	if (conf == nullptr) {
		lInfo() << "Insert audio video conference " << audioVideoConference << " in RAM with conference ID "
		        << conferenceId << ".";
		audioVideoConferenceById.insert(std::make_pair(prunedConferenceId, audioVideoConference));
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

shared_ptr<MediaConference::Conference>
Core::searchAudioVideoConference(const shared_ptr<ConferenceParams> &params,
                                 const std::shared_ptr<const Address> &localAddress,
                                 const std::shared_ptr<const Address> &remoteAddress,
                                 const std::list<std::shared_ptr<Address>> &participants) const {

	auto localAddressUri = (localAddress) ? localAddress->getUriWithoutGruu() : Address();
	auto remoteAddressUri = (remoteAddress) ? remoteAddress->getUriWithoutGruu() : Address();
	const auto it = std::find_if(audioVideoConferenceById.begin(), audioVideoConferenceById.end(), [&](const auto &p) {
		// p is of type std::pair<ConferenceId, std::shared_ptr<MediaConference::Conference>
		const auto &audioVideoConference = p.second;
		const ConferenceId &conferenceId = audioVideoConference->getConferenceId();

		auto curLocalAddress =
		    (conferenceId.getLocalAddress()) ? conferenceId.getLocalAddress()->getUriWithoutGruu() : Address();
		if (localAddressUri.isValid() && (localAddressUri != curLocalAddress)) return false;

		auto curPeerAddress =
		    (conferenceId.getPeerAddress()) ? conferenceId.getPeerAddress()->getUriWithoutGruu() : Address();
		if (remoteAddressUri.isValid() && (remoteAddressUri != curPeerAddress)) return false;

		// Check parameters only if pointer provided as argument is not null
		if (params) {
			const ConferenceParams confParams = audioVideoConference->getCurrentParams();
			if (!params->getSubject().empty() && (params->getSubject().compare(confParams.getSubject()) != 0))
				return false;
			if (params->chatEnabled() != confParams.chatEnabled()) return false;
			if (params->audioEnabled() != confParams.audioEnabled()) return false;
			if (params->videoEnabled() != confParams.videoEnabled()) return false;
			if (params->localParticipantEnabled() != confParams.localParticipantEnabled()) return false;
		}

		// Check participants only if list provided as argument is not empty
		bool participantListMatch = true;
		if (participants.empty() == false) {
			const std::list<std::shared_ptr<Participant>> &confParticipants = audioVideoConference->getParticipants();
			participantListMatch =
			    equal(participants.cbegin(), participants.cend(), confParticipants.cbegin(), confParticipants.cend(),
			          [](const auto &p1, const auto &p2) { return (p1->weakEqual(*p2->getAddress())); });
		}
		return participantListMatch;
	});

	shared_ptr<MediaConference::Conference> conference = nullptr;
	if (it != audioVideoConferenceById.cend()) {
		conference = it->second;
	}

	return conference;
}

shared_ptr<MediaConference::Conference>
Core::searchAudioVideoConference(const std::shared_ptr<Address> &conferenceAddress) const {

	if (!conferenceAddress || !conferenceAddress->isValid()) return nullptr;
	const auto it = std::find_if(audioVideoConferenceById.begin(), audioVideoConferenceById.end(), [&](const auto &p) {
		// p is of type std::pair<ConferenceId, std::shared_ptr<MediaConference::Conference>
		const auto &audioVideoConference = p.second;
		const auto curConferenceAddress = audioVideoConference->getConferenceAddress();
		return (*conferenceAddress == *curConferenceAddress);
	});

	shared_ptr<MediaConference::Conference> conference = nullptr;
	if (it != audioVideoConferenceById.cend()) {
		conference = it->second;
	}

	return conference;
}

shared_ptr<CallSession> Core::createConferenceOnServer(const shared_ptr<ConferenceParams> &confParams,
                                                       const std::shared_ptr<Address> &localAddr,
                                                       const std::list<std::shared_ptr<Address>> &participants) {
	return createOrUpdateConferenceOnServer(confParams, localAddr, participants, nullptr);
}

shared_ptr<CallSession> Core::createOrUpdateConferenceOnServer(const std::shared_ptr<ConferenceParams> &confParams,
                                                               const std::shared_ptr<Address> &localAddr,
                                                               const std::list<std::shared_ptr<Address>> &participants,
                                                               const std::shared_ptr<Address> &confAddr) {
	L_D()
	if (!confParams) {
		lWarning() << "Trying to create conference with null parameters";
		return nullptr;
	}

	LinphoneCore *lc = L_GET_C_BACK_PTR(this);
	auto params = linphone_core_create_call_params(lc, nullptr);

	std::shared_ptr<Address> conferenceFactoryUri;
	if (confAddr) {
		conferenceFactoryUri = confAddr;
	} else {
		conferenceFactoryUri = Core::getAudioVideoConferenceFactoryAddress(getSharedFromThis(), localAddr);
		if (!conferenceFactoryUri || !conferenceFactoryUri->isValid()) {
			lWarning() << "Not creating conference: no conference factory uri for local address [" << *localAddr << "]";
			return nullptr;
		}
		conferenceFactoryUri->setUriParam(Conference::SecurityModeParameter,
		                                  ConferenceParams::getSecurityLevelAttribute(confParams->getSecurityLevel()));
	}

	ConferenceId conferenceId = ConferenceId(nullptr, localAddr);
	if (!localAddr->hasUriParam("gr")) {
		lWarning() << "Local identity address [" << *localAddr << "] doesn't have a gruu, let's try to find it";
		auto localAddrWithGruu = d->getIdentityAddressWithGruu(localAddr);
		if (localAddrWithGruu && localAddrWithGruu->isValid()) {
			lInfo() << "Found matching contact address [" << localAddrWithGruu << "] to use instead";
			conferenceId = ConferenceId(nullptr, localAddrWithGruu);
		} else {
			lError() << "Failed to find matching contact address with gruu for identity address ["
			         << localAddr->toString() << "], client group chat room creation will fail!";
		}
	}

	// Participant with the focus call is admin
	L_GET_CPP_PTR_FROM_C_OBJECT(params)->addCustomContactParameter("admin", Utils::toString(true));
	auto addressesList(participants);

	addressesList.sort();
	addressesList.unique();

	if (!addressesList.empty()) {
		auto content = Content::create();
		content->setBodyFromUtf8(Utils::getResourceLists(addressesList));
		content->setContentType(ContentType::ResourceLists);
		content->setContentDisposition(ContentDisposition::RecipientList);
		if (linphone_core_content_encoding_supported(lc, "deflate")) {
			content->setContentEncoding("deflate");
		}

		L_GET_CPP_PTR_FROM_C_OBJECT(params)->addCustomContent(content);
	}
	linphone_call_params_set_start_time(params, confParams->getStartTime());
	linphone_call_params_set_end_time(params, confParams->getEndTime());
	linphone_call_params_enable_video(params, confParams->videoEnabled());
	linphone_call_params_set_description(params, L_STRING_TO_C(confParams->getDescription()));
	linphone_call_params_set_conference_creation(params, TRUE);
	linphone_call_params_enable_tone_indications(params, FALSE);

	auto participant = Participant::create(nullptr, localAddr);
	auto session = dynamic_pointer_cast<MediaSession>(
	    participant->createSession(getSharedFromThis(), L_GET_CPP_PTR_FROM_C_OBJECT(params),
	                               (confParams->audioEnabled() || confParams->videoEnabled()), nullptr));

	if (!session) {
		lWarning() << "Cannot create conference with subject [" << confParams->getSubject() << "]";
		return nullptr;
	}

	linphone_call_params_unref(params);

	std::shared_ptr<Address> meCleanedAddress = Address::create(localAddr->getUriWithoutGruu());
	session->configure(LinphoneCallOutgoing, nullptr, nullptr, meCleanedAddress, conferenceFactoryUri);
	const auto destAccount = session->getPrivate()->getDestAccount();
	std::shared_ptr<NatPolicy> natPolicy = nullptr;
	if (destAccount) {
		const auto accountParams = destAccount->getAccountParams();
		natPolicy = accountParams->getNatPolicy();
	}
	if (!natPolicy) {
		natPolicy = NatPolicy::toCpp(linphone_core_get_nat_policy(getCCore()))->getSharedFromThis();
	}
	if (natPolicy) {
		auto newNatPolicy = natPolicy->clone()->toSharedPtr();
		// remove stun server asynchronous gathering, we don't actually need it and it looses some time.
		newNatPolicy->enableStun(false);
		session->setNatPolicy(newNatPolicy);
	}
	session->initiateOutgoing();
	session->startInvite(nullptr, confParams->getSubject(), nullptr);
	return session;
}

bool Core::incompatibleSecurity(const std::shared_ptr<SalMediaDescription> &md) const {
	LinphoneCore *lc = L_GET_C_BACK_PTR(this);
	return linphone_core_is_media_encryption_mandatory(lc) &&
	       linphone_core_get_media_encryption(lc) == LinphoneMediaEncryptionSRTP && !md->hasSrtp();
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

const std::shared_ptr<Address>
Core::getAudioVideoConferenceFactoryAddress(const std::shared_ptr<Core> &core,
                                            const std::shared_ptr<Address> &localAddress) {
	std::shared_ptr<Address> addr = localAddress;
	LinphoneAccount *account = linphone_core_lookup_known_account(core->getCCore(), addr->toC());

	if (!account) {
		lWarning() << "No account found for local address: [" << *localAddress << "]";
		return nullptr;
	} else return getAudioVideoConferenceFactoryAddress(core, Account::toCpp(account)->getSharedFromThis());
}

const std::shared_ptr<Address> Core::getAudioVideoConferenceFactoryAddress(const std::shared_ptr<Core> &core,
                                                                           const std::shared_ptr<Account> account) {
	const auto &address = account->getAccountParams()->getAudioVideoConferenceFactoryAddress();
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
	szDirPath.append("\\lib*.dll");
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
		    (strstr(de->d_name, "lib") == de->d_name) && ((ext = strstr(de->d_name, LINPHONE_PLUGINS_EXT)) != NULL)) {
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
	bool changed = policy != d->videoCodecPriorityPolicy;
	d->videoCodecPriorityPolicy = policy;
	if (changed) d->reorderVideoCodecList();
}

LinphoneCodecPriorityPolicy Core::getVideoCodecPriorityPolicy() const {
	L_D();
	return d->videoCodecPriorityPolicy;
}

LINPHONE_END_NAMESPACE
