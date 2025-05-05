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

#include <algorithm>

#include <bctoolbox/defs.h>

#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "chat/chat-message/chat-message-p.h"
#include "chat/chat-message/imdn-message.h"
#include "chat/chat-message/is-composing-message.h"
#include "chat/chat-message/notification-message-p.h"
#include "chat/chat-room/chat-room.h"
#include "conference/conference.h"
#include "content/content-manager.h"
#include "core/core-p.h"
#include "linphone/api/c-chat-message.h"
#include "linphone/api/c-event-log.h"
#include "linphone/utils/algorithm.h"
#include "linphone/utils/utils.h"
#include "logger/logger.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

#define NEW_LINE 0x2028
#define CRLF 0x0D0A
#define LF 0x0A
#define CR 0x0D

// =============================================================================

ChatRoom::ChatRoom(const shared_ptr<Core> &core, const std::shared_ptr<Conference> &conf) : AbstractChatRoom(core) {
	conference = conf;
	mImdnHandler.reset(new Imdn(this));
	mIsComposingHandler.reset(new IsComposing(core->getCCore(), this));
}

ChatRoom::~ChatRoom() {
	mImdnHandler.reset();
	conference = nullptr;
}

std::shared_ptr<Conference> ChatRoom::getConference() const {
	return conference;
};

ConferenceInterface::State ChatRoom::getState() const {
	const auto conferencePtr = getConference();
	if (conferencePtr) {
		return conferencePtr->getState();
	}
	return ConferenceInterface::State::None;
}

void ChatRoom::setState(ConferenceInterface::State newState) {
	const auto conferencePtr = getConference();
	if (conferencePtr) {
		conferencePtr->setState(newState);
	}
}

void ChatRoom::notifyStateChanged() {
	LinphoneChatRoom *cr = getCChatRoom();
	const auto &state = getState();
	const auto cCore = getCore()->getCCore();
	// Do not output this log while Core is starting up, a lot of them may happen
	if (cCore->state == LinphoneGlobalStartup) {
		lDebug() << "Chat room [" << getConferenceId() << "] state changed to: " << Utils::toString(state);
	} else {
		lInfo() << "Chat room [" << getConferenceId() << "] state changed to: " << Utils::toString(state);
	}
	LinphoneChatRoomState chatRoomCState =
	    linphone_conference_state_to_chat_room_state(static_cast<LinphoneConferenceState>(state));
	linphone_core_notify_chat_room_state_changed(cCore, cr, chatRoomCState);
	_linphone_chat_room_notify_state_changed(cr, chatRoomCState);
}

void ChatRoom::onStateChanged(BCTBX_UNUSED(ConferenceInterface::State state)) {
	notifyStateChanged();
}

void ChatRoom::invalidateAccount() {
	const auto conferencePtr = getConference();
	if (conferencePtr) {
		conferencePtr->invalidateAccount();
	}
}

std::shared_ptr<Account> ChatRoom::getAccount() {
	const auto conferencePtr = getConference();
	if (conferencePtr) {
		return conferencePtr->getAccount();
	}
	return nullptr;
}

const std::string &ChatRoom::getSubjectUtf8() const {
	const auto conferencePtr = getConference();
	if (conferencePtr) {
		return conferencePtr->getUtf8Subject();
	}
	return Utils::getEmptyConstRefObject<std::string>();
}

void ChatRoom::setUtf8Subject(const std::string &subject) {
	const auto conferencePtr = getConference();
	if (conferencePtr) {
		conferencePtr->setUtf8Subject(subject);
	}
}

const ConferenceId &ChatRoom::getConferenceId() const {
	const auto conferencePtr = getConference();
	if (conferencePtr) {
		return conferencePtr->getConferenceId();
	}
	return Utils::getEmptyConstRefObject<ConferenceId>();
};
// -----------------------------------------------------------------------------

void ChatRoom::sendChatMessage(const shared_ptr<ChatMessage> &chatMessage) {
	shared_ptr<Call> call = getCall();
	if (call && call->getCurrentParams()->realtimeTextEnabled()) {
		chatMessage->putCharacter(NEW_LINE);
	} else {
		ChatMessagePrivate *dChatMessage = chatMessage->getPrivate();
		dChatMessage->setTime(ms_time(0));
		if (!canHandleCpim()) {
			// if not using cpim, ImdnMessageId = SIP Message call id, so should be computed each time, specially in
			// case of resend.
			dChatMessage->setImdnMessageId("");
		}
		dChatMessage->send();
	}
}

void ChatRoom::onChatMessageSent(const shared_ptr<ChatMessage> &chatMessage) {
	LinphoneChatRoom *cr = getCChatRoom();
	unique_ptr<MainDb> &mainDb = getCore()->getPrivate()->mainDb;
	shared_ptr<EventLog> eventLog = mainDb->getEvent(mainDb, chatMessage->getStorageId());

	_linphone_chat_room_notify_chat_message_sent(cr, L_GET_C_BACK_PTR(eventLog));
	linphone_core_notify_message_sent(getCore()->getCCore(), cr, L_GET_C_BACK_PTR(chatMessage));

	if (mIsComposing) mIsComposing = false;
	mIsComposingHandler->stopIdleTimer();
	mIsComposingHandler->stopRefreshTimer();
}

void ChatRoom::sendIsComposingNotification() {
	LinphoneImNotifPolicy *policy = linphone_core_get_im_notif_policy(getCore()->getCCore());
	if (!linphone_im_notif_policy_get_send_is_composing(policy)) return;

	auto isComposingMsg = createIsComposingMessage();
	isComposingMsg->getPrivate()->send();
}

// -----------------------------------------------------------------------------

void ChatRoom::addCapability(AbstractChatRoom::CapabilitiesMask capability) {
	const auto &params = getCurrentParams();
	auto conferenceCapabilities = ConferenceParams::toCapabilities(params);
	conferenceCapabilities |= capability;
	if (params) {
		params->updateAccordingToCapabilities(conferenceCapabilities);
	}
}

