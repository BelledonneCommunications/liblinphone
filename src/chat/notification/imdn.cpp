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

#include "linphone/utils/algorithm.h"

#include "chat/chat-message/imdn-message-p.h"
#include "chat/chat-room/chat-room-p.h"
#include "core/core-p.h"
#include "logger/logger.h"

#ifdef HAVE_ADVANCED_IM
#include "xml/imdn.h"
#include "xml/linphone-imdn.h"
#endif

#include "imdn.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

Imdn::Imdn (ChatRoom *chatRoom) : chatRoom(chatRoom) {
	chatRoom->getCore()->getPrivate()->registerListener(this);
}

Imdn::~Imdn () {
	stopTimer();
	try { //getCore may no longuer be available when deleting, specially in case of managed enviroment like java
		chatRoom->getCore()->getPrivate()->unregisterListener(this);
	} catch (const bad_weak_ptr &) {}
}

// -----------------------------------------------------------------------------

void Imdn::notifyDelivery (const shared_ptr<ChatMessage> &message) {
	if (find(deliveredMessages, message) == deliveredMessages.end()) {
		deliveredMessages.push_back(message);
		startTimer();
	}
}

void Imdn::notifyDeliveryError (const shared_ptr<ChatMessage> &message, LinphoneReason reason) {
	if (
		findIf(nonDeliveredMessages, [message](const MessageReason &mr) {
			return message == mr.message;
		}) == nonDeliveredMessages.end()
	) {
		nonDeliveredMessages.emplace_back(message, reason);
		startTimer();
	}
}

void Imdn::notifyDisplay (const shared_ptr<ChatMessage> &message) {
	auto it = find(deliveredMessages.begin(), deliveredMessages.end(), message);
	if (it != deliveredMessages.end())
		deliveredMessages.erase(it);

	if (find(displayedMessages.begin(), displayedMessages.end(), message) == displayedMessages.end()) {
		displayedMessages.push_back(message);
		startTimer();
	}
}

// -----------------------------------------------------------------------------

void Imdn::onImdnMessageDelivered (const std::shared_ptr<ImdnMessage> &message) {
	// If an IMDN has been successfully delivered, remove it from the list so that
	// it does not get sent again
	auto context = message->getPrivate()->getContext();
	for (const auto &chatMessage : context.deliveredMessages) {
		chatMessage->getPrivate()->disableDeliveryNotificationRequiredInDatabase();
		deliveredMessages.remove(chatMessage);
	}

	for (const auto &chatMessage : context.displayedMessages) {
		chatMessage->getPrivate()->disableDisplayNotificationRequiredInDatabase();
		displayedMessages.remove(chatMessage);
	}

	for (const auto &chatMessage : context.nonDeliveredMessages)
		nonDeliveredMessages.remove(chatMessage);

	sentImdnMessages.remove(message);
}

// -----------------------------------------------------------------------------

void Imdn::onGlobalStateChanged (LinphoneGlobalState state) {
	if (state == LinphoneGlobalShutdown) {
		auto ref = chatRoom->getSharedFromThis();
		deliveredMessages.clear();
		displayedMessages.clear();
		nonDeliveredMessages.clear();
		sentImdnMessages.clear();
	}
}

void Imdn::onRegistrationStateChanged(LinphoneProxyConfig *cfg, LinphoneRegistrationState state, const std::string &message){
	if (state == LinphoneRegistrationOk && cfg == getRelatedProxyConfig()){
		// When we are registered to the proxy, then send pending notification if any.
		sentImdnMessages.clear();
		send();
	}
}

void Imdn::onNetworkReachable (bool sipNetworkReachable, bool mediaNetworkReachable) {
	if (sipNetworkReachable && getRelatedProxyConfig() == nullptr) {
		// When the SIP network gets up and this chatroom isn't related to any proxy configuration, retry notification
		sentImdnMessages.clear();
		send();
	}
}



// -----------------------------------------------------------------------------

