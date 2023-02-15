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

#include "belle-sip/utils.h"
#include "linphone/api/c-address.h"
#include "linphone/enums/chat-room-enums.h"
#include "linphone/utils/utils.h"

#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "chat/chat-room/abstract-chat-room.h"
#include "conference/local-conference.h"
#include "conference/participant-device.h"
#include "conference/participant.h"
#include "content/content-manager.h"
#include "content/content-type.h"
#include "content/content.h"
#include "core/core.h"
#include "local-conference-event-handler.h"
#include "local-conference-list-event-handler.h"
#include "logger/logger.h"
#include "xml/resource-lists.h"
#include "xml/rlmi.h"

// TODO: Remove me later.
#include "private.h"

// =============================================================================

using namespace std;

class LocalConference;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

LocalConferenceListEventHandler::LocalConferenceListEventHandler(const std::shared_ptr<Core> &core)
    : CoreAccessor(core) {
}

// -----------------------------------------------------------------------------

void LocalConferenceListEventHandler::notifyResponseCb(const LinphoneEvent *lev) {
	auto ev = dynamic_pointer_cast<EventSubscribe>(Event::toCpp(const_cast<LinphoneEvent *>(lev))->getSharedFromThis());
	shared_ptr<EventCbs> cbs = ev->getCurrentCallbacks();
	LocalConferenceListEventHandler *listHandler = static_cast<LocalConferenceListEventHandler *>(cbs->getUserData());
	cbs->setUserData(nullptr);
	cbs->notifyResponseCb = nullptr;

	if (ev->getReason() != LinphoneReasonNone) return;

	for (const auto &p : listHandler->handlers) {
		cbs->setUserData(p.second);
		LocalConferenceEventHandler::notifyResponseCb(ev->toC());
	}
	cbs->setUserData(nullptr);
}

// -----------------------------------------------------------------------------