void ChatRoom::addEvent(const shared_ptr<EventLog> &eventLog) {
	EventLog::Type type = eventLog->getType();

	if (!getCore()->getPrivate()->mainDb->addEvent(eventLog)) {
		lWarning() << "Failed to add event of type " << type << " to the database";
	}

	if (type == EventLog::Type::ConferenceChatMessage || type == EventLog::Type::ConferenceChatMessageReaction) {
		// Only update last event time on the chat room for those events
		// because they are visible and may cause the chat room to move
		// up in the list, so the user will know why.
		setLastUpdateTime(eventLog->getCreationTime());
		getCore()->getPrivate()->mainDb->updateChatRoomLastUpdatedTime(getConferenceId(), lastUpdateTime);

		if (type == EventLog::Type::ConferenceChatMessage) {
			setIsEmpty(false);
		}
	}
}

void ChatRoom::addTransientEvent(const shared_ptr<EventLog> &eventLog) {
	auto it = find(transientEvents, eventLog);
	if (it == transientEvents.end()) transientEvents.push_back(eventLog);
}

void ChatRoom::removeTransientEvent(const shared_ptr<EventLog> &eventLog) {
	auto it = find(transientEvents, eventLog);
	if (it != transientEvents.end()) transientEvents.erase(it);
}

void ChatRoom::addTransientChatMessage(const shared_ptr<ChatMessage> &message) {
	auto it = find(transientMessages, message);
	if (it == transientMessages.end()) transientMessages.push_back(message);
}

void ChatRoom::removeTransientChatMessage(const shared_ptr<ChatMessage> &message) {
	auto it = find(transientMessages, message);
	if (it != transientMessages.end()) transientMessages.erase(it);
}

std::list<std::shared_ptr<ChatMessage>> ChatRoom::getTransientChatMessages() {
	return transientMessages;
}

void ChatRoom::setIsEmpty(const bool empty) {
	mEmpty = empty;
}

void ChatRoom::realtimeTextReceived(uint32_t character, const shared_ptr<Call> &call) {
	realtimeTextOrBaudotCharacterReceived(character, call, true);
}

#ifdef HAVE_BAUDOT
void ChatRoom::baudotCharacterReceived(char character, const std::shared_ptr<Call> &call) {
	realtimeTextOrBaudotCharacterReceived((uint32_t)character, call, false);
}
#endif /* HAVE_BAUDOT */

void ChatRoom::realtimeTextOrBaudotCharacterReceived(uint32_t character,
                                                     const std::shared_ptr<Call> &call,
                                                     bool isRealTimeText) {
	shared_ptr<Core> core = getCore();
	LinphoneCore *cCore = core->getCCore();

	if (call && (!isRealTimeText || call->getCurrentParams()->realtimeTextEnabled())) {
		mReceivedRttCharacters.push_back(character);
		remoteIsComposing.push_back(getPeerAddress());
		linphone_core_notify_is_composing_received(cCore, getCChatRoom());

		bool isNewLine = false;
		if (isRealTimeText) {
			isNewLine = (character == NEW_LINE) || (character == CRLF) || (character == LF);
		} else {
			// Baudot
			isNewLine = (character == CR) || (character == LF);
		}
		if (isNewLine) {
			// End of message
			string completeText = Utils::unicodeToUtf8(mLastMessageCharacters);

			shared_ptr<ChatMessage> pendingMessage = createChatMessage();
			pendingMessage->getPrivate()->setDirection(ChatMessage::Direction::Incoming);
			auto content = Content::create();
			content->setContentType(ContentType::PlainText);
			content->setBodyFromUtf8(completeText);
			pendingMessage->addContent(content);

			lDebug() << "New line received, forge a message with content [" << content->getBodyAsString() << "]";
			pendingMessage->getPrivate()->setParticipantState(getMe()->getAddress(), ChatMessage::State::Delivered,
			                                                  ::ms_time(nullptr));
			pendingMessage->getPrivate()->setTime(::ms_time(0));

			const char *parameterName = isRealTimeText ? "store_rtt_messages" : "store_baudot_messages";
			if (linphone_config_get_int(linphone_core_get_config(cCore), "misc", parameterName, 1) == 1) {
				pendingMessage->getPrivate()->storeInDb();
			}

			onChatMessageReceived(pendingMessage);
			mLastMessageCharacters.clear();
		} else {
			mLastMessageCharacters.push_back(character);
			string completeText = Utils::unicodeToUtf8(mLastMessageCharacters);
			string textType = isRealTimeText ? "RTT" : "Baudot";
			lDebug() << "Received " << textType << " character: [" << character << "], pending text is ["
			         << completeText << "]";
		}
	}
}

// -----------------------------------------------------------------------------

shared_ptr<ChatMessage> ChatRoom::createChatMessage(ChatMessage::Direction direction) {
	shared_ptr<ChatMessage> message = shared_ptr<ChatMessage>(new ChatMessage(getSharedFromThis(), direction));
	if (ephemeralEnabled() && direction == ChatMessage::Direction::Outgoing) {
		lDebug() << "Create an outgoing ephemeral message " << message << " with lifetime " << getEphemeralLifetime()
		         << " in chat room [" << getConferenceId() << "]";
		message->getPrivate()->enableEphemeralWithTime(getEphemeralLifetime());
	}
	return message;
}

Address ChatRoom::getImdnChatRoomPeerAddress(const shared_ptr<ChatMessage> &message) const {
	const auto messageParticipants = static_cast<int>(message->getParticipantsState().size());
	const auto imdnParticipantThreshold = getCore()->getImdnToEverybodyThreshold();
	std::shared_ptr<Address> peerAddress;
	if (!getCurrentParams()->isGroup() || (messageParticipants <= imdnParticipantThreshold)) {
		peerAddress = getPeerAddress();
	} else if (message->getDirection() == ChatMessage::Direction::Incoming) {
		// An IMDN for an outgoing message would be sent to ourself
		peerAddress = message->getFromAddress();
	}
	return peerAddress ? peerAddress->getUriWithoutGruu() : Address();
}

