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
#include "linphone/utils/utils.h"

#include "account/account.h"
#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "client-conference-event-handler.h"
#include "client-conference-list-event-handler.h"
#include "content/content-manager.h"
#include "content/content-type.h"
#include "core/core-p.h"
#include "event/event-subscribe.h"
#include "logger/logger.h"
#include "xml/conference-info.h"
#include "xml/resource-lists.h"
#include "xml/rlmi.h"

// TODO: Remove me later.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

ClientConferenceListEventHandler::ClientConferenceListEventHandler(const std::shared_ptr<Core> &core)
    : CoreAccessor(core) {
	getCore()->getPrivate()->registerListener(this);
}

ClientConferenceListEventHandler::~ClientConferenceListEventHandler() {
	try {
		getCore()->getPrivate()->unregisterListener(this);
	} catch (const bad_weak_ptr &) {
		// Unable to unregister listener here. Core is destroyed and the listener doesn't exist.
	}

	unsubscribe();
}

// -----------------------------------------------------------------------------
bool ClientConferenceListEventHandler::subscribe() {
	const auto &accounts = getCore()->getAccounts();
	bool ret = !accounts.empty();
	for (const auto &account : accounts) {
		ret &= subscribe(account);
	}
	return ret;
}

bool ClientConferenceListEventHandler::subscribe(const shared_ptr<Account> &account) {
	unsubscribe(account);

	if (handlers.empty()) return false;

	if (account->getState() != LinphoneRegistrationOk) return false;

	const auto &accountParams = account->getAccountParams();
	auto identityAddress = accountParams->getIdentityAddress();

	const auto &factoryUri = accountParams->getConferenceFactoryAddress();
	if (!factoryUri || !factoryUri->isValid()) {
		lError() << "Couldn't send chat room list subscription for " << *account
		         << " because there's no conference factory uri";
		return false;
	}

	auto content = Content::create();
	content->setContentType(ContentType::ResourceLists);

	Xsd::ResourceLists::ResourceLists rl = Xsd::ResourceLists::ResourceLists();
	Xsd::ResourceLists::ListType l = Xsd::ResourceLists::ListType();

	for (const auto &[key, handlerWkPtr] : handlers) {
		try {
			const std::shared_ptr<ClientConferenceEventHandler> handler(handlerWkPtr);
			const ConferenceId &conferenceId = handler->getConferenceId();
			if (identityAddress->weakEqual(*conferenceId.getLocalAddress())) {
				shared_ptr<AbstractChatRoom> cr = getCore()->findChatRoom(conferenceId, false);
				if (!cr) {
					lError() << "Couldn't add chat room " << conferenceId
					         << " in the chat room list subscription because chat room couldn't be found";
					continue;
				}
				if (cr->hasBeenLeft()) continue;

				Address addr = conferenceId.getPeerAddress()->getUri();
				const auto lastNotify = handler->getLastNotify();
				addr.setUriParam("Last-Notify", Utils::toString(lastNotify));
				handler->setInitialSubscriptionUnderWayFlag(handler->notAlreadySubscribed());
				Xsd::ResourceLists::EntryType entry = Xsd::ResourceLists::EntryType(addr.asStringUriOnly());
				l.getEntry().push_back(entry);
				handler->setManagedByListEventhandler(true);
			}
		} catch (const bad_weak_ptr &) {
		}
	}
	rl.getList().push_back(l);

	Xsd::XmlSchema::NamespaceInfomap map;
	stringstream xmlBody;
	serializeResourceLists(xmlBody, rl, map);
	content->setBodyFromUtf8(xmlBody.str());

	auto evSub = dynamic_pointer_cast<EventSubscribe>(
	    (new EventSubscribe(getCore(), factoryUri, "conference", 600))->toSharedPtr());
	evSub->getOp()->setFromAddress(account->getContactAddress()->getImpl());
	evSub->setInternal(true);
	evSub->addCustomHeader("Require", "recipient-list-subscribe");
	evSub->addCustomHeader("Accept", "multipart/related, application/conference-info+xml, application/rlmi+xml");
	evSub->addCustomHeader("Content-Disposition", "recipient-list");
	LinphoneCore *lc = getCore()->getCCore();
	if (linphone_core_content_encoding_supported(lc, "deflate")) {
		content->setContentEncoding("deflate");
		evSub->addCustomHeader("Accept-Encoding", "deflate");
	}
	evSub->setProperty("event-handler-private", this);
	auto ret = evSub->send(content);
	levs.push_back(evSub);

	return (ret == 0);
}

