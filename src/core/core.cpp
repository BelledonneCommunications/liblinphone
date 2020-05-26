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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

#include <algorithm>
#include <iterator>

#include <mediastreamer2/mscommon.h>

#ifdef HAVE_ADVANCED_IM
#include <xercesc/util/PlatformUtils.hpp>
#endif

#include "address/address-p.h"
#include "call/call.h"
#include "chat/encryption/encryption-engine.h"
#ifdef HAVE_LIME_X3DH
#include "chat/encryption/lime-x3dh-encryption-engine.h"
#endif
#ifdef HAVE_ADVANCED_IM
#include "conference/handlers/local-conference-list-event-handler.h"
#include "conference/handlers/remote-conference-list-event-handler.h"
#endif
#include "core/core-listener.h"
#include "core/core-p.h"
#include "chat/chat-room/chat-room-p.h"
#include "logger/logger.h"
#include "paths/paths.h"
#include "linphone/utils/utils.h"
#include "linphone/utils/algorithm.h"
#include "linphone/lpconfig.h"

#include "call/call-p.h"
#include "conference/session/media-session.h"
#include "conference/session/streams.h"
#include "conference_private.h"

// TODO: Remove me later.
#include "c-wrapper/c-wrapper.h"
#include "private.h"

#define LINPHONE_DB "linphone.db"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

void CorePrivate::init () {
	L_Q();

	mainDb.reset(new MainDb(q->getSharedFromThis()));
#ifdef HAVE_ADVANCED_IM
	remoteListEventHandler = makeUnique<RemoteConferenceListEventHandler>(q->getSharedFromThis());
	localListEventHandler = makeUnique<LocalConferenceListEventHandler>(q->getSharedFromThis());
#endif

	if (linphone_factory_is_database_storage_available(linphone_factory_get())) {
		AbstractDb::Backend backend;
		string uri = L_C_TO_STRING(lp_config_get_string(linphone_core_get_config(L_GET_C_BACK_PTR(q)), "storage", "uri", nullptr));
		if (!uri.empty())
			backend = strcmp(lp_config_get_string(linphone_core_get_config(L_GET_C_BACK_PTR(q)), "storage", "backend", "sqlite3"), "mysql") == 0
				? MainDb::Mysql
				: MainDb::Sqlite3;
		else {
			backend = AbstractDb::Sqlite3;
			uri = q->getDataPath() + LINPHONE_DB;
		}

		if (uri != "null"){ //special uri "null" means don't open database. We need this for tests.
			if (backend == MainDb::Mysql && uri.find("charset") == string::npos) {
				lInfo() << "No charset defined forcing utf8 4 bytes specially for conference subjet storage";
				uri += " charset=utf8mb4";
			}
			lInfo() << "Opening linphone database " << uri << " with backend " << backend;
			if (!mainDb->connect(backend, uri)) {
				ostringstream os;
				os << "Unable to open linphone database with uri " << uri << " and backend " << backend;
				throw DatabaseConnectionFailure(os.str());
			}

			loadChatRooms();
		} else lWarning() << "Database explicitely not requested, this Core is built with no database support.";
	}

	isFriendListSubscriptionEnabled = !!lp_config_get_int(linphone_core_get_config(L_GET_C_BACK_PTR(q)), "net", "friendlist_subscription_enabled", 1);
}

void CorePrivate::registerListener (CoreListener *listener) {
	listeners.push_back(listener);
}