std::shared_ptr<AbstractChatRoom> ChatRoom::getImdnChatRoom(const std::shared_ptr<Address> peerAddress) {
	auto chatRoomPeerAddress = getPeerAddress();
	std::shared_ptr<AbstractChatRoom> chatRoom;
	if (*peerAddress == chatRoomPeerAddress->getUriWithoutGruu()) {
		chatRoom = getSharedFromThis();
	} else {
		shared_ptr<ConferenceParams> params = ConferenceParams::create(getCore());
		params->setAccount(getCurrentParams()->getAccount());
		params->setChatDefaults();
		params->setGroup(false);
		const auto &backend = getCurrentParams()->getChatParams()->getBackend();
		params->getChatParams()->setBackend(backend);
		if (backend == ChatParams::Backend::FlexisipChat) {
			std::string subject(peerAddress->toString() + "'s IMDNs");
			params->setUtf8Subject(subject);
		}
		ConferenceParams::SecurityLevel securityLevel = ConferenceParams::SecurityLevel::None;
		auto isEncrypted = getCurrentParams()->getChatParams()->isEncrypted();
		if (isEncrypted) {
			// Try to infer chat room type based on requested participants number
			securityLevel = ConferenceParams::SecurityLevel::EndToEnd;
		} else {
			securityLevel = ConferenceParams::SecurityLevel::None;
		}
		params->setSecurityLevel(securityLevel);
		chatRoom = getCore()->getPrivate()->searchChatRoom(params, getLocalAddress(), nullptr, {peerAddress});
		if (!chatRoom) {
			chatRoom = getCore()->getPrivate()->createChatRoom(params, peerAddress);
		}
	}
	return chatRoom;
}

list<shared_ptr<ImdnMessage>> ChatRoom::createImdnMessages(const list<shared_ptr<ChatMessage>> &deliveredMessages,
                                                           const list<shared_ptr<ChatMessage>> &displayedMessages,
                                                           bool aggregate) {
	struct MessagePair {
		std::list<std::shared_ptr<ChatMessage>> deliveredMessages;
		std::list<std::shared_ptr<ChatMessage>> displayedMessages;
	};
	list<shared_ptr<ImdnMessage>> imdns;
	if (aggregate) {
		std::map<Address, MessagePair> messagesByImdnChatRoom;
		for (const auto &message : deliveredMessages) {
			auto peerAddress = getImdnChatRoomPeerAddress(message);
			if (peerAddress.isValid()) {
				messagesByImdnChatRoom[peerAddress].deliveredMessages.push_back(message);
			}
		}
		for (const auto &message : displayedMessages) {
			auto peerAddress = getImdnChatRoomPeerAddress(message);
			if (peerAddress.isValid()) {
				messagesByImdnChatRoom[peerAddress].displayedMessages.push_back(message);
			}
		}
		for (const auto &[peerAddress, messagePair] : messagesByImdnChatRoom) {
			imdns.push_back(
			    shared_ptr<ImdnMessage>(new ImdnMessage(getImdnChatRoom(Address::create(peerAddress)),
			                                            messagePair.deliveredMessages, messagePair.displayedMessages)));
		}
	} else {
		for (const auto &message : deliveredMessages) {
			auto peerAddress = getImdnChatRoomPeerAddress(message);
			if (peerAddress.isValid()) {
				imdns.push_back(shared_ptr<ImdnMessage>(new ImdnMessage(getImdnChatRoom(Address::create(peerAddress)),
				                                                        {message}, list<shared_ptr<ChatMessage>>())));
			}
		}
		for (const auto &message : displayedMessages) {
			auto peerAddress = getImdnChatRoomPeerAddress(message);
			if (peerAddress.isValid()) {
				imdns.push_back(shared_ptr<ImdnMessage>(new ImdnMessage(getImdnChatRoom(Address::create(peerAddress)),
				                                                        list<shared_ptr<ChatMessage>>(), {message})));
			}
		}
	}
	return imdns;
}

list<shared_ptr<ImdnMessage>> ChatRoom::createImdnMessages(const list<Imdn::MessageReason> &nonDeliveredMessages,
                                                           bool aggregate) {
	list<shared_ptr<ImdnMessage>> imdns;
	if (aggregate) {
		std::map<Address, std::list<Imdn::MessageReason>> messagesByImdnChatRoom;
		for (const auto &messageReason : nonDeliveredMessages) {
			auto peerAddress = getImdnChatRoomPeerAddress(messageReason.message);
			if (peerAddress.isValid()) {
				messagesByImdnChatRoom[peerAddress].push_back(messageReason);
			}
		}
		for (const auto &[peerAddress, messageReasons] : messagesByImdnChatRoom) {
			imdns.push_back(shared_ptr<ImdnMessage>(
			    new ImdnMessage(getImdnChatRoom(Address::create(peerAddress)), messageReasons)));
		}
	} else {
		for (const auto &messageReason : nonDeliveredMessages) {
			auto peerAddress = getImdnChatRoomPeerAddress(messageReason.message);
			if (peerAddress.isValid()) {
				imdns.push_back(shared_ptr<ImdnMessage>(
				    new ImdnMessage(getImdnChatRoom(Address::create(peerAddress)), {messageReason})));
			}
		}
	}
	return imdns;
}

shared_ptr<ImdnMessage> ChatRoom::createImdnMessage(const shared_ptr<ImdnMessage> &message) {
	return shared_ptr<ImdnMessage>(new ImdnMessage(message));
}

shared_ptr<IsComposingMessage> ChatRoom::createIsComposingMessage() {
	return shared_ptr<IsComposingMessage>(
	    new IsComposingMessage(getSharedFromThis(), *mIsComposingHandler.get(), mIsComposing));
}

shared_ptr<ChatMessage> ChatRoom::findChatMessage(const string &messageId) const {
	list<shared_ptr<ChatMessage>> chatMessages = findChatMessages(messageId);
	return chatMessages.empty() ? nullptr : chatMessages.front();
}

shared_ptr<EventLog> ChatRoom::findChatMessageEventLog(const string &messageId) const {
	return getCore()->getPrivate()->mainDb->findEventLog(getConferenceId(), messageId);
}