void ClientConferenceListEventHandler::unsubscribe() {
	for (auto &evSub : levs) {
		evSub->terminate();
	}
	levs.clear();

	for (const auto &[key, handlerWkPtr] : handlers) {
		try {
			const std::shared_ptr<ClientConferenceEventHandler> handler(handlerWkPtr);
			handler->setManagedByListEventhandler(false);
		} catch (const bad_weak_ptr &) {
		}
	}
}

void ClientConferenceListEventHandler::unsubscribe(const std::shared_ptr<Account> &account) {
	if (!account || !account->getContactAddress()) return;
	const auto &from = account->getContactAddress();
	auto it = std::find_if(levs.begin(), levs.end(), [from](const auto &lev) { return (*lev->getFrom() == *from); });

	if (it != levs.end()) {
		shared_ptr<EventSubscribe> evSub = *it;
		levs.erase(it);
		evSub->terminate();
	}

	const auto &accountParams = account->getAccountParams();
	auto identityAddress = accountParams->getIdentityAddress();
	for (const auto &[key, handlerWkPtr] : handlers) {
		try {
			const std::shared_ptr<ClientConferenceEventHandler> handler(handlerWkPtr);
			const ConferenceId &conferenceId = handler->getConferenceId();
			if (identityAddress->weakEqual(*conferenceId.getLocalAddress())) {
				handler->setManagedByListEventhandler(false);
			}
		} catch (const bad_weak_ptr &) {
		}
	}
}

void ClientConferenceListEventHandler::invalidateSubscription() {
	levs.clear();
}

bool ClientConferenceListEventHandler::getInitialSubscriptionUnderWayFlag(const ConferenceId &conferenceId) const {
	auto handler = findHandler(conferenceId);
	return (handler) ? handler->getInitialSubscriptionUnderWayFlag() : false;
}

void ClientConferenceListEventHandler::notifyReceived(std::shared_ptr<Event> notifyLev,
                                                      const std::shared_ptr<const Content> &notifyContent) {
	if (notifyContent) {
		const auto &from = notifyLev->getFrom();
		auto it = std::find_if(levs.begin(), levs.end(), [&from](const auto &lev) {
			return (*Address::create(lev->getOp()->getFrom()) == *from);
		});
		auto core = getCore();
		const auto conferenceIdParams = core->createConferenceIdParams();
		const auto levFound = (it != levs.end());
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
			ConferenceId id(entityAddress, from, conferenceIdParams);
			auto handler = findHandler(id);
			if (!handler) return;

			handler->notifyReceived(*notifyContent);
			if (handler->getInitialSubscriptionUnderWayFlag()) {
				handler->setInitialSubscriptionUnderWayFlag(!levFound);
			}
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
			ConferenceId id(peer, from, conferenceIdParams);
			auto handler = findHandler(id);
			if (!handler) continue;

			if (contentType == ContentType::Multipart) handler->multipartNotifyReceived(content);
			else if (contentType == ContentType::ConferenceInfo) handler->notifyReceived(content);

			for (const auto &[key, handlerWkPtr] : handlers) {
				try {
					const std::shared_ptr<ClientConferenceEventHandler> handler(handlerWkPtr);
					if (handler->getInitialSubscriptionUnderWayFlag()) {
						handler->setInitialSubscriptionUnderWayFlag(!levFound);
					}
				} catch (const bad_weak_ptr &) {
				}
			}
		}
	}
}

// -----------------------------------------------------------------------------

std::shared_ptr<ClientConferenceEventHandler>
ClientConferenceListEventHandler::findHandler(const ConferenceId &conferenceId) const {
	const auto it = handlers.find(conferenceId);
	if (it != handlers.end()) {
		try {
			const std::shared_ptr<ClientConferenceEventHandler> handler = (*it).second.lock();
			return handler;
		} catch (const bad_weak_ptr &) {
		}
	}
	return nullptr;
}

