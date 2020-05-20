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

#include <algorithm>

#include "linphone/utils/utils.h"
#include "linphone/utils/algorithm.h"

#include "c-wrapper/c-wrapper.h"
#include "chat/chat-message/chat-message-p.h"
#include "chat/chat-message/imdn-message.h"
#include "chat/chat-message/is-composing-message.h"
#include "chat/chat-message/notification-message-p.h"
#include "chat/chat-room/chat-room-p.h"
#include "core/core-p.h"
#include "logger/logger.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

void ChatRoomPrivate::setState (ChatRoom::State newState) {
	if (state != newState) {
		state = newState;
		notifyStateChanged();
	}
}

// -----------------------------------------------------------------------------

void ChatRoomPrivate::sendChatMessage (const shared_ptr<ChatMessage> &chatMessage) {
	L_Q();

	ChatMessagePrivate *dChatMessage = chatMessage->getPrivate();
	dChatMessage->setTime(ms_time(0));
	if (!q->canHandleCpim()) {
		//if not using cpim, ImdnMessageId = SIP Message call id, so should be computed each time, specially in case of resend.
		dChatMessage->setImdnMessageId("");
	}
	dChatMessage->send();
}

void ChatRoomPrivate::onChatMessageSent(const shared_ptr<ChatMessage> &chatMessage) {
	L_Q();

	ChatMessagePrivate *dChatMessage = chatMessage->getPrivate();
	LinphoneChatRoom *cr = getCChatRoom();

	// TODO: server currently don't stock message, remove condition in the future.
	if (!linphone_core_conference_server_enabled(q->getCore()->getCCore())) {
		shared_ptr<ConferenceChatMessageEvent> event = static_pointer_cast<ConferenceChatMessageEvent>(
			q->getCore()->getPrivate()->mainDb->getEventFromKey(dChatMessage->dbKey)
		);
		if (!event) {
			event = make_shared<ConferenceChatMessageEvent>(time(nullptr), chatMessage);
		}
		
		_linphone_chat_room_notify_chat_message_sent(cr, L_GET_C_BACK_PTR(event));
		linphone_core_notify_message_sent(q->getCore()->getCCore(), cr, L_GET_C_BACK_PTR(chatMessage));
	}

	if (isComposing) isComposing = false;
	isComposingHandler->stopIdleTimer();
	isComposingHandler->stopRefreshTimer();
}

void ChatRoomPrivate::sendIsComposingNotification () {
	L_Q();
	LinphoneImNotifPolicy *policy = linphone_core_get_im_notif_policy(q->getCore()->getCCore());
	if (!linphone_im_notif_policy_get_send_is_composing(policy))
		return;

	auto isComposingMsg = createIsComposingMessage();
	isComposingMsg->getPrivate()->send();
}

// -----------------------------------------------------------------------------

void ChatRoomPrivate::addEvent (const shared_ptr<EventLog> &eventLog) {
	L_Q();

	q->getCore()->getPrivate()->mainDb->addEvent(eventLog);

	EventLog::Type type = eventLog->getType();
	if (type == EventLog::Type::ConferenceParticipantDeviceAdded
		|| type == EventLog::Type::ConferenceParticipantDeviceRemoved) {
		// Do not update last event time on the chat room for those events
		// because they are invisible and will cause the chat room to move
		// up in the list and the user won't know why
	} else {
		setLastUpdateTime(eventLog->getCreationTime());
		if (type == EventLog::Type::ConferenceChatMessage) {
			setIsEmpty(false);
		}
	}
}

void ChatRoomPrivate::addTransientEvent (const shared_ptr<EventLog> &eventLog) {
	auto it = find(transientEvents, eventLog);
	if (it == transientEvents.end())
		transientEvents.push_back(eventLog);
}

void ChatRoomPrivate::removeTransientEvent (const shared_ptr<EventLog> &eventLog) {
	auto it = find(transientEvents, eventLog);
	if (it != transientEvents.end())
		transientEvents.erase(it);
}

void ChatRoomPrivate::addTransientChatMessage (const shared_ptr<ChatMessage> &message) {
	auto it = find(transientMessages, message);
	if (it == transientMessages.end())
		transientMessages.push_back(message);
}