void CorePrivate::unregisterListener (CoreListener *listener) {
	listeners.remove(listener);
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

	const list<shared_ptr<AbstractChatRoom>> chatRooms = q->getChatRooms();
	shared_ptr<ChatRoom> cr;
	for (const auto &chatRoom : chatRooms) {
		cr = dynamic_pointer_cast<ChatRoom>(chatRoom);
		if (cr && cr->getPrivate()->getImdnHandler()->hasUndeliveredImdnMessage()) {
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
	
	for (auto &audioDevice : audioDevices) {
		audioDevice->unref();
	}
	audioDevices.clear();

	if (toneManager) toneManager->deleteTimer();

	stopEphemeralMessageTimer();
	ephemeralMessages.clear();
}

// Called by _linphone_core_stop_async_end() just before going to globalStateOff.
// Put here the data that need to be freed before the stop.
void CorePrivate::uninit() {
	L_Q();

	const list<shared_ptr<AbstractChatRoom>> chatRooms = q->getChatRooms();
	shared_ptr<ChatRoom> cr;
	for (const auto &chatRoom : chatRooms) {
		cr = dynamic_pointer_cast<ChatRoom>(chatRoom);
		if (cr) cr->getPrivate()->getImdnHandler()->onLinphoneCoreStop();
	}

	chatRoomsById.clear();
	noCreatedClientGroupChatRooms.clear();
	listeners.clear();
	if (q->limeX3dhEnabled()) {
		q->enableLimeX3dh(false);
	}

#ifdef HAVE_ADVANCED_IM
	remoteListEventHandler = nullptr;
	localListEventHandler = nullptr;
#endif

	AddressPrivate::clearSipAddressesCache();
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

void CorePrivate::notifyRegistrationStateChanged (LinphoneProxyConfig *cfg, LinphoneRegistrationState state, const string &message) {
	auto listenersCopy = listeners; // Allow removal of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onRegistrationStateChanged(cfg, state, message);
}

void CorePrivate::notifyEnteringBackground () {
	if (isInBackground)
		return;

	isInBackground = true;
	auto listenersCopy = listeners; // Allow removal of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onEnteringBackground();

	if (isFriendListSubscriptionEnabled)
		enableFriendListsSubscription(false);
}

void CorePrivate::notifyEnteringForeground () {
	L_Q();
	if (!isInBackground)
		return;

	isInBackground = false;

	LinphoneCore *lc = L_GET_C_BACK_PTR(q);
	LinphoneProxyConfig *lpc = linphone_core_get_default_proxy_config(lc);
	if (lpc && linphone_proxy_config_get_state(lpc) == LinphoneRegistrationFailed) {
		// This is to ensure an app bring to foreground that isn't registered correctly will try to fix that and not show a red registration dot to the user
		linphone_proxy_config_refresh_register(lpc);
	}

	auto listenersCopy = listeners; // Allow removal of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onEnteringForeground();

	if (isFriendListSubscriptionEnabled)
		enableFriendListsSubscription(true);	
}

belle_sip_main_loop_t *CorePrivate::getMainLoop(){
	L_Q();
	return belle_sip_stack_get_main_loop(static_cast<belle_sip_stack_t*>(q->getCCore()->sal->getStackImpl()));
}

Sal * CorePrivate::getSal(){
	return getPublic()->getCCore()->sal;
}

LinphoneCore *CorePrivate::getCCore(){
	return getPublic()->getCCore();
}

void CorePrivate::doLater(const std::function<void ()> &something){
	belle_sip_main_loop_cpp_do_later(getMainLoop(), something);
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

CorePrivate::CorePrivate() : authStack(*this){
}

std::shared_ptr<ToneManager> CorePrivate::getToneManager() {
	L_Q();
	if (!toneManager) {
		toneManager = make_shared<ToneManager>(q->getSharedFromThis());
	}
	return toneManager;
}

int CorePrivate::ephemeralMessageTimerExpired (void *data, unsigned int revents) {
	CorePrivate *d = static_cast<CorePrivate *>(data);
	d->stopEphemeralMessageTimer();

	d->handleEphemeralMessages(ms_time(NULL));
	return BELLE_SIP_STOP;
}

void CorePrivate::startEphemeralMessageTimer (time_t expireTime) {
	double time = difftime(expireTime, ::ms_time(NULL));
	unsigned int timeoutValueMs = time>0 ? (unsigned int)time*1000 : 10;
	if (!timer) {
		timer = getPublic()->getCCore()->sal->createTimer(ephemeralMessageTimerExpired, this, timeoutValueMs, "ephemeral message handler");
	} else {
		belle_sip_source_set_timeout(timer, timeoutValueMs);
	}
}

void CorePrivate::stopEphemeralMessageTimer () {
	if (timer) {
		auto core = getPublic()->getCCore();
		if (core && core->sal)
			core->sal->cancelTimer(timer);
		belle_sip_object_unref(timer);
		timer = nullptr;
	}
}

bool CorePrivate::setInputAudioDevice(AudioDevice *audioDevice) {
	if ((audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Record)) == 0) {
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

	return applied;
}

bool CorePrivate::setOutputAudioDevice(AudioDevice *audioDevice) {
	if ((audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Play)) == 0) {
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

	return applied;
}


void CorePrivate::updateVideoDevice(){
	if (currentCall && currentCall->getState() == CallSession::State::StreamsRunning){
		VideoControlInterface *i = currentCall->getPrivate()->getMediaSession()->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
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

bool Core::isInBackground () {
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
	return Paths::getPath(Paths::Download, static_cast<PlatformHelpers *>(L_GET_C_BACK_PTR(this)->platform_helper)->getPathContext());
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
	return d->imee.get();
}

void Core::enableLimeX3dh (bool enable) {
#ifdef HAVE_LIME_X3DH
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
		LinphoneConfig *lpconfig = linphone_core_get_config(getCCore());
		string serverUrl = lp_config_get_string(lpconfig, "lime", "lime_server_url", lp_config_get_string(lpconfig, "lime", "x3dh_server_url", ""));
		if (serverUrl.empty()) {
			lInfo() << "Lime X3DH server URL not set, can't enable";
			//Do not enable encryption engine if url is undefined
			return;
		}
		string dbAccess = lp_config_get_string(lpconfig, "lime", "x3dh_db_path", "");
		if (dbAccess.empty()) {
			dbAccess = getDataPath() + "x3dh.c25519.sqlite3";
		}
		belle_http_provider_t *prov = linphone_core_get_http_provider(getCCore());

		LimeX3dhEncryptionEngine *engine = new LimeX3dhEncryptionEngine(dbAccess, serverUrl, prov, getSharedFromThis());
		setEncryptionEngine(engine);
		d->registerListener(engine);
		addSpec("lime");
	}
#else
	lWarning() << "Lime X3DH support is not available";
#endif
}

//Note: this will re-initialise	or start x3dh encryption engine if url is different from existing one
void Core::setX3dhServerUrl(const std::string &url) {
	if (!limeX3dhAvailable()) {
		return;
	}
	LinphoneConfig *lpconfig = linphone_core_get_config(getCCore());
	string prevUrl = lp_config_get_string(lpconfig, "lime", "lime_server_url", lp_config_get_string(lpconfig, "lime", "x3dh_server_url", ""));
	lp_config_set_string(lpconfig, "lime", "lime_server_url", url.c_str());
	lp_config_clean_entry(lpconfig, "lime", "x3dh_server_url");
	if (url.empty()) {
		enableLimeX3dh(false);
	} else if (url.compare(prevUrl)) {
		//Force re-initialisation
		enableLimeX3dh(false);
		enableLimeX3dh(true);
	}
}

std::string Core::getX3dhServerUrl() const {
	LinphoneConfig *lpconfig = linphone_core_get_config(getCCore());
	string serverUrl = lp_config_get_string(lpconfig, "lime", "lime_server_url", lp_config_get_string(lpconfig, "lime", "x3dh_server_url", ""));
	return serverUrl;
}

bool Core::limeX3dhEnabled () const {
#ifdef HAVE_LIME_X3DH
	L_D();
	if (d->imee && d->imee->getEngineType() == EncryptionEngine::EngineType::LimeX3dh)
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
	d->specs.remove_if([&pSpec](const std::string &spec) { return spec.compare(pSpec) == 0; });
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
		setSpecsList(Utils::toList(Utils::split(pSpecs, ",")));
	}
}

//Initial use of the public API of this function has been deprecated, but will still be kept as utility function for setSpecsList()
std::string Core::getSpecs() const {
	L_D();
	return Utils::join(Utils::toVector(d->specs), ",");
}

// ---------------------------------------------------------------------------
// Friends.
// ---------------------------------------------------------------------------

void Core::enableFriendListSubscription (bool enable) {
	L_D();
	if (d->isFriendListSubscriptionEnabled != enable) {
		d->isFriendListSubscriptionEnabled = enable;
		lp_config_set_int(linphone_core_get_config(getCCore()), "net", "friendlist_subscription_enabled", enable ? 1 : 0);
	}
	d->enableFriendListsSubscription(enable);
}

bool Core::isFriendListSubscriptionEnabled () const {
	L_D();
	return d->isFriendListSubscriptionEnabled;
}

// ---------------------------------------------------------------------------
// Audio devices.
// ---------------------------------------------------------------------------

void CorePrivate::computeAudioDevicesList() {
	for (auto &audioDevice : audioDevices) {
		audioDevice->unref();
	}
	audioDevices.clear();

	MSSndCardManager *snd_card_manager = ms_factory_get_snd_card_manager(getCCore()->factory);
	const bctbx_list_t *list = ms_snd_card_manager_get_list(snd_card_manager);

	for (const bctbx_list_t *it = list; it != nullptr; it = bctbx_list_next(it)) {
		MSSndCard *card = static_cast<MSSndCard *>(bctbx_list_get_data(it));
		AudioDevice *audioDevice = new AudioDevice(card);
		audioDevices.push_back(audioDevice);
	}
}

AudioDevice* Core::findAudioDeviceMatchingMsSoundCard(MSSndCard *soundCard) const {
	for (const auto &audioDevice : getExtendedAudioDevices()) {
		if (audioDevice->getSoundCard() == soundCard) {
			return audioDevice;
		}
	}
	return nullptr;
}

const list<AudioDevice *> Core::getAudioDevices() const {
	std::list<AudioDevice *> audioDevices;
	bool micFound = false, speakerFound = false, earpieceFound = false, bluetoothMicFound = false, bluetoothSpeakerFound = false;

	for (const auto &audioDevice : getExtendedAudioDevices()) {
		switch (audioDevice->getType()) {
			case AudioDevice::Type::Microphone:
				if (!micFound) {
					micFound = true;
					audioDevices.push_back(audioDevice);
				}
				break;
			case AudioDevice::Type::Earpiece:
				if (!earpieceFound) {
					earpieceFound = true;
					audioDevices.push_back(audioDevice);
				}
				break;
			case AudioDevice::Type::Speaker:
				if (!speakerFound) {
					speakerFound = true;
					audioDevices.push_back(audioDevice);
				}
				break;
			case AudioDevice::Type::Bluetooth:
				if (!bluetoothMicFound && (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Record))) {
					audioDevices.push_back(audioDevice);
				} else if (!bluetoothSpeakerFound && (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Play))) {
					audioDevices.push_back(audioDevice);
				}

				// Do not allow to be set to false
				// Not setting flags inside if statement in order to handle the case of a bluetooth device that can record and play sound
				if (!bluetoothMicFound) bluetoothMicFound = (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Record));
				if (!bluetoothSpeakerFound) bluetoothSpeakerFound = (audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Play));
				break;
			default:
				break;
		}
		if (micFound && speakerFound && earpieceFound && bluetoothMicFound && bluetoothSpeakerFound) break;
	}
	return audioDevices;
}