string Imdn::createXml (const string &id, time_t timestamp, Imdn::Type imdnType, LinphoneReason reason) {
#ifdef HAVE_ADVANCED_IM
	char *datetime = linphone_timestamp_to_rfc3339_string(timestamp);
	Xsd::Imdn::Imdn imdn(id, datetime);
	ms_free(datetime);
	bool needLinphoneImdnNamespace = false;
	if (imdnType == Imdn::Type::Delivery) {
		Xsd::Imdn::Status status;
		if (reason == LinphoneReasonNone) {
			auto delivered = Xsd::Imdn::Delivered();
			status.setDelivered(delivered);
		} else {
			auto failed = Xsd::Imdn::Failed();
			status.setFailed(failed);
			Xsd::LinphoneImdn::ImdnReason imdnReason(linphone_reason_to_string(reason));
			imdnReason.setCode(linphone_reason_to_error_code(reason));
			status.setReason(imdnReason);
			needLinphoneImdnNamespace = true;
		}
		Xsd::Imdn::DeliveryNotification deliveryNotification(status);
		imdn.setDeliveryNotification(deliveryNotification);
	} else if (imdnType == Imdn::Type::Display) {
		Xsd::Imdn::Status1 status;
		auto displayed = Xsd::Imdn::Displayed();
		status.setDisplayed(displayed);
		Xsd::Imdn::DisplayNotification displayNotification(status);
		imdn.setDisplayNotification(displayNotification);
	}

	stringstream ss;
	Xsd::XmlSchema::NamespaceInfomap map;
	map[""].name = "urn:ietf:params:xml:ns:imdn";
	if (needLinphoneImdnNamespace)
		map["imdn"].name = "http://www.linphone.org/xsds/imdn.xsd";
	Xsd::Imdn::serializeImdn(ss, imdn, map, "UTF-8", Xsd::XmlSchema::Flags::dont_pretty_print);
	return ss.str();
#else
	lWarning() << "Advanced IM such as group chat is disabled!";
	return "";
#endif
}

void Imdn::parse (const shared_ptr<ChatMessage> &chatMessage) {
#ifdef HAVE_ADVANCED_IM
	shared_ptr<AbstractChatRoom> cr = chatMessage->getChatRoom();
	for (const auto &content : chatMessage->getPrivate()->getContents()) {
		istringstream data(content->getBodyAsString());
		unique_ptr<Xsd::Imdn::Imdn> imdn;
		try {
			imdn = Xsd::Imdn::parseImdn(data, Xsd::XmlSchema::Flags::dont_validate);
		} catch (const exception &e) {
			lError() << "IMDN parsing exception!";
		}
		if (!imdn)
			continue;
		
		shared_ptr<ChatMessage> cm = cr->findChatMessage(imdn->getMessageId());
		if (!cm) {
			lWarning() << "Received IMDN for unknown message " << imdn->getMessageId();
		} else {
			auto policy = linphone_core_get_im_notif_policy(cr->getCore()->getCCore());
			time_t imdnTime = chatMessage->getTime();
			const IdentityAddress &participantAddress = chatMessage->getFromAddress().getAddressWithoutGruu();
			auto &deliveryNotification = imdn->getDeliveryNotification();
			auto &displayNotification = imdn->getDisplayNotification();
			if (deliveryNotification.present()) {
				auto &status = deliveryNotification.get().getStatus();
				if (status.getDelivered().present() && linphone_im_notif_policy_get_recv_imdn_delivered(policy))
					cm->getPrivate()->setParticipantState(participantAddress, ChatMessage::State::DeliveredToUser, imdnTime);
				else if ((status.getFailed().present() || status.getError().present())
					&& linphone_im_notif_policy_get_recv_imdn_delivered(policy)
				)
					cm->getPrivate()->setParticipantState(participantAddress, ChatMessage::State::NotDelivered, imdnTime);
			} else if (displayNotification.present()) {
				auto &status = displayNotification.get().getStatus();
				if (status.getDisplayed().present() && linphone_im_notif_policy_get_recv_imdn_displayed(policy))
					cm->getPrivate()->setParticipantState(participantAddress, ChatMessage::State::Displayed, imdnTime);
			}
		}
	}
#else
	lWarning() << "Advanced IM such as group chat is disabled!";
#endif
}

bool Imdn::isError (const shared_ptr<ChatMessage> &chatMessage) {
#ifdef HAVE_ADVANCED_IM
	for (const auto &content : chatMessage->getPrivate()->getContents()) {
		if (content->getContentType() != ContentType::Imdn)
			continue;
		
		istringstream data(content->getBodyAsString());
		unique_ptr<Xsd::Imdn::Imdn> imdn;
		try {
			imdn = Xsd::Imdn::parseImdn(data, Xsd::XmlSchema::Flags::dont_validate);
		} catch (const exception &e) {
			lError() << "IMDN parsing exception!";
		}
		if (!imdn)
			continue;
		
		auto &deliveryNotification = imdn->getDeliveryNotification();
		if (deliveryNotification.present()) {
			auto &status = deliveryNotification.get().getStatus();
			if (status.getFailed().present() || status.getError().present())
				return true;
		}
	}
	return false;
#else
	lWarning() << "Advanced IM such as group chat is disabled!";
	return false;
#endif
}

