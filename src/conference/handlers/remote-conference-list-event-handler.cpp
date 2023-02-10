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

#include <bctoolbox/defs.h>

#include "linphone/core.h"
#include "linphone/event.h"
#include "linphone/proxy_config.h"
#include "linphone/utils/utils.h"

#include "account/account.h"
#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "content/content-manager.h"
#include "content/content-type.h"
#include "core/core-p.h"
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

RemoteConferenceListEventHandler::RemoteConferenceListEventHandler (const std::shared_ptr<Core> &core) : CoreAccessor(core) {
	getCore()->getPrivate()->registerListener(this);
}

RemoteConferenceListEventHandler::~RemoteConferenceListEventHandler () {
	try {
		getCore()->getPrivate()->unregisterListener(this);
	} catch (const bad_weak_ptr &) {
		// Unable to unregister listener here. Core is destroyed and the listener doesn't exist.
	}

	unsubscribe();
}

// -----------------------------------------------------------------------------

void RemoteConferenceListEventHandler::subscribe () {
	LinphoneCore *lc = getCore()->getCCore();
	const bctbx_list_t* list=linphone_core_get_account_list(lc);

	for(;list!=NULL;list=list->next){
		subscribe((LinphoneAccount *)list->data);
	}
}

void RemoteConferenceListEventHandler::subscribe (LinphoneAccount * c_account) {
	unsubscribe(c_account);

	if (handlers.empty())
		return;

	Content content;
	content.setContentType(ContentType::ResourceLists);

	Xsd::ResourceLists::ResourceLists rl = Xsd::ResourceLists::ResourceLists();
	Xsd::ResourceLists::ListType l = Xsd::ResourceLists::ListType();

	auto account = Account::toCpp(c_account);
	const auto & accountParams = account->getAccountParams();
	Address identityAddress = *L_GET_CPP_PTR_FROM_C_OBJECT(accountParams->getIdentityAddress());

	for (const auto &p : handlers) {
		RemoteConferenceEventHandler *handler = p.second;
		const ConferenceId &conferenceId = handler->getConferenceId();
		if (identityAddress.weakEqual(conferenceId.getLocalAddress().asAddress())) {
			shared_ptr<AbstractChatRoom> cr = getCore()->findChatRoom(conferenceId);
			if (!cr) {
				lError() << "Couldn't add chat room " << conferenceId << "in the chat room list subscription because chat room couldn't be found";
				continue;
			}

			if (cr->hasBeenLeft())
				continue;

			Address addr = conferenceId.getPeerAddress().asAddress();
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

	if (account->getState() != LinphoneRegistrationOk)
		return;

	const auto & factoryUri = accountParams->getConferenceFactoryUri();
	if (factoryUri.empty()) {
		lError() << "Couldn't send chat room list subscription for account " << account << " (" << identityAddress << ") because there's no conference factory uri";
		return;
	}

	LinphoneAddress *rlsAddr = linphone_address_new(L_STRING_TO_C(factoryUri));

	LinphoneCore *lc = getCore()->getCCore();
	LinphoneEvent *lev = linphone_core_create_subscribe(lc, rlsAddr, "conference", 600);
	char *from = linphone_address_as_string(account->getContactAddress());
	lev->op->setFrom(from);
	bctbx_free(from);
	linphone_address_unref(rlsAddr);
	linphone_event_set_internal(lev, TRUE);
	linphone_event_add_custom_header(lev, "Require", "recipient-list-subscribe");
	linphone_event_add_custom_header(lev, "Accept", "multipart/related, application/conference-info+xml, application/rlmi+xml");
	linphone_event_add_custom_header(lev, "Content-Disposition", "recipient-list");
	if (linphone_core_content_encoding_supported(lc, "deflate")) {
		content.setContentEncoding("deflate");
		linphone_event_add_custom_header(lev, "Accept-Encoding", "deflate");
	}
	belle_sip_object_data_set(BELLE_SIP_OBJECT(lev), "list-event-handler", this, NULL);
	LinphoneContent *cContent = L_GET_C_BACK_PTR(&content);
	linphone_event_send_subscribe(lev, cContent);

	levs.push_back(lev);
}

void RemoteConferenceListEventHandler::unsubscribe () {
	for (auto & lev : levs) {
		linphone_event_terminate(lev);
		linphone_event_unref(lev);
	}
	levs.clear();
}

void RemoteConferenceListEventHandler::unsubscribe (LinphoneAccount * c_account) {
	auto account = Account::toCpp(c_account);
	if(!account || !account->getContactAddress())
		return;
	char *from = linphone_address_as_string(account->getContactAddress());
	auto it = std::find_if(levs.begin(), levs.end(), [from] (const auto & lev) {
		return (lev->op->getFrom() == from);
	});
	bctbx_free(from);

	if (it != levs.end()) {
		LinphoneEvent * lev = *it;
		levs.erase(it);
		linphone_event_terminate(lev);
		linphone_event_unref(lev);
	}
}

void RemoteConferenceListEventHandler::invalidateSubscription () {
	levs.clear();
}

bool RemoteConferenceListEventHandler::getInitialSubscriptionUnderWayFlag(const ConferenceId & conferenceId) const {
	auto handler = findHandler(conferenceId);
	return (handler) ? handler->getInitialSubscriptionUnderWayFlag() : false;

}

void RemoteConferenceListEventHandler::notifyReceived (std::string from, const Content *notifyContent) {
	const ConferenceAddress local(from);

	if (notifyContent->getContentType() == ContentType::ConferenceInfo) {
		// Simple notify received directly from a chat-room
		const string &xmlBody = notifyContent->getBodyAsUtf8String();
		istringstream data(xmlBody);
		unique_ptr<Xsd::ConferenceInfo::ConferenceType> confInfo;
		try {
			confInfo = Xsd::ConferenceInfo::parseConferenceInfo(
				data,
				Xsd::XmlSchema::Flags::dont_validate
			);
		} catch (const exception &) {
			lError() << "Error while parsing conference-info in conferences notify";
			return;
		}

		ConferenceAddress entityAddress(confInfo->getEntity().c_str());
		ConferenceId id(entityAddress, local);
		RemoteConferenceEventHandler *handler = findHandler(id);
		if (!handler)
			return;

		handler->notifyReceived(*notifyContent);
		return;
	}

	list<Content> contents = ContentManager::multipartToContentList(*notifyContent);
	map<string, IdentityAddress> addresses;
	for (const auto &content : contents) {
		const string &body = content.getBodyAsUtf8String();
		const ContentType &contentType = content.getContentType();
		if (contentType == ContentType::Rlmi) {
			addresses = parseRlmi(body);
			continue;
		}

		const string &cid = content.getHeader("Content-Id").getValue();
		if (cid.empty())
			continue;

		map<string, IdentityAddress>::const_iterator it = addresses.find(cid);
		if (it == addresses.cend())
			continue;

		IdentityAddress peer = it->second;
		ConferenceId id(peer, local);
		RemoteConferenceEventHandler *handler = findHandler(id);
		if (!handler)
			continue;

		if (contentType == ContentType::Multipart)
			handler->multipartNotifyReceived(content);
		else if (contentType == ContentType::ConferenceInfo)
			handler->notifyReceived(content);
	}
}

// -----------------------------------------------------------------------------

RemoteConferenceEventHandler *RemoteConferenceListEventHandler::findHandler (const ConferenceId &conferenceId) const {
	const auto it = handlers.find(conferenceId);
	if (it != handlers.end()){
		return (*it).second;
	}
	return nullptr;
}

void RemoteConferenceListEventHandler::addHandler (RemoteConferenceEventHandler *handler) {
	if (!handler) {
		lWarning() << "Trying to insert null handler in the remote conference handler list";
		return;
	}

	const ConferenceId & conferenceId = handler->getConferenceId();

	if (!conferenceId.isValid()){
		lError() << "RemoteConferenceListEventHandler::addHandler invalid handler.";
		return;
	}

	if (!isHandlerInSameDomainAsCore(conferenceId)){
		return;
	}

	if(findHandler(conferenceId)) {
		lWarning() << "Trying to insert an already present handler in the remote conference handler list: " << conferenceId;
		return;
	}
	handlers[conferenceId] = handler;
}

bool RemoteConferenceListEventHandler::isHandlerInSameDomainAsCore(const ConferenceId & conferenceId)const {
	// Ensure that conference and conference factory are in the same domain
	const ConferenceAddress & localAddress = conferenceId.getLocalAddress();
	const ConferenceAddress & peerAddress = conferenceId.getPeerAddress();
	IdentityAddress conferenceFactoryUri = IdentityAddress(Core::getConferenceFactoryUri(getCore(), localAddress));

	if (peerAddress.getDomain() != conferenceFactoryUri.getDomain()) {
		lWarning() << "Peer address " << peerAddress.asString() << " is not in the same domain as the conference factory URI " << conferenceFactoryUri.asString() << " hence not adding to the list of subscribes";
		return false;
	}

	return true;
}

void RemoteConferenceListEventHandler::removeHandler (RemoteConferenceEventHandler *handler) {
	const ConferenceId & conferenceId = handler->getConferenceId();
	if (!conferenceId.isValid()){
		lError() << "RemoteConferenceListEventHandler::removeHandler() invalid handler.";
		return;
	}

	if (!isHandlerInSameDomainAsCore(conferenceId)){
		return;
	}

	if (handler){
		auto it = handlers.find(conferenceId);
		if (it != handlers.end() && handler == (*it).second){
			handlers.erase(it);
			lInfo() << "Handler removed.";
		}else{
			lError() << "Handler not found in RemoteConferenceListEventHandler.";
		}
	}else{
		lError() << "Handler is null !";
	}
}


void RemoteConferenceListEventHandler::clearHandlers () {
	handlers.clear();
}

map<string, IdentityAddress> RemoteConferenceListEventHandler::parseRlmi (const string &xmlBody) const {
	istringstream data(xmlBody);
	map<string, IdentityAddress> addresses;
	unique_ptr<Xsd::Rlmi::List> rlmi;
	try {
		rlmi = Xsd::Rlmi::parseList(
			data,
			Xsd::XmlSchema::Flags::dont_validate
		);
	} catch (const exception &) {
		lError() << "Error while parsing RLMI in conferences notify";
		return addresses;
	}
	for (const auto &resource : rlmi->getResource()) {
		if (resource.getInstance().empty())
			continue;

		const string &uri = string(resource.getUri());
		if (uri.empty())
			continue;

		IdentityAddress peer(uri);
		for (const auto &instance : resource.getInstance()) {
			const string &cid = string(instance.getId());
			if (cid.empty())
				continue;

			addresses.emplace(cid, peer);
		}
	}
	return addresses;
}

// -----------------------------------------------------------------------------

void RemoteConferenceListEventHandler::onNetworkReachable (bool sipNetworkReachable, UNUSED(bool mediaNetworkReachable)) {
	if (sipNetworkReachable) {
		subscribe();
	} else {
		unsubscribe();
	}
}

void RemoteConferenceListEventHandler::onRegistrationStateChanged (LinphoneProxyConfig *cfg, LinphoneRegistrationState state, UNUSED(const std::string &message)) {
	if (state == LinphoneRegistrationOk )
		subscribe(cfg->account);
	else if(state == LinphoneRegistrationCleared){// On cleared, restart subscription if the cleared proxy config is the current subscription
		const LinphoneAddress * cfgAddress = linphone_proxy_config_get_contact(cfg);
		auto it = std::find_if(levs.begin(), levs.end(), [&cfgAddress] (const auto & lev) {
			LinphoneAddress * currentAddress = linphone_address_new(lev->op->getFrom().c_str());
			return linphone_address_weak_equal(currentAddress, cfgAddress);
		});

		if(it != levs.end())
			unsubscribe(cfg->account);
	}
}

void RemoteConferenceListEventHandler::onEnteringBackground () {
	unsubscribe();
}

void RemoteConferenceListEventHandler::onEnteringForeground () {
	subscribe();
}

LINPHONE_END_NAMESPACE