shared_ptr<ChatMessage> ChatRoom::findChatMessage(const string &messageId, ChatMessage::Direction direction) const {
	for (auto &chatMessage : findChatMessages(messageId))
		if (chatMessage->getDirection() == direction) return chatMessage;
	return nullptr;
}

shared_ptr<ChatMessage> ChatRoom::findChatMessageFromMessageId(const std::string &messageId) const {
	auto chatMessages = getCore()->getPrivate()->mainDb->findChatMessagesFromMessageId(messageId);
	return chatMessages.empty() ? nullptr : chatMessages.front();
}

shared_ptr<ChatMessage> ChatRoom::findChatMessageFromCallId(const std::string &callId) const {
	auto chatMessages = getCore()->getPrivate()->mainDb->findChatMessagesFromCallId(callId);
	return chatMessages.empty() ? nullptr : chatMessages.front();
}

void ChatRoom::markAsRead() {
	const auto &meAddress = getMe()->getAddress();
	// Mark any message currently waiting aggregation as read
	for (auto &chatMessage : aggregatedMessages) {
		chatMessage->getPrivate()->markAsRead();
		// Do not set the message state has displayed if it contains a file transfer (to prevent imdn sending)
		if (!chatMessage->getPrivate()->hasFileTransferContent()) {
			chatMessage->getPrivate()->setParticipantState(meAddress, ChatMessage::State::Displayed,
			                                               ::ms_time(nullptr));
		}
	}

	CorePrivate *dCore = getCore()->getPrivate();
	for (auto &chatMessage : dCore->mainDb->getUnreadChatMessages(getConferenceId())) {
		chatMessage->getPrivate()->markAsRead();
		// Do not set the message state has displayed if it contains a file transfer (to prevent imdn sending)
		if (!chatMessage->getPrivate()->hasFileTransferContent()) {
			chatMessage->getPrivate()->setParticipantState(meAddress, ChatMessage::State::Displayed,
			                                               ::ms_time(nullptr));
		}
	}

	dCore->mainDb->markChatMessagesAsRead(getConferenceId());
	_linphone_chat_room_notify_chat_room_read(getCChatRoom());
	linphone_core_notify_chat_room_read(getCore()->getCCore(), getCChatRoom());
}

list<shared_ptr<ChatMessage>> ChatRoom::findChatMessages(const string &messageId) const {
	return getCore()->getPrivate()->mainDb->findChatMessages(getConferenceId(), messageId);
}

list<shared_ptr<ChatMessage>> ChatRoom::findChatMessages(const list<string> &messageIds) const {
	return getCore()->getPrivate()->mainDb->findChatMessages(getConferenceId(), messageIds);
}

std::shared_ptr<EventLog> ChatRoom::searchChatMessageByText(const std::string &text,
                                                            const std::shared_ptr<const EventLog> &from,
                                                            LinphoneSearchDirection direction) const {
	return getCore()->getPrivate()->mainDb->searchChatMessagesByText(getConferenceId(), text, from, direction);
}

// -----------------------------------------------------------------------------

void ChatRoom::sendDeliveryErrorNotification(const shared_ptr<ChatMessage> &chatMessage, LinphoneReason reason) {
	LinphoneImNotifPolicy *policy = linphone_core_get_im_notif_policy(chatMessage->getCore()->getCCore());
	ChatMessagePrivate *dChatMessage = chatMessage->getPrivate();
	if ((linphone_im_notif_policy_get_send_imdn_delivered(policy) ||
	     linphone_im_notif_policy_get_send_imdn_delivery_error(policy)) &&
	    chatMessage->getPrivate()->getNegativeDeliveryNotificationRequired()) {
		dChatMessage->setNegativeDeliveryNotificationRequired(false);
		mImdnHandler->notifyDeliveryError(chatMessage, reason);
	}
}

void ChatRoom::sendDeliveryNotification(const shared_ptr<ChatMessage> &chatMessage) {
	LinphoneImNotifPolicy *policy = linphone_core_get_im_notif_policy(chatMessage->getCore()->getCCore());
	ChatMessagePrivate *dChatMessage = chatMessage->getPrivate();
	if (linphone_im_notif_policy_get_send_imdn_delivered(policy) &&
	    dChatMessage->getPositiveDeliveryNotificationRequired()) {
		dChatMessage->setPositiveDeliveryNotificationRequired(false);
		mImdnHandler->notifyDelivery(chatMessage);
	}
}

void ChatRoom::sendDeliveryNotifications(const std::shared_ptr<ChatMessage> &chatMessage) {
	ChatMessagePrivate *dChatMessage = chatMessage->getPrivate();
	if (dChatMessage->getPositiveDeliveryNotificationRequired()) {
		dChatMessage->setPositiveDeliveryNotificationRequired(false);
		mImdnHandler->notifyDelivery(chatMessage);
	}
}

void ChatRoom::sendDisplayNotification(const shared_ptr<ChatMessage> &chatMessage) {
	LinphoneImNotifPolicy *policy = linphone_core_get_im_notif_policy(getCore()->getCCore());
	ChatMessagePrivate *dChatMessage = chatMessage->getPrivate();
	if (linphone_im_notif_policy_get_send_imdn_displayed(policy) &&
	    chatMessage->getPrivate()->getDisplayNotificationRequired()) {
		dChatMessage->setPositiveDeliveryNotificationRequired(false);
		dChatMessage->setDisplayNotificationRequired(false);
		mImdnHandler->notifyDisplay(chatMessage);
	}
}

// -----------------------------------------------------------------------------

void ChatRoom::notifyChatMessageReceived(const shared_ptr<ChatMessage> &chatMessage) {
	LinphoneChatRoom *cr = getCChatRoom();
	if (!chatMessage->getPrivate()->getText().empty()) {
		/* Legacy API */
		LinphoneAddress *fromAddress = chatMessage->getFromAddress()->toC();
		linphone_core_notify_text_message_received(getCore()->getCCore(), cr, fromAddress,
		                                           chatMessage->getPrivate()->getText().c_str());
	}
	_linphone_chat_room_notify_message_received(cr, L_GET_C_BACK_PTR(chatMessage));
	linphone_core_notify_message_received(getCore()->getCCore(), cr, L_GET_C_BACK_PTR(chatMessage));
}