void ChatRoomPrivate::removeTransientChatMessage (const shared_ptr<ChatMessage> &message) {
	auto it = find(transientMessages, message);
	if (it != transientMessages.end())
		transientMessages.erase(it);
}

void ChatRoomPrivate::setIsEmpty (const bool empty) {
	isEmpty = empty;
}

// -----------------------------------------------------------------------------

shared_ptr<ChatMessage> ChatRoomPrivate::createChatMessage (ChatMessage::Direction direction) {
	L_Q();
	shared_ptr<ChatMessage> message = shared_ptr<ChatMessage>(new ChatMessage(q->getSharedFromThis(), direction));
	if (q->ephemeralEnabled() && direction == ChatMessage::Direction::Outgoing) {
		lDebug() << "Create an outgoing ephemeral message " << message << " with lifetime " << q->getEphemeralLifetime() << " in chat room [" << conferenceId << "]";
		message->getPrivate()->enableEphemeralWithTime(q->getEphemeralLifetime());
	}
	return message;
}

shared_ptr<ImdnMessage> ChatRoomPrivate::createImdnMessage (
	const list<shared_ptr<ChatMessage>> &deliveredMessages,
	const list<shared_ptr<ChatMessage>> &displayedMessages
) {
	L_Q();
	return shared_ptr<ImdnMessage>(new ImdnMessage(q->getSharedFromThis(), deliveredMessages, displayedMessages));
}

shared_ptr<ImdnMessage> ChatRoomPrivate::createImdnMessage (const list<Imdn::MessageReason> &nonDeliveredMessages) {
	L_Q();
	return shared_ptr<ImdnMessage>(new ImdnMessage(q->getSharedFromThis(), nonDeliveredMessages));
}

shared_ptr<ImdnMessage> ChatRoomPrivate::createImdnMessage (const shared_ptr<ImdnMessage> &message) {
	return shared_ptr<ImdnMessage>(new ImdnMessage(message));
}

shared_ptr<IsComposingMessage> ChatRoomPrivate::createIsComposingMessage () {
	L_Q();
	return shared_ptr<IsComposingMessage>(new IsComposingMessage(q->getSharedFromThis(), *isComposingHandler.get(), isComposing));
}

list<shared_ptr<ChatMessage>> ChatRoomPrivate::findChatMessages (const string &messageId) const {
	L_Q();
	return q->getCore()->getPrivate()->mainDb->findChatMessages(q->getConferenceId(), messageId);
}

// -----------------------------------------------------------------------------

void ChatRoomPrivate::sendDeliveryErrorNotification (const shared_ptr<ChatMessage> &chatMessage, LinphoneReason reason) {
	LinphoneImNotifPolicy *policy = linphone_core_get_im_notif_policy(chatMessage->getCore()->getCCore());
	ChatMessagePrivate *dChatMessage = chatMessage->getPrivate();
	if (
		linphone_im_notif_policy_get_send_imdn_delivered(policy) &&
		chatMessage->getPrivate()->getNegativeDeliveryNotificationRequired()
	) {
		dChatMessage->setNegativeDeliveryNotificationRequired(false);
		imdnHandler->notifyDeliveryError(chatMessage, reason);
	}
}

void ChatRoomPrivate::sendDeliveryNotification (const shared_ptr<ChatMessage> &chatMessage) {
	LinphoneImNotifPolicy *policy = linphone_core_get_im_notif_policy(chatMessage->getCore()->getCCore());
	ChatMessagePrivate *dChatMessage = chatMessage->getPrivate();
	if (
		linphone_im_notif_policy_get_send_imdn_delivered(policy) &&
		dChatMessage->getPositiveDeliveryNotificationRequired()
	) {
		dChatMessage->setPositiveDeliveryNotificationRequired(false);
		imdnHandler->notifyDelivery(chatMessage);
	}
}

void ChatRoomPrivate::sendDeliveryNotifications (const std::shared_ptr<ChatMessage> &chatMessage) {
	ChatMessagePrivate *dChatMessage = chatMessage->getPrivate();
	if (dChatMessage->getPositiveDeliveryNotificationRequired()) {
		dChatMessage->setPositiveDeliveryNotificationRequired(false);
		imdnHandler->notifyDelivery(chatMessage);
	}
}

