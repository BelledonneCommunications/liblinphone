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

#include "linphone/utils/algorithm.h"

#include "chat/chat-message/imdn-message-p.h"
#include "chat/chat-room/chat-room.h"
#include "core/core-p.h"
#include "logger/logger.h"

#ifdef HAVE_ADVANCED_IM
#include "chat/encryption/encryption-engine.h"
#include "xml/imdn.h"
#include "xml/linphone-imdn.h"
#endif

#include "imdn.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

Imdn::Imdn(ChatRoom *chatRoom) : chatRoom(chatRoom) {
	chatRoom->getCore()->getPrivate()->registerListener(this);
	auto config = linphone_core_get_config(chatRoom->getCore()->getCCore());
	aggregationAllowed = linphone_config_get_bool(config, "misc", "aggregate_imdn", TRUE);
}

Imdn::~Imdn() {
	stopTimer();
	try { // getCore may no longuer be available when deleting, specially in case of managed enviroment like java
		chatRoom->getCore()->getPrivate()->unregisterListener(this);
	} catch (const bad_weak_ptr &) {
	}
}

// -----------------------------------------------------------------------------

void Imdn::notifyDelivery(const shared_ptr<ChatMessage> &message) {
	if (find(deliveredMessages, message) == deliveredMessages.end()) {
		deliveredMessages.push_back(message);
		startTimer();
	}
}

void Imdn::notifyDeliveryError(const shared_ptr<ChatMessage> &message, LinphoneReason reason) {
	if (findIf(nonDeliveredMessages, [message](const MessageReason &mr) { return message == mr.message; }) ==
	    nonDeliveredMessages.end()) {
		nonDeliveredMessages.emplace_back(message, reason);
		startTimer();
	}
}

void Imdn::notifyDisplay(const shared_ptr<ChatMessage> &message) {
	auto it = find(deliveredMessages.begin(), deliveredMessages.end(), message);
	if (it != deliveredMessages.end()) deliveredMessages.erase(it);

	if (find(displayedMessages.begin(), displayedMessages.end(), message) == displayedMessages.end()) {
		displayedMessages.push_back(message);
		startTimer();
	}
}

// -----------------------------------------------------------------------------