void ChatRoom::notifyIsComposingReceived(const std::shared_ptr<Address> &remoteAddress, bool isComposing) {
	auto it = find_if(remoteIsComposing.cbegin(), remoteIsComposing.cend(),
	                  [&remoteAddress](const auto &address) { return (*remoteAddress == *address); });

	if (isComposing) {
		if (it == remoteIsComposing.cend()) {
			remoteIsComposing.push_back(remoteAddress);
		}
	} else {
		if (it != remoteIsComposing.cend()) {
			remoteIsComposing.erase(it);
		}
	}

	LinphoneChatRoom *cr = getCChatRoom();
	LinphoneAddress *lAddr = remoteAddress->toC();
	_linphone_chat_room_notify_is_composing_received(cr, lAddr, !!isComposing);
	// Legacy notification
	linphone_core_notify_is_composing_received(getCore()->getCCore(), cr);
}

void ChatRoom::notifyUndecryptableChatMessageReceived(const shared_ptr<ChatMessage> &chatMessage) {
	LinphoneChatRoom *cr = getCChatRoom();
	_linphone_chat_room_notify_undecryptable_message_received(cr, L_GET_C_BACK_PTR(chatMessage));
	linphone_core_notify_message_received_unable_decrypt(getCore()->getCCore(), cr, L_GET_C_BACK_PTR(chatMessage));
}

// -----------------------------------------------------------------------------
void ChatRoom::handleMessageRejected(BCTBX_UNUSED(const std::shared_ptr<ChatMessage> &chatMessage)) {
	lDebug() << __func__ << " not implemented in chatroom [" << this;
}

std::shared_ptr<ChatMessage> ChatRoom::getMessageFromSal(SalOp *op, const SalMessage *message) {
	shared_ptr<ChatMessage> msg;

	auto from = Address::create(op->getFrom());
	msg = createChatMessage((*from == *getLocalAddress()) ? ChatMessage::Direction::Outgoing
	                                                      : ChatMessage::Direction::Incoming);

	Content content;
	if (message->url && ContentType(message->content_type) == ContentType::ExternalBody) {
		lInfo() << "Received a message with an external body URL " << message->url;
		content.setContentType(ContentType::FileTransfer);
		content.setBodyFromUtf8(msg->getPrivate()->createFakeFileTransferFromUrl(message->url));
	} else {
		content.setContentType(ContentType(message->content_type));
		content.setBodyFromUtf8(message->text ? message->text : "");
	}
	msg->setInternalContent(content);

	msg->getPrivate()->setTime(message->time);
	string callId = op->getCallId();
	ostringstream messageId;
	if (op->hasDialog()) {
		/* If this message has been received part of a dialog (which is unlikely to happen for IM),
		 * set an IMDN Message ID abitrary to be the SIP Call-ID followed by the CSeq number.
		 * This avoids considering incoming SIP MESSAGE received within a dialog as being duplicates. */
		messageId << callId << "-" << op->getRemoteCSeq();
	} else {
		messageId << callId;
	}

	// Don't do it for flexisip backend chat rooms, we need to know if the real message id from CPIM was retrieved or
	// not Based on that we will send IMDNs or not In case CPIM was enabled on a Basic chat room, IMDN message ID will
	// be overwritten by real one
	const bool isBasicChatRoom = (getCurrentParams()->getChatParams()->getBackend() == ChatParams::Backend::Basic);
	if (isBasicChatRoom) {
		msg->getPrivate()->setImdnMessageId(messageId.str());
	}
	msg->getPrivate()->setMessageId(messageId.str());
	msg->getPrivate()->setCallId(callId);

	const SalCustomHeader *ch = op->getRecvCustomHeaders();
	if (ch) msg->getPrivate()->setSalCustomHeaders(sal_custom_header_clone(ch));

	addTransientChatMessage(msg);

	return msg;
}

LinphoneReason ChatRoom::onSipMessageReceived(SalOp *op, const SalMessage *message) {
	auto msg = getMessageFromSal(op, message);
	LinphoneReason reason = msg->getPrivate()->receive();
	return reason;
}

void ChatRoom::onChatMessageReceived(const shared_ptr<ChatMessage> &chatMessage) {
	shared_ptr<Core> core = getCore();
	LinphoneCore *cCore = core->getCCore();

	if (chatMessage->getPrivate()->getContentType() == ContentType::ImIsComposing) {
		onIsComposingReceived(chatMessage->getFromAddress(), chatMessage->getPrivate()->getText());
		if (linphone_config_get_int(linphone_core_get_config(cCore), "sip", "deliver_imdn", 0) != 1) return;
	} else if (chatMessage->getPrivate()->getContentType() == ContentType::Imdn) {
		onImdnReceived(chatMessage);
		if (linphone_config_get_int(linphone_core_get_config(cCore), "sip", "deliver_imdn", 0) != 1) return;
	}

	const std::shared_ptr<Address> &fromAddress = chatMessage->getFromAddress();
	if ((chatMessage->getPrivate()->getContentType() != ContentType::ImIsComposing) &&
	    (chatMessage->getPrivate()->getContentType() != ContentType::Imdn)) {
		mIsComposingHandler->stopRemoteRefreshTimer(fromAddress->toString());
		notifyIsComposingReceived(fromAddress, false);
	}

	if (core->isCurrentlyAggregatingChatMessages()) {
		lDebug() << "[Chat Room] [" << getConferenceId()
		         << "] Core is currently aggregating chat messages, push message to list";
		chatMessage->setInAggregationQueue(true);
		aggregatedMessages.push_back(chatMessage);
	} else {
		// No aggregation, notify right away
		lDebug() << "[Chat Room] [" << getConferenceId() << "] No aggregation, notify right away";

		// Need to notify using aggregated callback even if there is only on message
		// This can happen when auto download is enabled and auto download takes longer than the aggregation delay
		if (core->canAggregateChatMessages()) {
			aggregatedMessages.push_back(chatMessage);
			notifyAggregatedChatMessages();
		} else {
			notifyMessageReceived(chatMessage);
		}
	}
}