void ChatRoomPrivate::sendDisplayNotification (const shared_ptr<ChatMessage> &chatMessage) {
	L_Q();
	LinphoneImNotifPolicy *policy = linphone_core_get_im_notif_policy(q->getCore()->getCCore());
	ChatMessagePrivate *dChatMessage = chatMessage->getPrivate();
	if (
		linphone_im_notif_policy_get_send_imdn_displayed(policy) &&
		chatMessage->getPrivate()->getDisplayNotificationRequired()
	) {
		dChatMessage->setPositiveDeliveryNotificationRequired(false);
		dChatMessage->setDisplayNotificationRequired(false);
		imdnHandler->notifyDisplay(chatMessage);
	}
}

// -----------------------------------------------------------------------------

void ChatRoomPrivate::notifyChatMessageReceived (const shared_ptr<ChatMessage> &chatMessage) {
	L_Q();
	LinphoneChatRoom *cr = getCChatRoom();
	if (!chatMessage->getPrivate()->getText().empty()) {
		/* Legacy API */
		LinphoneAddress *fromAddress = linphone_address_new(chatMessage->getFromAddress().asString().c_str());
		linphone_core_notify_text_message_received(
			q->getCore()->getCCore(),
			cr,
			fromAddress,
			chatMessage->getPrivate()->getText().c_str()
		);
		linphone_address_unref(fromAddress);
	}
	_linphone_chat_room_notify_message_received(cr, L_GET_C_BACK_PTR(chatMessage));
	linphone_core_notify_message_received(q->getCore()->getCCore(), cr, L_GET_C_BACK_PTR(chatMessage));
}

void ChatRoomPrivate::notifyIsComposingReceived (const Address &remoteAddress, bool isComposing) {
	L_Q();

	if (isComposing) {
		auto it = find(remoteIsComposing.cbegin(), remoteIsComposing.cend(), remoteAddress);
		if (it == remoteIsComposing.cend())
			remoteIsComposing.push_back(remoteAddress);
	} else {
		remoteIsComposing.remove(remoteAddress);
	}

	LinphoneChatRoom *cr = getCChatRoom();
	LinphoneAddress *lAddr = linphone_address_new(remoteAddress.asString().c_str());
	_linphone_chat_room_notify_is_composing_received(cr, lAddr, !!isComposing);
	linphone_address_unref(lAddr);
	// Legacy notification
	linphone_core_notify_is_composing_received(q->getCore()->getCCore(), cr);
}

void ChatRoomPrivate::notifyStateChanged () {
	L_Q();
	LinphoneChatRoom *cr = getCChatRoom();
	// Do not output this log while Core is starting up, a lot of them may happen
	if (q->getCore()->getCCore()->state == LinphoneGlobalStartup) {
		lDebug() << "Chat room [" << q->getConferenceId() << "] state changed to: " << Utils::toString(state);
	} else {
		lInfo() << "Chat room [" << q->getConferenceId() << "] state changed to: " << state;
	}
	linphone_core_notify_chat_room_state_changed(q->getCore()->getCCore(), cr, (LinphoneChatRoomState)state);
	_linphone_chat_room_notify_state_changed(cr, (LinphoneChatRoomState)state);
}

void ChatRoomPrivate::notifyUndecryptableChatMessageReceived (const shared_ptr<ChatMessage> &chatMessage) {
	L_Q();
	LinphoneChatRoom *cr = getCChatRoom();
	_linphone_chat_room_notify_undecryptable_message_received(cr, L_GET_C_BACK_PTR(chatMessage));
	linphone_core_notify_message_received_unable_decrypt(q->getCore()->getCCore(), cr, L_GET_C_BACK_PTR(chatMessage));
}

// -----------------------------------------------------------------------------