void Imdn::onImdnMessageDelivered(const std::shared_ptr<ImdnMessage> &message) {
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

void Imdn::onImdnMessageNotDelivered(const std::shared_ptr<ImdnMessage> &message) {
	sentImdnMessages.remove(message);
}

bool Imdn::isCurrentlySendingImdnMessages() {
	// This check is here ensure that we avoid blocking the stopping of the core due to IMDNs from previous accounts
	// that are still in the message data base
	if (!chatRoom->getCore()->getCCore()->send_imdn_if_unregistered) {
		LinphoneProxyConfig *cfg = getRelatedProxyConfig();
		if (!cfg || linphone_proxy_config_get_state(cfg) != LinphoneRegistrationOk) {
			return false;
		}
	}

	// IMDNs are pending if the timer before sending them is not NULL or if the list of IMDN chat message isn't empty
	return timer != NULL || !sentImdnMessages.empty();
}

// -----------------------------------------------------------------------------

void Imdn::onLinphoneCoreStop() {
	auto ref = chatRoom->getSharedFromThis();
	deliveredMessages.clear();
	displayedMessages.clear();
	nonDeliveredMessages.clear();
	sentImdnMessages.clear();
}

void Imdn::onRegistrationStateChanged(LinphoneProxyConfig *cfg,
                                      LinphoneRegistrationState state,
                                      BCTBX_UNUSED(const std::string &message)) {
	if (state == LinphoneRegistrationOk && cfg == getRelatedProxyConfig()) {
		// When we are registered to the proxy, then send pending notification if any.
		sentImdnMessages.clear();
		send();
	}
}

void Imdn::onNetworkReachable(bool sipNetworkReachable, BCTBX_UNUSED(bool mediaNetworkReachable)) {
	if (sipNetworkReachable && getRelatedProxyConfig() == nullptr) {
		// When the SIP network gets up and this chatroom isn't related to any proxy configuration, retry notification
		sentImdnMessages.clear();
		send();
	}
}

void Imdn::onGlobalStateChanged(LinphoneGlobalState state) {
	if (state == LinphoneGlobalShutdown) {
		// Stop timer and send IMDN even though it may not be sent to all devices in the chat room if we are not up to
		// date
		stopTimer();
		send();
	}
}

// -----------------------------------------------------------------------------
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
string Imdn::createXml(const string &id, time_t timestamp, Imdn::Type imdnType, LinphoneReason reason) {
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
	if (needLinphoneImdnNamespace) map["imdn"].name = "http://www.linphone.org/xsds/imdn.xsd";
	Xsd::Imdn::serializeImdn(ss, imdn, map, "UTF-8", Xsd::XmlSchema::Flags::dont_pretty_print);
	return ss.str();
#else
	lWarning() << "Advanced IM such as group chat is disabled!";
	return "";
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void Imdn::parse(const shared_ptr<ChatMessage> &chatMessage) {
#ifdef HAVE_ADVANCED_IM
	list<string> messagesIds;
	list<unique_ptr<Xsd::Imdn::Imdn>> imdns;

	for (const auto &content : chatMessage->getPrivate()->getContents()) {
		istringstream data(content->getBodyAsString());
		unique_ptr<Xsd::Imdn::Imdn> imdn;
		try {
			imdn = Xsd::Imdn::parseImdn(data, Xsd::XmlSchema::Flags::dont_validate);
		} catch (const exception &e) {
			lError() << "IMDN parsing exception: " << e.what();
		}
		if (!imdn) continue;

		messagesIds.push_back(imdn->getMessageId());
		imdns.push_back(std::move(imdn));
	}

	// It seems to be more efficient to only make one database request to get all chat messages from their IMDN message
	// ID
	list<shared_ptr<ChatMessage>> chatMessages =
	    chatMessage->getCore()->getPrivate()->mainDb->findChatMessagesFromImdnMessageId(messagesIds);
	for (const auto &imdn : imdns) {
		shared_ptr<ChatMessage> cm = nullptr;
		for (const auto &chatMessage : chatMessages) {
			if (chatMessage->getImdnMessageId() == imdn->getMessageId()) {
				cm = chatMessage;
				break;
			}
		}

		if (!cm) {
			lWarning() << "Received IMDN for unknown message " << imdn->getMessageId();
		} else {
			chatMessages.remove(cm);

			shared_ptr<AbstractChatRoom> cr = cm->getChatRoom();
			auto policy = linphone_core_get_im_notif_policy(cr->getCore()->getCCore());
			time_t imdnTime = chatMessage->getTime();
			std::shared_ptr<Address> participantAddress =
			    Address::create(chatMessage->getFromAddress()->getUriWithoutGruu());
			std::shared_ptr<Address> localAddress = cr->getLocalAddress();
			std::shared_ptr<Address> chatMessageFromAddress = cm->getFromAddress();
			auto &deliveryNotification = imdn->getDeliveryNotification();
			auto &displayNotification = imdn->getDisplayNotification();
			if (deliveryNotification.present()) {
				auto &status = deliveryNotification.get().getStatus();
				if (status.getDelivered().present() && linphone_im_notif_policy_get_recv_imdn_delivered(policy)) {
					cm->getPrivate()->setParticipantState(participantAddress, ChatMessage::State::DeliveredToUser,
					                                      imdnTime);
				} else if ((status.getFailed().present() || status.getError().present()) &&
				           (linphone_im_notif_policy_get_recv_imdn_delivered(policy) ||
				            linphone_im_notif_policy_get_recv_imdn_delivery_error(policy))) {
					cm->getPrivate()->setParticipantState(participantAddress, ChatMessage::State::NotDelivered,
					                                      imdnTime);

					const auto &chatRoomParams = cr->getCurrentParams();
					// When the IMDN status is failed for reason code 488 (Not acceptable here) and the chatroom is
					// encrypted, something is wrong with our encryption session with this peer, stale the active
					// session the next message (which can be a resend of this one) will be encrypted with a new session
					if (localAddress->weakEqual(*chatMessageFromAddress) // check the imdn is in response to a message
					                                                     // sent by the local user
					    && status.getFailed().present()                  // that we have a fail tag
					    && status.getReason().present()                  // and a reason tag
					    && chatRoomParams->getChatParams()->isEncrypted()) { // and the chatroom is encrypted
						// Check the reason code is 488
						auto reason = status.getReason().get();
						auto imee = cr->getCore()->getEncryptionEngine();
						if ((reason.getCode() == 488) && imee) {
							// stale the encryption sessions with this device: something went wrong, we will create a
							// new one at next encryption
							lWarning() << "Peer " << *chatMessage->getFromAddress()
							           << " could not decrypt message from " << *chatMessageFromAddress
							           << " -> Stale the lime X3DH session";
							imee->staleSession(chatMessageFromAddress->asStringUriOnly(),
							                   chatMessage->getFromAddress()->asStringUriOnly());
						}
					}
				}
			} else if (displayNotification.present()) {
				auto &status = displayNotification.get().getStatus();
				if (status.getDisplayed().present() && linphone_im_notif_policy_get_recv_imdn_displayed(policy)) {
					cm->getPrivate()->setParticipantState(participantAddress, ChatMessage::State::Displayed, imdnTime);
					if (localAddress->weakEqual(*participantAddress)) {
						auto lastMsg = cr->getLastChatMessageInHistory();
						if (lastMsg == cm) {
							lInfo() << "Received Display IMDN from ourselves for last message in this chat room, "
							           "marking it as read";
							cr->markAsRead();
						}
					}
				}
			}
		}
	}
#else
	lWarning() << "Advanced IM such as group chat is disabled!";
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
bool Imdn::isError(const shared_ptr<ChatMessage> &chatMessage) {
#ifdef HAVE_ADVANCED_IM
	for (const auto &content : chatMessage->getPrivate()->getContents()) {
		if (content->getContentType() != ContentType::Imdn) continue;

		istringstream data(content->getBodyAsString());
		unique_ptr<Xsd::Imdn::Imdn> imdn;
		try {
			imdn = Xsd::Imdn::parseImdn(data, Xsd::XmlSchema::Flags::dont_validate);
		} catch (const exception &e) {
			lError() << "IMDN parsing exception: " << e.what();
		}
		if (!imdn) continue;

		auto &deliveryNotification = imdn->getDeliveryNotification();
		if (deliveryNotification.present()) {
			auto &status = deliveryNotification.get().getStatus();
			if (status.getFailed().present() || status.getError().present()) return true;
		}
	}
	return false;
#else
	lWarning() << "Advanced IM such as group chat is disabled!";
	return false;
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

// -----------------------------------------------------------------------------

int Imdn::timerExpired(void *data, BCTBX_UNUSED(unsigned int revents)) {
	Imdn *d = static_cast<Imdn *>(data);
	d->stopTimer();
	d->send();
	return BELLE_SIP_STOP;
}

// -----------------------------------------------------------------------------

bool Imdn::aggregationEnabled() const {
	return chatRoom->canHandleCpim() && chatRoom->canHandleMultipart() && aggregationAllowed;
}

LinphoneProxyConfig *Imdn::getRelatedProxyConfig() {
	const auto &addr = chatRoom->getLocalAddress();
	if (!addr) {
		return NULL;
	}
	LinphoneProxyConfig *cfg =
	    linphone_core_lookup_proxy_by_identity_strict(chatRoom->getCore()->getCCore(), addr->toC());
	return cfg;
}

void Imdn::send() {
	if (deliveredMessages.empty() && displayedMessages.empty() && nonDeliveredMessages.empty()) {
		/* nothing to do */
		return;
	}

	try {
		if (!chatRoom->getCore()->getCCore()->send_imdn_if_unregistered) {
			LinphoneProxyConfig *cfg = getRelatedProxyConfig();
			if (!cfg) {
				lInfo() << "No matching proxy config found, will wait to send pending IMDNs";
				return;
			} else if (linphone_proxy_config_get_state(cfg) != LinphoneRegistrationOk) {
				lInfo() << "Proxy config not registered, will wait to send pending IMDNs";
				return;
			}

			if (!linphone_core_is_network_reachable(chatRoom->getCore()->getCCore())) return;
		}
	} catch (const bad_weak_ptr &) {
		return; // Cannot send imdn if core is destroyed.
	}

	std::list<std::shared_ptr<ImdnMessage>> imdnMessages;
	if (!deliveredMessages.empty() || !displayedMessages.empty()) {
		imdnMessages = chatRoom->createImdnMessages(deliveredMessages, displayedMessages, aggregationEnabled());
		deliveredMessages.clear();
		displayedMessages.clear();
	}
	if (!nonDeliveredMessages.empty()) {
		imdnMessages = chatRoom->createImdnMessages(nonDeliveredMessages, aggregationEnabled());
		nonDeliveredMessages.clear();
	}
	for (const auto &message : imdnMessages) {
		if (message->getPrivate()->getContents().empty()) {
			lWarning() << "Not sending IMDN delivery/displayed message as it contains no content";
		} else {
			sentImdnMessages.push_back(message);
			message->getChatRoom()->sendChatMessage(message);
		}
	}
}

void Imdn::startTimer() {
	if (!aggregationEnabled()) {
		// Compatibility mode for basic chat rooms, do not aggregate notifications
		send();
		return;
	}

	unsigned int duration = 500;
	if (!timer) timer = chatRoom->getCore()->getCCore()->sal->createTimer(timerExpired, this, duration, "imdn timeout");
	else belle_sip_source_set_timeout_int64(timer, duration);
	bgTask.start(chatRoom->getCore(), 1);
}

void Imdn::stopTimer() {
	if (timer) {
		auto core = chatRoom->getCore()->getCCore();
		if (core && core->sal) core->sal->cancelTimer(timer);
		belle_sip_object_unref(timer);
		timer = nullptr;
	}
	bgTask.stop();
}

LINPHONE_END_NAMESPACE
