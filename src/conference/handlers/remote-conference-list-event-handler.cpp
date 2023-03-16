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

#include <bctoolbox/defs.h>

#include "linphone/api/c-event.h"
#include "linphone/core.h"
#include "linphone/proxy_config.h"
#include "linphone/utils/utils.h"

#include "account/account.h"
#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "content/content-manager.h"
#include "content/content-type.h"
#include "core/core-p.h"
#include "event/event-subscribe.h"
#include "logger/logger.h"
#include "remote-conference-event-handler.h"
#include "remote-conference-list-event-handler.h"
#include "xml/conference-info.h"
#include "xml/resource-lists.h"
#include "xml/rlmi.h"

// TODO: Remove me later.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

RemoteConferenceListEventHandler::RemoteConferenceListEventHandler(const std::shared_ptr<Core> &core)
    : CoreAccessor(core) {
	getCore()->getPrivate()->registerListener(this);
}

RemoteConferenceListEventHandler::~RemoteConferenceListEventHandler() {
	try {
		getCore()->getPrivate()->unregisterListener(this);
	} catch (const bad_weak_ptr &) {
		// Unable to unregister listener here. Core is destroyed and the listener doesn't exist.
	}

	unsubscribe();
}

// -----------------------------------------------------------------------------

void RemoteConferenceListEventHandler::subscribe() {
	LinphoneCore *lc = getCore()->getCCore();
	const bctbx_list_t *list = linphone_core_get_account_list(lc);

	for (; list != NULL; list = list->next) {
		const auto &account = Account::toCpp((LinphoneAccount *)list->data)->getSharedFromThis();
		subscribe(account);
	}
}

void RemoteConferenceListEventHandler::subscribe(const shared_ptr<Account> &account) {
	unsubscribe(account);

	if (handlers.empty()) return;

	Content content;
	content.setContentType(ContentType::ResourceLists);

	Xsd::ResourceLists::ResourceLists rl = Xsd::ResourceLists::ResourceLists();
	Xsd::ResourceLists::ListType l = Xsd::ResourceLists::ListType();

	const auto &accountParams = account->getAccountParams();
	std::shared_ptr<Address> identityAddress = accountParams->getIdentityAddress();

	for (const auto &p : handlers) {
		RemoteConferenceEventHandler *handler = p.second;
		const ConferenceId &conferenceId = handler->getConferenceId();
		if (identityAddress->weakEqual(*conferenceId.getLocalAddress())) {
			shared_ptr<AbstractChatRoom> cr = getCore()->findChatRoom(conferenceId);
			if (!cr) {
				lError() << "Couldn't add chat room " << conferenceId
				         << " in the chat room list subscription because chat room couldn't be found";
				continue;
			}

			if (cr->hasBeenLeft()) continue;

			Address addr = conferenceId.getPeerAddress()->getUri();
			const auto lastNotify = handler->getLastNotify();
			addr.setUriParam("Last-Notify", Utils::toString(lastNotify));
			handler->setInitialSubscriptionUnderWayFlag((lastNotify == 0));
			Xsd::ResourceLists::EntryType entry = Xsd::ResourceLists::EntryType(addr.asStringUriOnly());
			l.getEntry().push_back(entry);
		}
	}
	rl.getList().push_back(l);

	Xsd::XmlSchema::NamespaceInfomap map;
	stringstream xmlBody;
	serializeResourceLists(xmlBody, rl, map);
	content.setBodyFromUtf8(xmlBody.str());

	if (account->getState() != LinphoneRegistrationOk) return;

	const auto &factoryUri = accountParams->getConferenceFactoryUri();
	if (factoryUri.empty()) {
		lError() << "Couldn't send chat room list subscription for account " << account << " (" << identityAddress
		         << ") because there's no conference factory uri";
		return;
	}

	auto rlsAddr = Address::create(factoryUri);
	auto evSub = dynamic_pointer_cast<EventSubscribe>(
	    (new EventSubscribe(getCore(), rlsAddr, "conference", 600))->toSharedPtr());
	std::string from = account->getContactAddress()->toString();
	evSub->getOp()->setFrom(from);
	evSub->setInternal(true);
	evSub->addCustomHeader("Require", "recipient-list-subscribe");
	evSub->addCustomHeader("Accept", "multipart/related, application/conference-info+xml, application/rlmi+xml");
	evSub->addCustomHeader("Content-Disposition", "recipient-list");
	LinphoneCore *lc = getCore()->getCCore();
	if (linphone_core_content_encoding_supported(lc, "deflate")) {
		content.setContentEncoding("deflate");
		evSub->addCustomHeader("Accept-Encoding", "deflate");
	}
	evSub->setProperty("event-handler-private", this);
	LinphoneContent *cContent = L_GET_C_BACK_PTR(&content);
	evSub->send(cContent);

	levs.push_back(evSub);
}