void LocalConferenceListEventHandler::subscribeReceived(const std::shared_ptr<EventSubscribe> &ev,
                                                        const LinphoneContent *body) {
	LinphoneSubscriptionState subscriptionState = ev->getState();

	const string &xmlBody = string(linphone_content_get_utf8_text(body));
	if (xmlBody.empty()) {
		ev->deny(LinphoneReasonDeclined);
		return;
	}

	ev->accept();

	if (subscriptionState != LinphoneSubscriptionIncomingReceived &&
	    subscriptionState != LinphoneSubscriptionTerminated)
		return;

	const auto &participantAddr = ev->getFrom();
	const auto &deviceAddr = ev->getRemoteContact();

	// Create Rlmi body
	Xsd::Rlmi::List::ResourceSequence resources;

	// Parse resource list
	bool noContent = true;
	list<Content> contents;
	istringstream data(xmlBody);
	unique_ptr<Xsd::ResourceLists::ResourceLists> rl;
	try {
		rl = Xsd::ResourceLists::parseResourceLists(data, Xsd::XmlSchema::Flags::dont_validate);
	} catch (const exception &) {
		lError() << "Error while parsing subscribe body for conferences asked by: " << participantAddr;
		return;
	}

	for (const auto &l : rl->getList()) {
		for (const auto &entry : l.getEntry()) {
			std::shared_ptr<Address> addr = Address::create(entry.getUri());
			string notifyIdStr = addr->getUriParamValue("Last-Notify");
			addr->removeUriParam("Last-Notify");
			ConferenceId conferenceId(addr, addr);
			LocalConferenceEventHandler *handler = findHandler(conferenceId);
			if (!handler) continue;

			shared_ptr<AbstractChatRoom> chatRoom = ev->getCore()->findChatRoom(conferenceId);
			if (!chatRoom) {
				lError() << "Received subscribe for unknown chat room: " << conferenceId;
				continue;
			}

			shared_ptr<Participant> participant = chatRoom->findParticipant(participantAddr);
			if (!participant) {
				lError() << "Received subscribe for unknown participant: " << participantAddr
				         << " for chat room: " << conferenceId;
				continue;
			}
			shared_ptr<ParticipantDevice> device = participant->findDevice(deviceAddr);
			if (!device || (device->getState() != ParticipantDevice::State::Present &&
			                device->getState() != ParticipantDevice::State::Joining)) {
				lError() << "Received subscribe for unknown device: " << deviceAddr
				         << " for participant: " << participantAddr << " for chat room: " << conferenceId;
				continue;
			}
			device->setConferenceSubscribeEvent((subscriptionState == LinphoneSubscriptionIncomingReceived) ? ev
			                                                                                                : nullptr);

			int notifyId = (notifyIdStr.empty() || device->getState() == ParticipantDevice::State::Joining)
			                   ? 0
			                   : Utils::stoi(notifyIdStr);
			Content content = handler->getNotifyForId(notifyId, device->getConferenceSubscribeEvent());
			if (content.isEmpty()) continue;

			noContent = false;
			char token[17];
			belle_sip_random_token(token, sizeof(token));
			content.addHeader("Content-Id", token);
			content.addHeader("Content-Length", Utils::toString(content.getSize()));
			contents.push_back(std::move(content));

			// Add entry into the Rlmi content of the notify body
			Xsd::Rlmi::Resource resource(addr->asStringUriOnly());
			Xsd::Rlmi::Resource::InstanceSequence instances;
			Xsd::Rlmi::Instance instance(token, Xsd::Rlmi::State::Value::active);
			instances.push_back(instance);
			resource.setInstance(instances);
			resources.push_back(resource);
		}
	}

	if (noContent) return;

	Xsd::Rlmi::List rlmiList("", 0, TRUE);
	rlmiList.setResource(resources);
	Xsd::XmlSchema::NamespaceInfomap map;
	stringstream rlmiBody;
	Xsd::Rlmi::serializeList(rlmiBody, rlmiList, map);

	Content rlmiContent;
	rlmiContent.setContentType(ContentType::Rlmi);
	rlmiContent.setBodyFromUtf8(rlmiBody.str());

	list<Content *> contentsAsPtr;
	contentsAsPtr.push_back(&rlmiContent);
	for (Content &content : contents) {
		contentsAsPtr.push_back(&content);
	}

	Content multipart = ContentManager::contentListToMultipart(contentsAsPtr);
	if (linphone_core_content_encoding_supported(getCore()->getCCore(), "deflate"))
		multipart.setContentEncoding("deflate");
	LinphoneContent *cContent = L_GET_C_BACK_PTR(&multipart);
	shared_ptr<EventCbs> cbs = EventCbs::create();
	cbs->setUserData(this);
	cbs->notifyResponseCb = notifyResponseCb;
	ev->addCallbacks(cbs);
	ev->notify(cContent);
}

// -----------------------------------------------------------------------------

void LocalConferenceListEventHandler::addHandler(LocalConferenceEventHandler *handler) {
	if (!handler) {
		lError() << "Trying to insert null handler in the local conference handler list";
		return;
	}

	if (findHandler(handler->conf->getConferenceId())) {
		lError() << "Trying to insert an already present handler in the local conference handler list: "
		         << handler->conf->getConferenceId();
		return;
	}

	handlers[handler->conf->getConferenceId()] = handler;
}

void LocalConferenceListEventHandler::removeHandler(LocalConferenceEventHandler *handler) {
	if (handler) {
		auto it = handlers.find(handler->conf->getConferenceId());
		if (it != handlers.end()) {
			handlers.erase(it);
			lInfo() << "Handler removed.";
		} else {
			lError() << "Handler not found in LocalConferenceListEventHandler.";
		}
	} else {
		lError() << "Handler is null !";
	}
}

LocalConferenceEventHandler *LocalConferenceListEventHandler::findHandler(const ConferenceId &conferenceId) const {
	auto it = handlers.find(conferenceId);
	if (it != handlers.end()) return (*it).second;
	return nullptr;
}

LINPHONE_END_NAMESPACE