LinphoneReason ChatRoomPrivate::onSipMessageReceived (SalOp *op, const SalMessage *message) {
	L_Q();

	LinphoneReason reason = LinphoneReasonNone;
	shared_ptr<ChatMessage> msg;

	msg = createChatMessage(
		IdentityAddress(op->getFrom()) == q->getLocalAddress()
			? ChatMessage::Direction::Outgoing
			: ChatMessage::Direction::Incoming
	);

	Content content;
	if (message->url && ContentType(message->content_type) == ContentType::ExternalBody) {
		lInfo() << "Received a message with an external body URL " << message->url;
		content.setContentType(ContentType::FileTransfer);
		content.setBody(msg->getPrivate()->createFakeFileTransferFromUrl(message->url));
	} else {
		content.setContentType(ContentType(message->content_type));
		content.setBodyFromUtf8(message->text ? message->text : "");
	}
	msg->setInternalContent(content);

	msg->getPrivate()->setTime(message->time);
	ostringstream messageId;
	if (op->hasDialog()){
		/* If this message has been received part of a dialog (which is unlikely to happen for IM),
		 * set an IMDN Message ID abitrary to be the SIP Call-ID followed by the CSeq number.
		 * This avoids considering incoming SIP MESSAGE received within a dialog as being duplicates. */
		messageId << op->getCallId() << "-" << op->getRemoteCSeq();
	}else{
		messageId << op->getCallId();
	}
	msg->getPrivate()->setImdnMessageId(messageId.str());
	msg->getPrivate()->setCallId(op->getCallId());

	const SalCustomHeader *ch = op->getRecvCustomHeaders();
	if (ch)
		msg->getPrivate()->setSalCustomHeaders(sal_custom_header_clone(ch));

	addTransientChatMessage(msg);
	reason = msg->getPrivate()->receive();
	return reason;
}

void ChatRoomPrivate::onChatMessageReceived (const shared_ptr<ChatMessage> &chatMessage) {
	L_Q();

	shared_ptr<Core> core = q->getCore();
	LinphoneCore *cCore = core->getCCore();

	if (chatMessage->getPrivate()->getContentType() == ContentType::ImIsComposing) {
		onIsComposingReceived(chatMessage->getFromAddress(), chatMessage->getPrivate()->getText());
		if (lp_config_get_int(linphone_core_get_config(cCore), "sip", "deliver_imdn", 0) != 1)
			return;
	} else if (chatMessage->getPrivate()->getContentType() == ContentType::Imdn) {
		onImdnReceived(chatMessage);
		if (lp_config_get_int(linphone_core_get_config(cCore), "sip", "deliver_imdn", 0) != 1)
			return;
	}

	const IdentityAddress &fromAddress = chatMessage->getFromAddress();
	if ((chatMessage->getPrivate()->getContentType() != ContentType::ImIsComposing)
		&& (chatMessage->getPrivate()->getContentType() != ContentType::Imdn)
	) {
		isComposingHandler->stopRemoteRefreshTimer(fromAddress.asString());
		notifyIsComposingReceived(fromAddress, false);
	}
	chatMessage->getPrivate()->notifyReceiving();
}

void ChatRoomPrivate::onImdnReceived (const shared_ptr<ChatMessage> &chatMessage) {
	Imdn::parse(chatMessage);
}

void ChatRoomPrivate::onIsComposingReceived (const Address &remoteAddress, const string &text) {
	isComposingHandler->parse(remoteAddress, text);
}

void ChatRoomPrivate::onIsComposingRefreshNeeded () {
	sendIsComposingNotification();
}

void ChatRoomPrivate::onIsComposingStateChanged (bool isComposing) {
	this->isComposing = isComposing;
	sendIsComposingNotification();
}

void ChatRoomPrivate::onIsRemoteComposingStateChanged (const Address &remoteAddress, bool isComposing) {
	notifyIsComposingReceived(remoteAddress, isComposing);
}

// -----------------------------------------------------------------------------

LinphoneChatRoom *ChatRoomPrivate::getCChatRoom () const {
	L_Q();
	if (proxyChatRoom)
		return L_GET_C_BACK_PTR(proxyChatRoom);
	else
		return L_GET_C_BACK_PTR(q);
}

// =============================================================================

ChatRoom::ChatRoom (ChatRoomPrivate &p, const shared_ptr<Core> &core, const ConferenceId &conferenceId, const std::shared_ptr<ChatRoomParams> &params) :
	AbstractChatRoom(p, core) {
	L_D();

	d->params = params;
	d->conferenceId = conferenceId;
	d->imdnHandler.reset(new Imdn(this));
	d->isComposingHandler.reset(new IsComposing(core->getCCore(), d));
}