void RemoteConferenceListEventHandler::unsubscribe() {
	for (auto &evSub : levs) {
		evSub->terminate();
	}
	levs.clear();
}

void RemoteConferenceListEventHandler::unsubscribe(const std::shared_ptr<Account> &account) {
	if (!account || !account->getContactAddress()) return;
	const auto &from = account->getContactAddress();
	auto it = std::find_if(levs.begin(), levs.end(), [from](const auto &lev) { return (*lev->getFrom() == *from); });

	if (it != levs.end()) {
		shared_ptr<EventSubscribe> evSub = *it;
		levs.erase(it);
		evSub->terminate();
	}
}

void RemoteConferenceListEventHandler::invalidateSubscription() {
	levs.clear();
}

bool RemoteConferenceListEventHandler::getInitialSubscriptionUnderWayFlag(const ConferenceId &conferenceId) const {
	auto handler = findHandler(conferenceId);
	return (handler) ? handler->getInitialSubscriptionUnderWayFlag() : false;
}

void RemoteConferenceListEventHandler::notifyReceived(std::string from, const Content *notifyContent) {
	const std::shared_ptr<Address> local = Address::create(from);

	if (notifyContent->getContentType() == ContentType::ConferenceInfo) {
		// Simple notify received directly from a chat-room
		const string &xmlBody = notifyContent->getBodyAsUtf8String();
		istringstream data(xmlBody);
		unique_ptr<Xsd::ConferenceInfo::ConferenceType> confInfo;
		try {
			confInfo = Xsd::ConferenceInfo::parseConferenceInfo(data, Xsd::XmlSchema::Flags::dont_validate);
		} catch (const exception &) {
			lError() << "Error while parsing conference-info in conferences notify";
			return;
		}

		std::shared_ptr<Address> entityAddress = Address::create(confInfo->getEntity().c_str());
		ConferenceId id(entityAddress, local);
		RemoteConferenceEventHandler *handler = findHandler(id);
		if (!handler) return;

		handler->notifyReceived(*notifyContent);
		return;
	}

	list<Content> contents = ContentManager::multipartToContentList(*notifyContent);
	map<string, std::shared_ptr<Address>> addresses;
	for (const auto &content : contents) {
		const string &body = content.getBodyAsUtf8String();
		const ContentType &contentType = content.getContentType();
		if (contentType == ContentType::Rlmi) {
			addresses = parseRlmi(body);
			continue;
		}

		const string &cid = content.getHeader("Content-Id").getValue();
		if (cid.empty()) continue;

		map<string, std::shared_ptr<Address>>::const_iterator it = addresses.find(cid);
		if (it == addresses.cend()) continue;

		std::shared_ptr<Address> peer = it->second;
		ConferenceId id(peer, local);
		RemoteConferenceEventHandler *handler = findHandler(id);
		if (!handler) continue;

		if (contentType == ContentType::Multipart) handler->multipartNotifyReceived(content);
		else if (contentType == ContentType::ConferenceInfo) handler->notifyReceived(content);
	}
}

// -----------------------------------------------------------------------------

RemoteConferenceEventHandler *RemoteConferenceListEventHandler::findHandler(const ConferenceId &conferenceId) const {
	const auto it = handlers.find(conferenceId);
	if (it != handlers.end()) {
		return (*it).second;
	}
	return nullptr;
}

void RemoteConferenceListEventHandler::addHandler(RemoteConferenceEventHandler *handler) {
	if (!handler) {
		lWarning() << "Trying to insert null handler in the remote conference handler list";
		return;
	}

	const ConferenceId &conferenceId = handler->getConferenceId();

	if (!conferenceId.isValid()) {
		lError() << "RemoteConferenceListEventHandler::addHandler invalid handler.";
		return;
	}

	if (!isHandlerInSameDomainAsCore(conferenceId)) {
		return;
	}

	if (findHandler(conferenceId)) {
		lWarning() << "Trying to insert an already present handler in the remote conference handler list: "
		           << conferenceId;
		return;
	}
	handlers[conferenceId] = handler;
}