// -----------------------------------------------------------------------------

int Imdn::timerExpired (void *data, unsigned int revents) {
	Imdn *d = reinterpret_cast<Imdn *>(data);
	d->stopTimer();
	d->send();
	return BELLE_SIP_STOP;
}

// -----------------------------------------------------------------------------

bool Imdn::aggregationEnabled () const {
	auto config = linphone_core_get_config(chatRoom->getCore()->getCCore());
	return (chatRoom->canHandleCpim() && linphone_config_get_bool(config, "misc", "aggregate_imdn", TRUE));
}

LinphoneProxyConfig * Imdn::getRelatedProxyConfig(){
	LinphoneAddress *addr = linphone_address_new(chatRoom->getLocalAddress().asString().c_str());
	if (!addr) {
		return NULL;
	}
	LinphoneProxyConfig *cfg = linphone_core_lookup_proxy_by_identity(chatRoom->getCore()->getCCore(), addr);
	linphone_address_unref(addr);
	return cfg;
}

void Imdn::send () {
	try {
		LinphoneProxyConfig *cfg = getRelatedProxyConfig();
		if (cfg && linphone_proxy_config_get_state(cfg) != LinphoneRegistrationOk){
			lInfo() << "Proxy config not registered, will wait to send pending IMDNs";
			return;
		}
		
		if (!linphone_core_is_network_reachable(chatRoom->getCore()->getCCore()))
			return;
	} catch (const bad_weak_ptr &) {
		return; // Cannot send imdn if core is destroyed.
	}

	if (!deliveredMessages.empty() || !displayedMessages.empty()) {
		if (aggregationEnabled()) {
			auto imdnMessage = chatRoom->getPrivate()->createImdnMessage(deliveredMessages, displayedMessages);
			sentImdnMessages.push_back(imdnMessage);
			imdnMessage->getPrivate()->send();
		} else {
			list<shared_ptr<ImdnMessage>> imdnMessages;
			for (const auto &message : deliveredMessages) {
				list<shared_ptr<ChatMessage>> l;
				l.push_back(message);
				imdnMessages.push_back(chatRoom->getPrivate()->createImdnMessage(l, list<shared_ptr<ChatMessage>>()));
			}
			for (const auto &message : displayedMessages) {
				list<shared_ptr<ChatMessage>> l;
				l.push_back(message);
				imdnMessages.push_back(chatRoom->getPrivate()->createImdnMessage(list<shared_ptr<ChatMessage>>(), l));
			}
			for (const auto &message : imdnMessages) {
				sentImdnMessages.push_back(message);
				message->getPrivate()->send();
			}
			deliveredMessages.clear();
			displayedMessages.clear();
		}
	}
	if (!nonDeliveredMessages.empty()) {
		if (aggregationEnabled()) {
			auto imdnMessage = chatRoom->getPrivate()->createImdnMessage(nonDeliveredMessages);
			sentImdnMessages.push_back(imdnMessage);
			imdnMessage->getPrivate()->send();
		} else {
			list<shared_ptr<ImdnMessage>> imdnMessages;
			for (const auto &message : nonDeliveredMessages) {
				list<MessageReason> l;
				l.push_back(message);
				imdnMessages.push_back(chatRoom->getPrivate()->createImdnMessage(l));
			}
			for (const auto &message : imdnMessages) {
				sentImdnMessages.push_back(message);
				message->getPrivate()->send();
			}
			nonDeliveredMessages.clear();
		}
	}
}

void Imdn::startTimer () {
	if (!aggregationEnabled()) {
		// Compatibility mode for basic chat rooms, do not aggregate notifications
		send();
		return;
	}

	unsigned int duration = 500;
	if (!timer)
		timer = chatRoom->getCore()->getCCore()->sal->createTimer(timerExpired, this, duration, "imdn timeout");
	else
		belle_sip_source_set_timeout(timer, duration);
	bgTask.start(chatRoom->getCore(), 1);
}

void Imdn::stopTimer () {
	if (timer) {
		auto core = chatRoom->getCore()->getCCore();
		if (core && core->sal)
			core->sal->cancelTimer(timer);
		belle_sip_object_unref(timer);
		timer = nullptr;
	}
	bgTask.stop();
}

LINPHONE_END_NAMESPACE