ChatRoom::~ChatRoom () {
	L_D();

	if (d->params) {
		d->params.reset();
	}
	d->imdnHandler.reset();
}

// -----------------------------------------------------------------------------

const ConferenceId &ChatRoom::getConferenceId () const {
	L_D();
	return d->conferenceId;
}

const IdentityAddress &ChatRoom::getPeerAddress () const {
	L_D();
	return d->conferenceId.getPeerAddress();
}

const IdentityAddress &ChatRoom::getLocalAddress () const {
	L_D();
	return d->conferenceId.getLocalAddress();
}

// -----------------------------------------------------------------------------

time_t ChatRoom::getCreationTime () const {
	L_D();
	return d->creationTime;
}

time_t ChatRoom::getLastUpdateTime () const {
	L_D();
	return d->lastUpdateTime;
}

// -----------------------------------------------------------------------------

ChatRoom::State ChatRoom::getState () const {
	L_D();
	return d->state;
}

ChatRoom::SecurityLevel ChatRoom::getSecurityLevel () const {
	return ChatRoom::SecurityLevel::ClearText;
}

// -----------------------------------------------------------------------------

list<shared_ptr<EventLog>> ChatRoom::getMessageHistory (int nLast) const {
	return getCore()->getPrivate()->mainDb->getHistory(getConferenceId(), nLast, MainDb::Filter::ConferenceChatMessageFilter);
}

list<shared_ptr<EventLog>> ChatRoom::getMessageHistoryRange (int begin, int end) const {
	return getCore()->getPrivate()->mainDb->getHistoryRange(getConferenceId(), begin, end, MainDb::Filter::ConferenceChatMessageFilter);
}

int ChatRoom::getMessageHistorySize () const {
	return getCore()->getPrivate()->mainDb->getHistorySize(getConferenceId(), MainDb::Filter::ConferenceChatMessageFilter);
}

list<shared_ptr<EventLog>> ChatRoom::getHistory (int nLast) const {
	return getCore()->getPrivate()->mainDb->getHistory(
		getConferenceId(),
		nLast,
		MainDb::FilterMask({ MainDb::Filter::ConferenceChatMessageFilter, MainDb::Filter::ConferenceInfoNoDeviceFilter })
	);
}

list<shared_ptr<EventLog>> ChatRoom::getHistoryRange (int begin, int end) const {
	return getCore()->getPrivate()->mainDb->getHistoryRange(
		getConferenceId(),
		begin,
		end,
		MainDb::FilterMask({ MainDb::Filter::ConferenceChatMessageFilter, MainDb::Filter::ConferenceInfoNoDeviceFilter })
	);
}

int ChatRoom::getHistorySize () const {
	return getCore()->getPrivate()->mainDb->getHistorySize(getConferenceId());
}

void ChatRoom::deleteFromDb () {
	L_D();
	// Keep a ref, otherwise the object might be destroyed before we can set the Deleted state
	shared_ptr<AbstractChatRoom> ref = this->getSharedFromThis();
	Core::deleteChatRoom(ref);
	d->setState(ChatRoom::State::Deleted);
}

void ChatRoom::deleteHistory () {
	L_D();
	getCore()->getPrivate()->mainDb->cleanHistory(getConferenceId());
	d->setIsEmpty(true);
}

void ChatRoom::deleteMessageFromHistory (const shared_ptr<ChatMessage> &message) {
	L_D();
	shared_ptr<LinphonePrivate::EventLog> event = LinphonePrivate::MainDb::getEventFromKey(
		message->getPrivate()->dbKey
	);
	if (event) {
		LinphonePrivate::EventLog::deleteFromDatabase(event);
		d->setIsEmpty(getCore()->getPrivate()->mainDb->isChatRoomEmpty(getConferenceId()));
	}
}

shared_ptr<ChatMessage> ChatRoom::getLastChatMessageInHistory () const {
	return getCore()->getPrivate()->mainDb->getLastChatMessage(getConferenceId());
}

bool ChatRoom::isEmpty () const {
	L_D();

	return d->isEmpty;
}

int ChatRoom::getChatMessageCount () const {
	return getCore()->getPrivate()->mainDb->getChatMessageCount(getConferenceId());
}