void ClientConferenceListEventHandler::addHandler(std::shared_ptr<ClientConferenceEventHandler> handler) {
	if (!handler) {
		lWarning() << "Trying to insert null handler in the client conference handler list";
		return;
	}

	const ConferenceId &conferenceId = handler->getConferenceId();
	if (!conferenceId.isValid()) {
		lError() << "ClientConferenceListEventHandler::addHandler conference ID " << conferenceId << " is not valid";
		return;
	}

	if (!isHandlerInSameDomainAsCore(conferenceId)) {
		// lWarning() << "The chatroom with conference id " << conferenceId
		//            << " is not in the same domain as the conference factory of the account is linked to hence not "
		//               "adding to the list of subscribes";
		return;
	}

	if (findHandler(conferenceId)) {
		lWarning() << "Trying to insert an already present handler in the client conference handler list: "
		           << conferenceId;
		return;
	}
	handlers[conferenceId] = handler;
}

bool ClientConferenceListEventHandler::isHandlerInSameDomainAsCore(const ConferenceId &conferenceId) const {
	// Ensure that conference and conference factory are in the same domain
	const std::shared_ptr<Address> &localAddress = conferenceId.getLocalAddress();

	const auto conferenceFactoryUri = Core::getConferenceFactoryAddress(getCore(), localAddress);
	if (!conferenceFactoryUri) {
		lDebug() << "Account with local address [" << localAddress->getUri()
		         << "] hasn't a conference factory URI defined.";
		return false;
	}

	const std::shared_ptr<Address> &peerAddress = conferenceId.getPeerAddress();
	if (peerAddress->getDomain() != conferenceFactoryUri->getDomain()) {
		lDebug() << "Peer address [" << peerAddress->getUri()
		         << "] is not in the same domain as the conference factory URI [" << conferenceFactoryUri->getUri()
		         << "]";
		return false;
	}

	return true;
}

void ClientConferenceListEventHandler::removeHandler(std::shared_ptr<ClientConferenceEventHandler> handler) {
	if (!handler) {
		return;
	}

	const ConferenceId &conferenceId = handler->getConferenceId();
	if (!conferenceId.isValid()) {
		lError() << "ClientConferenceListEventHandler::removeHandler() invalid handler.";
		return;
	}

	if (!isHandlerInSameDomainAsCore(conferenceId)) {
		lWarning() << "The chatroom with conference id " << conferenceId
		           << " is not in the same domain as the conference factory of the account is linked to hence no need "
		              "to remove it from the list of subscribes";
		return;
	}

	lInfo() << __func__ << " searched handler " << handler << " handler id " << handler->getConferenceId();
	for (const auto &[id, handler2] : handlers) {
		lInfo() << __func__ << " id " << id << " handler " << handler2.lock() << " handler id "
		        << handler2.lock()->getConferenceId();
	}

	auto it = handlers.find(conferenceId);
	if (it != handlers.end()) {
		handler->setManagedByListEventhandler(false);
		handlers.erase(it);
		lInfo() << "Client Conference Event Handler with conference id " << conferenceId << " [" << handler
		        << "] has been removed.";
	} else {
		lError() << "Client Conference Event Handler with conference id " << conferenceId << " has not been found.";
	}
}

void ClientConferenceListEventHandler::clearHandlers() {
	handlers.clear();
}

map<string, std::shared_ptr<Address>> ClientConferenceListEventHandler::parseRlmi(const string &xmlBody) const {
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

void ClientConferenceListEventHandler::onNetworkReachable(bool sipNetworkReachable,
                                                          BCTBX_UNUSED(bool mediaNetworkReachable)) {
	if (sipNetworkReachable) {
		subscribe();
	} else {
		unsubscribe();
	}
}

void ClientConferenceListEventHandler::onAccountRegistrationStateChanged(std::shared_ptr<Account> account,
                                                                         LinphoneRegistrationState state,
                                                                         BCTBX_UNUSED(const std::string &message)) {
	if ((state == LinphoneRegistrationOk) && (account->getPreviousState() != LinphoneRegistrationRefreshing))
		subscribe(account);
	else if (state == LinphoneRegistrationCleared) { // On cleared, restart subscription if the cleared proxy config is
		                                             // the current subscription
		const auto &accountParams = account->getAccountParams();
		const auto &cfgAddress = accountParams->getIdentityAddress();
		auto it = std::find_if(levs.begin(), levs.end(), [&cfgAddress](const auto &lev) {
			return (*Address::create(lev->getOp()->getFrom()) == *cfgAddress);
		});

		// If no subscription is found, then unsubscribe the account
		if (it != levs.end()) unsubscribe(account);
	}
}

void ClientConferenceListEventHandler::onEnteringBackground() {
	unsubscribe();
}

void ClientConferenceListEventHandler::onEnteringForeground() {
	subscribe();
}

LINPHONE_END_NAMESPACE