void ChatRoom::notifyMessageReceived(const shared_ptr<ChatMessage> &chatMessage) {
	shared_ptr<ConferenceChatMessageEvent> event =
	    make_shared<ConferenceChatMessageEvent>(::time(nullptr), chatMessage);
	_linphone_chat_room_notify_chat_message_received(getCChatRoom(), L_GET_C_BACK_PTR(event));
	// Legacy.
	notifyChatMessageReceived(chatMessage);

	// Notify delivery
	sendDeliveryNotification(chatMessage);
}

void ChatRoom::notifyAggregatedChatMessages() {
	if (aggregatedMessages.empty()) {
		lDebug() << "[Chat Room] [" << getConferenceId() << "] No aggregated message to notify";
		return;
	}

	size_t aggregatedMessagesSize = aggregatedMessages.size();
	if (aggregatedMessagesSize == 1) {
		lDebug() << "[Chat Room] [" << getConferenceId() << "] There is 1 aggregated message to notify";
	} else {
		lDebug() << "[Chat Room] [" << getConferenceId() << "] There are " << aggregatedMessagesSize
		         << " aggregated messages to notify";
	}
	LinphoneChatRoom *cChatRoom = getCChatRoom();
	auto core = getCore()->getCCore();

	// Notify as ChatMessages
	bctbx_list_t *cMessages = L_GET_RESOLVED_C_LIST_FROM_CPP_LIST(aggregatedMessages);
	_linphone_chat_room_notify_messages_received(cChatRoom, cMessages);
	linphone_core_notify_messages_received(core, cChatRoom, cMessages);

	// Notify as Events
	std::list<std::shared_ptr<ConferenceChatMessageEvent>> eventsList;
	for (auto &chatMessage : aggregatedMessages) {
		shared_ptr<ConferenceChatMessageEvent> event =
		    make_shared<ConferenceChatMessageEvent>(::time(nullptr), chatMessage);
		eventsList.push_back(event);
		chatMessage->setInAggregationQueue(false);
	}
	bctbx_list_t *cEvents = L_GET_RESOLVED_C_LIST_FROM_CPP_LIST(eventsList);
	_linphone_chat_room_notify_chat_messages_received(cChatRoom, cEvents);

	// Notify delivery - do the same things as when chat messages are not aggregated: send the delivery notification
	// when the message is in the Delivered state
	for (auto &chatMessage : aggregatedMessages) {
		chatMessage->getPrivate()->setParticipantState(getMe()->getAddress(), ChatMessage::State::Delivered,
		                                               ::ms_time(nullptr));
		sendDeliveryNotification(chatMessage);
	}

	bctbx_list_free_with_data(cMessages, (bctbx_list_free_func)linphone_chat_message_unref);
	bctbx_list_free_with_data(cEvents, (bctbx_list_free_func)linphone_event_log_unref);

	aggregatedMessages.clear();
}

void ChatRoom::onImdnReceived(const shared_ptr<ChatMessage> &chatMessage) {
	Imdn::parse(chatMessage);
}

void ChatRoom::onIsComposingReceived(const std::shared_ptr<Address> &remoteAddress, const string &text) {
	mIsComposingHandler->parse(remoteAddress, text);
}

void ChatRoom::onIsComposingRefreshNeeded() {
	sendIsComposingNotification();
}

void ChatRoom::onIsComposingStateChanged(bool isComposing) {
	mIsComposing = isComposing;
	sendIsComposingNotification();
}

void ChatRoom::onIsRemoteComposingStateChanged(const std::shared_ptr<Address> &remoteAddress, bool isComposing) {
	notifyIsComposingReceived(remoteAddress, isComposing);
}

void ChatRoom::addPendingMessage(BCTBX_UNUSED(const std::shared_ptr<ChatMessage> &chatMessage)) {
	lInfo() << __func__ << ": not implemented";
}

// -----------------------------------------------------------------------------

LinphoneChatRoom *ChatRoom::getCChatRoom() {
	return toC();
}

const std::shared_ptr<Address> &ChatRoom::getPeerAddress() const {
	return getConferenceId().getPeerAddress();
}

const std::shared_ptr<Address> &ChatRoom::getLocalAddress() const {
	return getConferenceId().getLocalAddress();
}

// -----------------------------------------------------------------------------

time_t ChatRoom::getCreationTime() const {
	return creationTime;
}

time_t ChatRoom::getLastUpdateTime() const {
	return lastUpdateTime;
}

// -----------------------------------------------------------------------------
ChatRoom::SecurityLevel ChatRoom::getSecurityLevel() const {
	return ChatRoom::SecurityLevel::ClearText;
}

// -----------------------------------------------------------------------------

list<shared_ptr<Content>> ChatRoom::getMediaContents() const {
	return getCore()->getPrivate()->mainDb->getMediaContents(getConferenceId());
}

list<shared_ptr<Content>> ChatRoom::getDocumentContents() const {
	return getCore()->getPrivate()->mainDb->getDocumentContents(getConferenceId());
}

list<shared_ptr<EventLog>> ChatRoom::getMessageHistory(int nLast) const {
	return getCore()->getPrivate()->mainDb->getHistory(getConferenceId(), nLast,
	                                                   MainDb::Filter::ConferenceChatMessageFilter);
}

list<shared_ptr<EventLog>> ChatRoom::getMessageHistoryRange(int begin, int end) const {
	return getCore()->getPrivate()->mainDb->getHistoryRange(getConferenceId(), begin, end,
	                                                        MainDb::Filter::ConferenceChatMessageFilter);
}

list<shared_ptr<ChatMessage>> ChatRoom::getUnreadChatMessages() const {
	return getCore()->getPrivate()->mainDb->getUnreadChatMessages(getConferenceId());
}

int ChatRoom::getMessageHistorySize() const {
	return getCore()->getPrivate()->mainDb->getHistorySize(getConferenceId(),
	                                                       MainDb::Filter::ConferenceChatMessageFilter);
}

