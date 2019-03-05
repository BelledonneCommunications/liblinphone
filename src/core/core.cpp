/*
 * core.cpp
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

#include <algorithm>
#include <iterator>

#include <mediastreamer2/mscommon.h>
#include <xercesc/util/PlatformUtils.hpp>

#include "address/address-p.h"
#include "call/call.h"
#include "chat/encryption/encryption-engine.h"
#ifdef HAVE_LIME_X3DH
#include "chat/encryption/lime-x3dh-encryption-engine.h"
#endif
#include "conference/handlers/local-conference-list-event-handler.h"
#include "conference/handlers/remote-conference-list-event-handler.h"
#include "core/core-listener.h"
#include "core/core-p.h"
#include "logger/logger.h"
#include "paths/paths.h"
#include "linphone/utils/utils.h"
#include "linphone/utils/algorithm.h"
#include "linphone/lpconfig.h"

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
	remoteListEventHandler = makeUnique<RemoteConferenceListEventHandler>(q->getSharedFromThis());
	localListEventHandler = makeUnique<LocalConferenceListEventHandler>(q->getSharedFromThis());

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
		lInfo() << "Opening linphone database " << uri << " with backend " << backend;
		if (!mainDb->connect(backend, uri))
			lFatal() << "Unable to open linphone database with uri " << uri << " and backend " << backend;

		loadChatRooms();
	} else lWarning() << "Database explicitely not requested, this Core is built with no database support.";

	isFriendListSubscriptionEnabled = !!lp_config_get_int(linphone_core_get_config(L_GET_C_BACK_PTR(q)), "net", "friendlist_subscription_enabled", 1);
}

void CorePrivate::registerListener (CoreListener *listener) {
	listeners.push_back(listener);
}

void CorePrivate::unregisterListener (CoreListener *listener) {
	listeners.remove(listener);
}

void CorePrivate::uninit () {
	L_Q();
	while (!calls.empty()) {
		calls.front()->terminate();
		linphone_core_iterate(L_GET_C_BACK_PTR(q));
		ms_usleep(10000);
	}

	chatRooms.clear();
	chatRoomsById.clear();
	noCreatedClientGroupChatRooms.clear();
	listeners.clear();
	if (q->limeX3dhEnabled()) {
		q->enableLimeX3dh(false);
	}

	remoteListEventHandler = nullptr;
	localListEventHandler = nullptr;

	AddressPrivate::clearSipAddressesCache();
	if (mainDb != nullptr) {
		mainDb->disconnect();
	}
}

// -----------------------------------------------------------------------------

void CorePrivate::notifyGlobalStateChanged (LinphoneGlobalState state) {
	auto listenersCopy = listeners; // Allow removable of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onGlobalStateChanged(state);
}

void CorePrivate::notifyNetworkReachable (bool sipNetworkReachable, bool mediaNetworkReachable) {
	auto listenersCopy = listeners; // Allow removable of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onNetworkReachable(sipNetworkReachable, mediaNetworkReachable);
}

void CorePrivate::notifyRegistrationStateChanged (LinphoneProxyConfig *cfg, LinphoneRegistrationState state, const string &message) {
	auto listenersCopy = listeners; // Allow removable of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onRegistrationStateChanged(cfg, state, message);
}

void CorePrivate::notifyEnteringBackground () {
	if (isInBackground)
		return;

	isInBackground = true;
	auto listenersCopy = listeners; // Allow removable of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onEnteringBackground();

	if (isFriendListSubscriptionEnabled)
		enableFriendListsSubscription(false);
}

void CorePrivate::notifyEnteringForeground () {
	if (!isInBackground)
		return;

	isInBackground = false;
	auto listenersCopy = listeners; // Allow removable of a listener in its own call
	for (const auto &listener : listenersCopy)
		listener->onEnteringForeground();

	if (isFriendListSubscriptionEnabled)
		enableFriendListsSubscription(true);	
}

belle_sip_main_loop_t *CorePrivate::getMainLoop(){
	L_Q();
	return belle_sip_stack_get_main_loop(static_cast<belle_sip_stack_t*>(q->getCCore()->sal->getStackImpl()));
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

// =============================================================================

Core::Core () : Object(*new CorePrivate) {
	L_D();
	d->imee.reset();
	xercesc::XMLPlatformUtils::Initialize();
}

Core::~Core () {
	lInfo() << "Destroying core: " << this;
	xercesc::XMLPlatformUtils::Terminate();
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
	return Paths::getPath(Paths::Data, static_cast<PlatformHelpers *>(L_GET_C_BACK_PTR(this)->platform_helper));
}

string Core::getConfigPath () const {
	return Paths::getPath(Paths::Config, static_cast<PlatformHelpers *>(L_GET_C_BACK_PTR(this)->platform_helper));
}

string Core::getDownloadPath() const {
	return Paths::getPath(Paths::Download, static_cast<PlatformHelpers *>(L_GET_C_BACK_PTR(this)->platform_helper));
}

void Core::setEncryptionEngine (EncryptionEngine *imee) {
	L_D();
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
		if (d->imee != nullptr)
			d->imee.release();
		removeSpec("lime");
		return;
	}

	if (limeX3dhEnabled())
		return;

	if (d->imee != nullptr)
		d->imee.release();

	if (d->imee == nullptr) {
		LinphoneConfig *lpconfig = linphone_core_get_config(getCCore());
		string serverUrl = lp_config_get_string(lpconfig, "lime", "x3dh_server_url", "");
		if (serverUrl.empty()) {
			lInfo() << "Lime X3DH server URL not set, can't enable";
			//Do not enable encryption engine if url is undefined
			return;
		}
		string filename = lp_config_get_string(lpconfig, "lime", "x3dh_db_path", "x3dh.c25519.sqlite3");
		string dbAccess = getDataPath() + filename;
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
	string prevUrl = lp_config_get_string(lpconfig, "lime", "x3dh_server_url", "");
	lp_config_set_string(lpconfig, "lime", "x3dh_server_url", url.c_str());
	if (url.empty()) {
		enableLimeX3dh(false);
	} else if (url.compare(prevUrl)) {
		//Force re-initialisation
		enableLimeX3dh(false);
		enableLimeX3dh(true);
	}
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

// -----------------------------------------------------------------------------
// Misc.
// -----------------------------------------------------------------------------

int Core::getUnreadChatMessageCount () const {
	L_D();
	return d->mainDb->getUnreadChatMessageCount();
}

int Core::getUnreadChatMessageCount (const IdentityAddress &localAddress) const {
	L_D();
	int count = 0;
	for (const auto &chatRoom : d->chatRooms)
		if (chatRoom->getLocalAddress() == localAddress)
			count += chatRoom->getUnreadChatMessageCount();
	return count;
}

int Core::getUnreadChatMessageCountFromActiveLocals () const {
	L_D();

	set<IdentityAddress> localAddresses;
	{
		LinphoneAddress *address = linphone_core_get_primary_contact_parsed(getCCore());
		localAddresses.insert(*L_GET_CPP_PTR_FROM_C_OBJECT(address));
		linphone_address_unref(address);
	}

	for (const bctbx_list_t *it = linphone_core_get_proxy_config_list(getCCore()); it; it = bctbx_list_next(it))
		localAddresses.insert(*L_GET_CPP_PTR_FROM_C_OBJECT(static_cast<LinphoneProxyConfig *>(it->data)->identity_address));

	int count = 0;
	for (const auto &chatRoom : d->chatRooms) {
		auto it = localAddresses.find(chatRoom->getLocalAddress());
		if (it != localAddresses.end())
			count += chatRoom->getUnreadChatMessageCount();
	}
	return count;
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


LINPHONE_END_NAMESPACE