const list<AudioDevice *> Core::getExtendedAudioDevices() const {
	L_D();
	return d->audioDevices; 
}

void Core::setInputAudioDevice(AudioDevice *audioDevice) {

	L_D();
	bool success = d->setInputAudioDevice(audioDevice);

	if (success) {
		linphone_core_notify_audio_device_changed(L_GET_C_BACK_PTR(getSharedFromThis()), audioDevice->toC());
	}
}

void Core::setOutputAudioDevice(AudioDevice *audioDevice) {

	L_D();
	bool success = d->setOutputAudioDevice(audioDevice);

	if (success) {
		linphone_core_notify_audio_device_changed(L_GET_C_BACK_PTR(getSharedFromThis()), audioDevice->toC());
	}
}

AudioDevice* Core::getInputAudioDevice() const {
	shared_ptr<LinphonePrivate::Call> call = getCurrentCall();
	if (call) {
		return call->getInputAudioDevice();
	}

	for (const auto &call : getCalls()) {
		return call->getInputAudioDevice();
	}

	return nullptr;
}

AudioDevice* Core::getOutputAudioDevice() const {
	shared_ptr<LinphonePrivate::Call> call = getCurrentCall();
	if (call) {
		return call->getOutputAudioDevice();
	}

	for (const auto &call : getCalls()) {
		return call->getOutputAudioDevice();
	}

	return nullptr;
}