bool RemoteConferenceListEventHandler::isHandlerInSameDomainAsCore(const ConferenceId &conferenceId) const {
	// Ensure that conference and conference factory are in the same domain
	const std::shared_ptr<Address> &localAddress = conferenceId.getLocalAddress();
	const std::shared_ptr<Address> &peerAddress = conferenceId.getPeerAddress();
	std::shared_ptr<Address> conferenceFactoryUri =
	    Address::create(Core::getConferenceFactoryUri(getCore(), localAddress));

	if (peerAddress->getDomain() != conferenceFactoryUri->getDomain()) {
		lWarning() << "Peer address " << peerAddress->toString()
		           << " is not in the same domain as the conference factory URI " << conferenceFactoryUri->toString()
		           << " hence not adding to the list of subscribes";
		return false;
	}

	return true;
}

void RemoteConferenceListEventHandler::removeHandler(RemoteConferenceEventHandler *handler) {
	const ConferenceId &conferenceId = handler->getConferenceId();
	if (!conferenceId.isValid()) {
		lError() << "RemoteConferenceListEventHandler::removeHandler() invalid handler.";
		return;
	}

	if (!isHandlerInSameDomainAsCore(conferenceId)) {
		return;
	}

	if (handler) {
		auto it = handlers.find(conferenceId);
		if (it != handlers.end() && handler == (*it).second) {
			handlers.erase(it);
			lInfo() << "Handler removed.";
		} else {
			lError() << "Handler not found in RemoteConferenceListEventHandler.";
		}
	} else {
		lError() << "Handler is null !";
	}
}

void RemoteConferenceListEventHandler::clearHandlers() {
	handlers.clear();
}

map<string, std::shared_ptr<Address>> RemoteConferenceListEventHandler::parseRlmi(const string &xmlBody) const {
	istringstream data(xmlBody);
	map<string, std::shared_ptr<Address>> addresses;
	unique_ptr<Xsd::Rlmi::List> rlmi;
	try {
		rlmi = Xsd::Rlmi::parseList(data, Xsd::XmlSchema::Flags::dont_validate);
	} catch (const exception &) {
		lError() << "Error while parsing RLMI in conferences notify";
		return addresses;
	}
	for (const auto &resource : rlmi->getResource()) {
		if (resource.getInstance().empty()) continue;

		const string &uri = string(resource.getUri());
		if (uri.empty()) continue;

		std::shared_ptr<Address> peer = Address::create(uri);
		for (const auto &instance : resource.getInstance()) {
			const string &cid = string(instance.getId());
			if (cid.empty()) continue;

			addresses.emplace(cid, peer);
		}
	}
	return addresses;
}

// -----------------------------------------------------------------------------

void RemoteConferenceListEventHandler::onNetworkReachable(bool sipNetworkReachable,
                                                          BCTBX_UNUSED(bool mediaNetworkReachable)) {
	if (sipNetworkReachable) {
		subscribe();
	} else {
		unsubscribe();
	}
}

void RemoteConferenceListEventHandler::onRegistrationStateChanged(LinphoneProxyConfig *cfg,
                                                                  LinphoneRegistrationState state,
                                                                  BCTBX_UNUSED(const std::string &message)) {
	const auto &account = Account::toCpp(cfg->account)->getSharedFromThis();
	if (state == LinphoneRegistrationOk) subscribe(account);
	else if (state == LinphoneRegistrationCleared) { // On cleared, restart subscription if the cleared proxy config is
		                                             // the current subscription
		const LinphoneAddress *cfgAddress = linphone_proxy_config_get_identity_address(cfg);
		auto it = std::find_if(levs.begin(), levs.end(), [&cfgAddress](const auto &evSub) {
			const auto &currentAddress = evSub->getFrom();
			return currentAddress->weakEqual(*Address::toCpp(cfgAddress));
		});

		if (it != levs.end()) unsubscribe(account);
	}
}

void RemoteConferenceListEventHandler::onEnteringBackground() {
	unsubscribe();
}

void RemoteConferenceListEventHandler::onEnteringForeground() {
	subscribe();
}

LINPHONE_END_NAMESPACE