list<shared_ptr<EventLog>> ChatRoom::getHistory(int nLast) const {
	return getCore()->getPrivate()->mainDb->getHistory(
	    getConferenceId(), nLast,
	    MainDb::FilterMask(
	        {MainDb::Filter::ConferenceChatMessageFilter, MainDb::Filter::ConferenceInfoNoDeviceFilter}));
}

list<shared_ptr<EventLog>> ChatRoom::getHistory(int nLast, HistoryFilterMask filters) const {
	return getCore()->getPrivate()->mainDb->getHistory(getConferenceId(), nLast,
	                                                   MainDb::getFilterMaskFromHistoryFilterMask(filters));
}

list<shared_ptr<EventLog>> ChatRoom::getHistoryRange(int begin, int end) const {
	return getCore()->getPrivate()->mainDb->getHistoryRange(
	    getConferenceId(), begin, end,
	    MainDb::FilterMask(
	        {MainDb::Filter::ConferenceChatMessageFilter, MainDb::Filter::ConferenceInfoNoDeviceFilter}));
}

list<shared_ptr<EventLog>> ChatRoom::getHistoryRange(int begin, int end, HistoryFilterMask filters) const {
	return getCore()->getPrivate()->mainDb->getHistoryRange(getConferenceId(), begin, end,
	                                                        MainDb::getFilterMaskFromHistoryFilterMask(filters));
}

list<shared_ptr<EventLog>> ChatRoom::getHistoryRangeNear(unsigned int before,
                                                         unsigned int after,
                                                         const shared_ptr<EventLog> &event,
                                                         HistoryFilterMask filters) const {
	return getCore()->getPrivate()->mainDb->getHistoryRangeNear(getConferenceId(), before, after, event,
	                                                            MainDb::getFilterMaskFromHistoryFilterMask(filters));
}

list<shared_ptr<EventLog>> ChatRoom::getHistoryRangeBetween(const shared_ptr<EventLog> &firstEvent,
                                                            const shared_ptr<EventLog> &lastEvent,
                                                            HistoryFilterMask filters) const {
	return getCore()->getPrivate()->mainDb->getHistoryRangeBetween(getConferenceId(), firstEvent, lastEvent,
	                                                               MainDb::getFilterMaskFromHistoryFilterMask(filters));
}

int ChatRoom::getHistorySize() const {
	return getCore()->getPrivate()->mainDb->getHistorySize(getConferenceId());
}

int ChatRoom::getHistorySize(HistoryFilterMask filters) const {
	return getCore()->getPrivate()->mainDb->getHistorySize(getConferenceId(),
	                                                       MainDb::getFilterMaskFromHistoryFilterMask(filters));
}

void ChatRoom::deleteFromDb() {
	// Keep a ref, otherwise the object might be destroyed before we can set the Deleted state
	shared_ptr<AbstractChatRoom> ref = this->getSharedFromThis();
	Core::deleteChatRoom(ref);
	setState(ConferenceInterface::State::Deleted);

	// Clear all transient events after deleting the chatroom.
	// The application might still keep a reference to the chatroom, therefore the destructor may not be called
	// immediately after the chatroom reference is freed by the core
	remoteIsComposing.clear();
	transientEvents.clear();
	transientMessages.clear();
	aggregatedMessages.clear();
}

void ChatRoom::deleteHistory() {
	getCore()->getPrivate()->mainDb->cleanHistory(getConferenceId());
	setIsEmpty(true);
}

void ChatRoom::deleteMessageFromHistory(const shared_ptr<ChatMessage> &message) {
	shared_ptr<LinphonePrivate::EventLog> event =
	    LinphonePrivate::MainDb::getEvent(getCore()->getPrivate()->mainDb, message->getStorageId());
	if (event) {
		LinphonePrivate::EventLog::deleteFromDatabase(event);
		setIsEmpty(getCore()->getPrivate()->mainDb->isChatRoomEmpty(getConferenceId()));
	}
}

shared_ptr<ChatMessage> ChatRoom::getLastChatMessageInHistory() const {
	return getCore()->getPrivate()->mainDb->getLastChatMessage(getConferenceId());
}

bool ChatRoom::isEmpty() const {
	return mEmpty;
}

int ChatRoom::getChatMessageCount() const {
	return getCore()->getPrivate()->mainDb->getChatMessageCount(getConferenceId());
}

int ChatRoom::getUnreadChatMessageCount() const {
	return getCore()->getPrivate()->mainDb->getUnreadChatMessageCount(getConferenceId());
}

// -----------------------------------------------------------------------------

void ChatRoom::compose() {
	if (!mIsComposing) {
		mIsComposing = true;
		sendIsComposingNotification();
		mIsComposingHandler->startRefreshTimer();
	}
	mIsComposingHandler->startIdleTimer();
}

bool ChatRoom::isRemoteComposing() const {
	return !remoteIsComposing.empty();
}

list<std::shared_ptr<Address>> ChatRoom::getComposingAddresses() const {
	return remoteIsComposing;
}

// -----------------------------------------------------------------------------

shared_ptr<ChatMessage> ChatRoom::createChatMessage() {
	return createChatMessage(ChatMessage::Direction::Outgoing);
}

// Deprecated
shared_ptr<ChatMessage> ChatRoom::createChatMessage(const string &text) {
	shared_ptr<ChatMessage> chatMessage = createChatMessage();
	auto content = Content::create();
	content->setContentType(ContentType::PlainText);
	content->setBodyFromLocale(text);
	chatMessage->addContent(content);
	return chatMessage;
}

shared_ptr<ChatMessage> ChatRoom::createChatMessageFromUtf8(const string &text) {
	shared_ptr<ChatMessage> chatMessage = createChatMessage();
	auto content = Content::create();
	content->setContentType(ContentType::PlainText);
	content->setBodyFromUtf8(text);
	chatMessage->addContent(content);
	return chatMessage;
}

shared_ptr<ChatMessage> ChatRoom::createFileTransferMessage(const std::shared_ptr<FileContent> &content) {
	shared_ptr<ChatMessage> chatMessage = createChatMessage();
	chatMessage->addContent(content);
	return chatMessage;
}