void Core::setDefaultInputAudioDevice(AudioDevice *audioDevice) {
	if ((audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Record)) == 0) {
		lError() << "Audio device [" << audioDevice << "] doesn't have Record capability";
		return;
	}
	linphone_core_set_capture_device(getCCore(), audioDevice->getId().c_str());
}

void Core::setDefaultOutputAudioDevice(AudioDevice *audioDevice) {
	if ((audioDevice->getCapabilities() & static_cast<int>(AudioDevice::Capabilities::Play)) == 0) {
		lError() << "Audio device [" << audioDevice << "] doesn't have Play capability";
		return;
	}
	linphone_core_set_playback_device(getCCore(), audioDevice->getId().c_str());
}

void Core::setOutputAudioDeviceBySndCard(MSSndCard *card){
	L_D();

	if (card) {
		AudioDevice * audioDevice = findAudioDeviceMatchingMsSoundCard(card);
		if (audioDevice) {
			d->setOutputAudioDevice(audioDevice);
			return;
		}
	}
	AudioDevice * defaultAudioDevice = getDefaultOutputAudioDevice();
	if (defaultAudioDevice) {
		d->setOutputAudioDevice(defaultAudioDevice);
		return;
	}
	MSSndCardManager *snd_card_manager = ms_factory_get_snd_card_manager(getCCore()->factory);
	MSSndCard *defaultCard = ms_snd_card_manager_get_default_playback_card(snd_card_manager);
	if (defaultCard) {
		AudioDevice * audioDevice = findAudioDeviceMatchingMsSoundCard(defaultCard);
		if (audioDevice) {
			d->setOutputAudioDevice(audioDevice);
			return;
		}
	}
	lError() << "[ " << __func__ << " ] Unable to find suitable output audio device";
}