int ChatRoom::getUnreadChatMessageCount () const {
	return getCore()->getPrivate()->mainDb->getUnreadChatMessageCount(getConferenceId());
}

// -----------------------------------------------------------------------------

void ChatRoom::compose () {
	L_D();
	if (!d->isComposing) {
		d->isComposing = true;
		d->sendIsComposingNotification();
		d->isComposingHandler->startRefreshTimer();
	}
	d->isComposingHandler->startIdleTimer();
}

bool ChatRoom::isRemoteComposing () const {
	L_D();
	return !d->remoteIsComposing.empty();
}

list<IdentityAddress> ChatRoom::getComposingAddresses () const {
	L_D();
	return d->remoteIsComposing;
}

// -----------------------------------------------------------------------------

shared_ptr<ChatMessage> ChatRoom::createChatMessage () {
	L_D();
	return d->createChatMessage(ChatMessage::Direction::Outgoing);
}

shared_ptr<ChatMessage> ChatRoom::createChatMessage (const string &text) {
	shared_ptr<ChatMessage> chatMessage = createChatMessage();
	Content *content = new Content();
	content->setContentType(ContentType::PlainText);
	content->setBody(text);
	chatMessage->addContent(content);
	return chatMessage;
}

shared_ptr<ChatMessage> ChatRoom::createFileTransferMessage (FileContent *content) {
	shared_ptr<ChatMessage> chatMessage = createChatMessage();
	chatMessage->addContent(content);
	return chatMessage;
}

shared_ptr<ChatMessage> ChatRoom::createForwardMessage (const shared_ptr<ChatMessage> &msg) {
	shared_ptr<ChatMessage> chatMessage = createChatMessage();
	for (const Content *c : msg->getContents()) {
		chatMessage->addContent(c->clone());
	}

	// set forward info
	std::string fInfo;
	bool hidden = linphone_core_is_sender_name_hidden_in_forward_message(getCore()->getCCore());
	if (hidden) {
		fInfo = "Anonymous";
	} else {
		fInfo = msg->getForwardInfo().empty()? msg->getFromAddress().asString():msg->getForwardInfo();
	}

	chatMessage->getPrivate()->setForwardInfo(fInfo);

	return chatMessage;
}
// -----------------------------------------------------------------------------

shared_ptr<ChatMessage> ChatRoom::findChatMessage (const string &messageId) const {
	L_D();
	list<shared_ptr<ChatMessage>> chatMessages = d->findChatMessages(messageId);
	return chatMessages.empty() ? nullptr : chatMessages.front();
}

shared_ptr<ChatMessage> ChatRoom::findChatMessage (const string &messageId, ChatMessage::Direction direction) const {
	L_D();
	for (auto &chatMessage : d->findChatMessages(messageId))
		if (chatMessage->getDirection() == direction)
			return chatMessage;
	return nullptr;
}

void ChatRoom::markAsRead () {
	L_D();

	CorePrivate *dCore = getCore()->getPrivate();
	for (auto &chatMessage : dCore->mainDb->getUnreadChatMessages(d->conferenceId)) {
		chatMessage->getPrivate()->markAsRead();
		// Do not set the message state has displayed if it contains a file transfer (to prevent imdn sending)
		if (!chatMessage->getPrivate()->hasFileTransferContent()) {
			chatMessage->getPrivate()->setState(ChatMessage::State::Displayed);
		}
	}

	dCore->mainDb->markChatMessagesAsRead(d->conferenceId);
	linphone_core_notify_chat_room_read(getCore()->getCCore(), d->getCChatRoom());
}

const std::shared_ptr<ChatRoomParams> &ChatRoom::getCurrentParams() const {
	L_D();

	return d->params;
}

void ChatRoom::enableEphemeral (bool ephem, bool updateDb) {
	lDebug() << "Ephemeral message is only supported in conference based chat room!";
}

bool ChatRoom::ephemeralEnabled() const {
	return false;
}

void ChatRoom::setEphemeralLifetime (long lifetime, bool updateDb) {
	lDebug() << "Ephemeral message is only supported in conference based chat room!";
}

long ChatRoom::getEphemeralLifetime () const {
	return 0;
}

bool ChatRoom::ephemeralSupportedByAllParticipants () const  {
	return false;
}

LINPHONE_END_NAMESPACE