shared_ptr<ChatMessage> ChatRoom::createForwardMessage(const shared_ptr<ChatMessage> &msg) {
	shared_ptr<ChatMessage> chatMessage = createChatMessage();
	for (const auto &c : msg->getContents()) {
		chatMessage->addContent(c->clone()->toSharedPtr());
	}

	// set forward info
	std::string fInfo;
	bool hidden = linphone_core_is_sender_name_hidden_in_forward_message(getCore()->getCCore());
	if (hidden) {
		fInfo = "Anonymous";
	} else {
		fInfo = msg->getForwardInfo().empty() ? msg->getFromAddress()->asStringUriOnly() : msg->getForwardInfo();
	}

	chatMessage->getPrivate()->setForwardInfo(fInfo);

	return chatMessage;
}

shared_ptr<ChatMessage> ChatRoom::createReplyMessage(const shared_ptr<ChatMessage> &msg) {
	shared_ptr<ChatMessage> chatMessage = createChatMessage();
	chatMessage->getPrivate()->setReplyToMessageIdAndSenderAddress(msg->getImdnMessageId(), msg->getFromAddress());
	return chatMessage;
}

// -----------------------------------------------------------------------------
bool ChatRoom::isSubscriptionUnderWay() const {
	const auto conference = getConference();
	if (!conference) {
		return false;
	}
	return conference->isSubscriptionUnderWay();
}

bool ChatRoom::isMe(const std::shared_ptr<Address> &address) const {
	const auto conference = getConference();
	if (!conference) {
		return false;
	}
	return conference->isMe(address);
}

std::shared_ptr<Participant> ChatRoom::getMe() const {
	const auto conference = getConference();
	if (!conference) {
		return nullptr;
	}
	return conference->getMe();
}

std::shared_ptr<Address> ChatRoom::getConferenceAddress() const {
	const auto conference = getConference();
	if (!conference) {
		return nullptr;
	}
	return conference->getConferenceAddress();
}

std::shared_ptr<Participant> ChatRoom::findParticipant(const std::shared_ptr<Address> &address) const {
	const auto conference = getConference();
	if (!conference) {
		return nullptr;
	}
	return conference->findParticipant(address);
}

std::list<std::shared_ptr<Participant>> ChatRoom::getParticipants() const {
	const auto conference = getConference();
	if (!conference) {
		return std::list<std::shared_ptr<Participant>>();
	}
	return conference->getParticipants();
}

std::list<std::shared_ptr<Address>> ChatRoom::getParticipantAddresses() const {
	const auto conference = getConference();
	if (!conference) {
		return {};
	}
	return conference->getParticipantAddresses();
}

std::optional<std::reference_wrapper<const std::string>> ChatRoom::getIdentifier() const {
	const auto conference = getConference();
	if (!conference) {
		return std::nullopt;
	}
	return conference->getIdentifier();
}

std::shared_ptr<ConferenceParams> ChatRoom::getCurrentParams() const {
	const auto conference = getConference();
	if (!conference) {
		return nullptr;
	}
	auto &params = conference->getCurrentParams();
	shared_ptr<Call> call = getCall();
	if (call && call->getCurrentParams()) {
		params->getChatParams()->setRealTimeText(call->getCurrentParams()->realtimeTextEnabled());
	}
	return params;
}

void ChatRoom::enableEphemeral(BCTBX_UNUSED(bool ephem), BCTBX_UNUSED(bool updateDb)) {
	lDebug() << "Ephemeral message is only supported in conference based chat room!";
}

bool ChatRoom::ephemeralEnabled() const {
	return false;
}

void ChatRoom::setEphemeralLifetime(BCTBX_UNUSED(long lifetime), BCTBX_UNUSED(bool updateDb)) {
	lError() << "Ephemeral message is only supported in conference based chat room!";
}

long ChatRoom::getEphemeralLifetime() const {
	return 0;
}

void ChatRoom::setEphemeralMode(BCTBX_UNUSED(AbstractChatRoom::EphemeralMode mode), BCTBX_UNUSED(bool updateDb)) {
	lError() << "Ephemeral message is only supported in conference based chat room!";
}

AbstractChatRoom::EphemeralMode ChatRoom::getEphemeralMode() const {
	lError() << "Ephemeral message is only supported in conference based chat room!";
	return AbstractChatRoom::EphemeralMode::DeviceManaged;
}

bool ChatRoom::ephemeralSupportedByAllParticipants() const {
	return false;
}

uint32_t ChatRoom::getChar() {
	try {
		uint32_t character = 0;

		if (mReadCharacterIndex < mReceivedRttCharacters.size()) {
			character = mReceivedRttCharacters.at(mReadCharacterIndex);
			mReadCharacterIndex += 1;
		}

		if (mReadCharacterIndex == mReceivedRttCharacters.size()) {
			mReadCharacterIndex = 0;
			mReceivedRttCharacters.clear();
		}

		return character;
	} catch (std::out_of_range &) {
		return 0;
	}
}

void ChatRoom::setCallId(const std::string &value) {
	mCallId = value;
}

std::shared_ptr<Call> ChatRoom::getCall() const {
	return getCore()->getCallByCallId(mCallId);
}

bool ChatRoom::getIsMuted() const {
	return mIsMuted;
}

void ChatRoom::setIsMuted(const bool muted, const bool updateDb) {
	if (muted != mIsMuted) {
		mIsMuted = muted;
		if (updateDb) {
			getCore()->getPrivate()->mainDb->updateChatRoomMutedState(getConferenceId(), muted);
		}
	}
}

std::shared_ptr<ConferenceInfo> ChatRoom::getConferenceInfo() const {
	const auto conference = getConference();
	if (!conference) {
		return nullptr;
	}
	return conference->createOrGetConferenceInfo();
}

ChatRoomLogContextualizer::ChatRoomLogContextualizer(const LinphoneChatRoom *cr)
    : CoreLogContextualizer(*AbstractChatRoom::toCpp(cr)) {
}

LINPHONE_END_NAMESPACE