void Core::setInputAudioDeviceBySndCard(MSSndCard *card){
	L_D();

	if (card) {
		AudioDevice * audioDevice = findAudioDeviceMatchingMsSoundCard(card);
		if (audioDevice) {
			d->setInputAudioDevice(audioDevice);
			return;
		}
	}
	AudioDevice * defaultAudioDevice = getDefaultInputAudioDevice();
	if (defaultAudioDevice) {
		d->setInputAudioDevice(defaultAudioDevice);
		return;
	}
	MSSndCardManager *snd_card_manager = ms_factory_get_snd_card_manager(getCCore()->factory);
	MSSndCard *defaultCard = ms_snd_card_manager_get_default_capture_card(snd_card_manager);
	if (defaultCard) {
		AudioDevice * audioDevice = findAudioDeviceMatchingMsSoundCard(defaultCard);
		if (audioDevice) {
			d->setInputAudioDevice(audioDevice);
			return;
		}
	}
	lError() << "[ " << __func__ << " ] Unable to find suitable input audio device";
}



AudioDevice* Core::getDefaultInputAudioDevice() const {
	MSSndCard *card = getCCore()->sound_conf.capt_sndcard;
	return findAudioDeviceMatchingMsSoundCard(card);
}

AudioDevice* Core::getDefaultOutputAudioDevice() const {
	MSSndCard *card = getCCore()->sound_conf.play_sndcard;
	return findAudioDeviceMatchingMsSoundCard(card);
}

// -----------------------------------------------------------------------------
// Misc.
// -----------------------------------------------------------------------------

void Core::pushNotificationReceived () const {
	LinphoneCore *lc = getCCore();
	const bctbx_list_t *proxies = linphone_core_get_proxy_config_list(lc);
	bctbx_list_t *it = (bctbx_list_t *)proxies;

	lInfo() << "Push notification received";

	// We can assume network should be reachable when a push notification is received.
	// If the app was put in DOZE mode, internal network reachability will have been disabled and thus may prevent registration 
	linphone_core_set_network_reachable_internal(lc, TRUE);

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
	 * Finally if any of the connection is already pending a retry, the following code will request an immediate
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
	if (sendKeepAlive) {
		lInfo() << "Sending keep-alive to ensure sockets aren't broken";
		getCCore()->sal->sendKeepAlive();
		linphone_core_iterate(lc);
		linphone_core_iterate(lc);
	}
}

int Core::getUnreadChatMessageCount () const {
	L_D();
	return d->mainDb->getUnreadChatMessageCount();
}

int Core::getUnreadChatMessageCount (const IdentityAddress &localAddress) const {
	L_D();
	int count = 0;
	for (auto it = d->chatRoomsById.begin(); it != d->chatRoomsById.end(); it++) {
		const auto &chatRoom = it->second;
		if (chatRoom->getLocalAddress() == localAddress)
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
			if (L_GET_CPP_PTR_FROM_C_OBJECT(identityAddr)->weakEqual(chatRoom->getLocalAddress())) {
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

Address Core::interpretUrl (const std::string &url) const {
	LinphoneAddress *cAddress = linphone_core_interpret_url(getCCore(), url.c_str());
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

belle_sip_source_t *Core::createTimer(const std::function<bool ()> &something, unsigned int milliseconds, const string &name){
	return belle_sip_main_loop_create_cpp_timeout_2(getPrivate()->getMainLoop(), something, (unsigned)milliseconds, name.c_str());
}

/* Stop and destroy a timer created by createTimer()*/
void Core::destroyTimer(belle_sip_source_t *timer){
	belle_sip_source_cancel(timer);
	belle_sip_object_unref(timer);
}


LINPHONE_END_NAMESPACE
