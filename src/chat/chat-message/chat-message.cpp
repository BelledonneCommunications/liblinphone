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

#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "chat/chat-message/chat-message-listener.h"
#include "chat/chat-message/chat-message-p.h"
#include "chat/chat-room/abstract-chat-room.h"
#include "chat/chat-room/chat-room.h"
#include "chat/ics/ics.h"
#include "chat/modifier/cpim-chat-message-modifier.h"
#include "chat/modifier/encryption-chat-message-modifier.h"
#include "chat/modifier/multipart-chat-message-modifier.h"
#include "chat/notification/imdn.h"
#include "conference/participant-imdn-state.h"
#include "conference/participant.h"
#include "content/content-disposition.h"
#include "content/content-manager.h"
#include "content/header/header-param.h"
#include "core/core-p.h"
#include "core/core.h"
#include "db/main-db-key-p.h"
#include "db/main-db-p.h"
#include "factory/factory.h"
#include "http/http-client.h"
#include "linphone/api/c-chat-message-cbs.h"
#include "linphone/api/c-chat-message-reaction.h"
#include "linphone/api/c-chat-message.h"
#include "linphone/api/c-conference-info.h"
#include "linphone/api/c-content.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include "linphone/utils/utils.h"
#include "linphone/wrapper_utils.h"
#include "logger/logger.h"
#include "object/object-p.h"
#include "sip-tools/sip-headers.h"
#include "utils/fsm-integrity-checker.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

static FsmIntegrityChecker<ChatMessage::State> chatMessageFsmChecker{
    {{ChatMessage::State::Idle,
      {ChatMessage::State::PendingDelivery, ChatMessage::State::InProgress, ChatMessage::State::Delivered,
       ChatMessage::State::NotDelivered, ChatMessage::State::DeliveredToUser, ChatMessage::State::Displayed,
       ChatMessage::State::FileTransferInProgress, ChatMessage::State::FileTransferError,
       ChatMessage::State::FileTransferDone}},
     {ChatMessage::State::InProgress,
      {ChatMessage::State::Delivered, ChatMessage::State::NotDelivered, ChatMessage::State::DeliveredToUser,
       ChatMessage::State::Displayed, ChatMessage::State::FileTransferInProgress, ChatMessage::State::FileTransferError,
       ChatMessage::State::FileTransferDone}},
     {ChatMessage::State::Delivered,
      {ChatMessage::State::InProgress, ChatMessage::State::NotDelivered, ChatMessage::State::DeliveredToUser,
       ChatMessage::State::Displayed, ChatMessage::State::FileTransferInProgress, ChatMessage::State::FileTransferError,
       ChatMessage::State::FileTransferDone}},
     {ChatMessage::State::PendingDelivery,
      {ChatMessage::State::Idle, ChatMessage::State::InProgress, ChatMessage::State::Delivered,
       ChatMessage::State::DeliveredToUser, ChatMessage::State::Displayed, ChatMessage::State::FileTransferInProgress,
       ChatMessage::State::FileTransferError, ChatMessage::State::FileTransferDone}},
     {ChatMessage::State::NotDelivered,
      {ChatMessage::State::Idle, ChatMessage::State::InProgress, ChatMessage::State::Delivered,
       ChatMessage::State::DeliveredToUser, ChatMessage::State::Displayed, ChatMessage::State::FileTransferInProgress,
       ChatMessage::State::FileTransferError, ChatMessage::State::FileTransferDone}},
     {ChatMessage::State::DeliveredToUser,
      {ChatMessage::State::InProgress, ChatMessage::State::Displayed, ChatMessage::State::FileTransferInProgress,
       ChatMessage::State::FileTransferError, ChatMessage::State::FileTransferDone}},
     {ChatMessage::State::Displayed,
      {ChatMessage::State::InProgress, ChatMessage::State::FileTransferInProgress,
       ChatMessage::State::FileTransferError, ChatMessage::State::FileTransferDone}},
     {ChatMessage::State::FileTransferCancelling,
      {ChatMessage::State::NotDelivered, ChatMessage::State::FileTransferError}},
     {ChatMessage::State::FileTransferInProgress,
      {ChatMessage::State::PendingDelivery, ChatMessage::State::InProgress, ChatMessage::State::NotDelivered,
       ChatMessage::State::Delivered, ChatMessage::State::DeliveredToUser, ChatMessage::State::Displayed,
       ChatMessage::State::FileTransferError, ChatMessage::State::FileTransferCancelling,
       ChatMessage::State::FileTransferDone}},
     {ChatMessage::State::FileTransferError,
      {ChatMessage::State::InProgress, ChatMessage::State::NotDelivered, ChatMessage::State::Delivered,
       ChatMessage::State::DeliveredToUser, ChatMessage::State::Displayed, ChatMessage::State::FileTransferInProgress,
       ChatMessage::State::FileTransferDone}},
     {ChatMessage::State::FileTransferDone,
      {ChatMessage::State::InProgress, ChatMessage::State::NotDelivered, ChatMessage::State::Delivered,
       ChatMessage::State::PendingDelivery, ChatMessage::State::DeliveredToUser, ChatMessage::State::Displayed,
       ChatMessage::State::FileTransferInProgress, ChatMessage::State::FileTransferError}}}};

ChatMessagePrivate::ChatMessagePrivate(const std::shared_ptr<AbstractChatRoom> &chatRoom, ChatMessage::Direction dir)
    : fileTransferChatMessageModifier(chatRoom->getCore()->getHttpClient().getProvider()) {
	direction = dir;
	setChatRoom(chatRoom);
}

ChatMessagePrivate::~ChatMessagePrivate() {
	if (salOp) {
		salOp->setUserPointer(nullptr);
		salOp->unref();
	}
	if (salCustomHeaders) sal_custom_header_unref(salCustomHeaders);
	if (errorInfo) linphone_error_info_unref(errorInfo);
}

void ChatMessagePrivate::setStorageId(long long id) {
	L_Q();

	if (id < 0) {
		// Negative IDs invalidate the message therefore it will be deleted from the cache
		q->deleteChatMessageFromCache();
	}

	storageId = id;
}

void ChatMessagePrivate::resetStorageId() {
	setStorageId(-1);
}

void ChatMessagePrivate::setDirection(ChatMessage::Direction dir) {
	direction = dir;
}

void ChatMessagePrivate::setTime(time_t t) {
	time = t;
}

void ChatMessagePrivate::setIsReadOnly(bool readOnly) {
	isReadOnly = readOnly;
}

void ChatMessagePrivate::markAsRead() {
	markedAsRead = true;
}

bool ChatMessagePrivate::isMarkedAsRead() const {
	return markedAsRead;
}

void ChatMessagePrivate::setParticipantState(const std::shared_ptr<Address> &participantAddress,
                                             ChatMessage::State newState,
                                             time_t stateChangeTime,
                                             LinphoneReason reason) {
	L_Q();

	const auto &chatRoom = dynamic_pointer_cast<ChatRoom>(q->getChatRoom());
	if (!chatRoom) return;

	const shared_ptr<ChatMessage> &sharedMessage = q->getSharedFromThis();
	const bool isBasicChatRoom =
	    (chatRoom->getCurrentParams()->getChatParams()->getBackend() == ChatParams::Backend::Basic);
	ChatMessage::State currentState = q->getParticipantState(participantAddress);
	const auto &conferenceAddress = chatRoom->getConferenceAddress();
	const auto conferenceAddressStr = conferenceAddress ? conferenceAddress->toString() : std::string("sip:");

	if (!chatMessageFsmChecker.isValid(currentState, newState)) {
		if (isBasicChatRoom) {
			lWarning() << "Chat message " << sharedMessage << ": Invalid transaction of message in basic chat room "
			           << conferenceAddressStr << " from state " << Utils::toString(currentState) << " to state "
			           << Utils::toString(newState);
		} else {
			lWarning() << "Chat message " << sharedMessage << ": Invalid transaction of participant "
			           << *participantAddress << " from state " << Utils::toString(currentState) << " to state "
			           << Utils::toString(newState);
		}
		return;
	}

	lDebug() << "Chat message " << sharedMessage << " of chat room " << chatRoom << " (" << conferenceAddressStr
	         << "): Moving participant " << *participantAddress << " from state " << Utils::toString(currentState)
	         << " to state " << Utils::toString(newState);

	auto me = chatRoom->getMe();
	const auto isMe = participantAddress->weakEqual(*me->getAddress());
	// Send IMDN if the participant whose state changes is me
	if (isMe) {
		switch (newState) {
			case ChatMessage::State::Displayed:
				chatRoom->sendDisplayNotification(sharedMessage);
				break;
			case ChatMessage::State::DeliveredToUser:
				chatRoom->sendDeliveryNotification(sharedMessage);
				break;
			case ChatMessage::State::NotDelivered:
				if (reason != LinphoneReasonNone) {
					chatRoom->sendDeliveryErrorNotification(sharedMessage, reason);
				}
				break;
			default:
				break;
		}
	}

	unique_ptr<MainDb> &mainDb = chatRoom->getCore()->getPrivate()->mainDb;
	shared_ptr<EventLog> eventLog = mainDb->getEvent(mainDb, q->getStorageId());
	if (!q->isValid() && (eventLog || (newState == ChatMessage::State::NotDelivered))) {
		if (newState == ChatMessage::State::NotDelivered) {
			setState(newState);
		}
		return;
	}

	// Participant states are not supported
	if (isBasicChatRoom) {
		setState(newState);
		return;
	}

	if (linphone_config_get_bool(linphone_core_get_config(chatRoom->getCore()->getCCore()), "misc",
	                             "enable_simple_group_chat_message_state", FALSE)) {
		setState(newState);
	} else {
		lInfo() << "Chat message " << sharedMessage << ": moving participant '" << *participantAddress << "' state to "
		        << Utils::toString(newState);
		if (eventLog) {
			mainDb->setChatMessageParticipantState(eventLog, participantAddress, newState, stateChangeTime);
		}

		// Update chat message state if it doesn't depend on IMDN
		if (isMe && !isImdnControlledState(newState)) {
			setState(newState);
		}

		if (isImdnControlledState(newState)) {
			const auto imdnStates = q->getParticipantsState();
			size_t nbRecipients = 0;
			size_t nbDisplayedStates = 0;
			size_t nbDeliveredStates = 0;
			size_t nbDeliveredToUserStates = 0;
			size_t nbNotDeliveredStates = 0;
			for (const auto &imdnState : imdnStates) {
				const auto &participantState = imdnState.getState();
				const auto &imdnParticipant = imdnState.getParticipant();
				if (fromAddress->weakEqual(*(imdnParticipant->getAddress()))) {
					if (participantState == ChatMessage::State::NotDelivered) {
						nbNotDeliveredStates++;
					}
				} else {
					nbRecipients++;
					switch (participantState) {
						case ChatMessage::State::Displayed:
							nbDisplayedStates++;
							break;
						case ChatMessage::State::DeliveredToUser:
							nbDeliveredToUserStates++;
							break;
						case ChatMessage::State::Delivered:
							nbDeliveredStates++;
							break;
						case ChatMessage::State::NotDelivered:
							nbNotDeliveredStates++;
							break;
						default:
							break;
					}
				}
			}

			if (nbNotDeliveredStates > 0) {
				setState(ChatMessage::State::NotDelivered);
			} else if ((nbRecipients > 0) && (nbDisplayedStates == nbRecipients)) {
				setState(ChatMessage::State::Displayed);
			} else if ((nbRecipients > 0) && ((nbDisplayedStates + nbDeliveredToUserStates) == nbRecipients)) {
				setState(ChatMessage::State::DeliveredToUser);
			} else if ((nbRecipients > 0) &&
			           ((nbDeliveredStates + nbDisplayedStates + nbDeliveredToUserStates) == nbRecipients)) {
				setState(ChatMessage::State::Delivered);
			}
		}

		if (isMe) {
			// Set me participant state to displayed if we are the sender, set the message as Displayed as soon as we
			// received the 202 Accepted response
			if (fromAddress->weakEqual(*participantAddress) && (newState == ChatMessage::State::DeliveredToUser)) {
				setParticipantState(participantAddress, ChatMessage::State::Displayed, ::ms_time(nullptr));
			}
			if ((newState == ChatMessage::State::NotDelivered) && (reason == LinphoneReasonForbidden)) {
				// Try to recover from a situation where the server replied 403 to an outgoing message
				chatRoom->handleMessageRejected(sharedMessage);
			}
		}
	}

	// Once the participant and IMDN state are updated, it is possible to notify the application
	LinphoneChatMessage *msg = L_GET_C_BACK_PTR(q);
	LinphoneChatRoom *cr = chatRoom->toC();
	auto participant = isMe ? me : chatRoom->findParticipant(participantAddress);
	ParticipantImdnState imdnState(participant, newState, stateChangeTime);

	// Legacy callbacks, deprecated !
	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
	if (cbs && linphone_chat_message_cbs_get_participant_imdn_state_changed(cbs)) {
		linphone_chat_message_cbs_get_participant_imdn_state_changed(cbs)(
		    msg, _linphone_participant_imdn_state_from_cpp_obj(imdnState));
	}

	const LinphoneParticipantImdnState *c_state = _linphone_participant_imdn_state_from_cpp_obj(imdnState);
	_linphone_chat_message_notify_participant_imdn_state_changed(msg, c_state);
	_linphone_chat_room_notify_chat_message_participant_imdn_state_changed(cr, msg, c_state);
}

void ChatMessagePrivate::setAutomaticallyResent(bool enable) {
	mAutomaticallyResent = enable;
}

void ChatMessagePrivate::setState(ChatMessage::State newState) {
	L_Q();

	// 1. Check valid transition.
	if (!chatMessageFsmChecker.isValid(state, newState)) return;

	const shared_ptr<ChatMessage> &sharedMessage = q->getSharedFromThis();
	const auto &chatRoom = q->getChatRoom();
	auto me = chatRoom->getMe();
	bool hasBeenAutomaticallyResent = mAutomaticallyResent;
	// If the message was automatically resent because of a previous failure, then wait that it reaches a state such as
	// Delivered, NotDelivered, DeliveredToUser or Displayed before resetting the flag In fact, it may happen that the
	// message goes from the NotDelivered state to InProgress and finally coming back to NotDelivered in some cases and
	// we want to avoid the message to enter an infinite loop
	if (mAutomaticallyResent &&
	    ((newState == ChatMessage::State::PendingDelivery) || (newState == ChatMessage::State::Delivered) ||
	     (newState == ChatMessage::State::DeliveredToUser) || (newState == ChatMessage::State::Displayed))) {
		setAutomaticallyResent(false);
	}

	// 2. Update state and notify changes.
	lInfo() << "Chat message " << sharedMessage << " of chat room "
	        << (chatRoom ? chatRoom->getConferenceId() : ConferenceId()) << " : moving from " << Utils::toString(state)
	        << " to " << Utils::toString(newState);
	ChatMessage::State oldState = state;
	state = newState;

	if (direction == ChatMessage::Direction::Outgoing) {
		// Delivered state isn't triggered by IMDN, so participants state won't be set unless we manually do so here
		if (state == ChatMessage::State::Delivered) {
			// Use list of participants the client is sure have received the message and not the actual list of
			// participants being part of the chatroom
			const auto imdnStates = q->getParticipantsState();
			for (const auto &imdnState : imdnStates) {
				const auto &participant = imdnState.getParticipant();
				const auto &participantAddress = participant->getAddress();
				const auto isMe = chatRoom->isMe(participantAddress);
				// me participant was already set to Delivered by function message_delivery_update
				if (!isMe) {
					setParticipantState(participantAddress, state, q->getTime());
				}
			}
		}

		if (state == ChatMessage::State::NotDelivered || state == ChatMessage::State::Delivered ||
		    state == ChatMessage::State::DeliveredToUser || state == ChatMessage::State::Displayed) {
			chatRoom->removeTransientChatMessage(sharedMessage);
		}
	}

	LinphoneChatMessage *msg = L_GET_C_BACK_PTR(q);
	if (linphone_chat_message_get_message_state_changed_cb(msg))
		linphone_chat_message_get_message_state_changed_cb(msg)(
		    msg, LinphoneChatMessageState(state), linphone_chat_message_get_message_state_changed_cb_user_data(msg));

	LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
	if (cbs && linphone_chat_message_cbs_get_msg_state_changed(cbs))
		linphone_chat_message_cbs_get_msg_state_changed(cbs)(msg, (LinphoneChatMessageState)state);
	_linphone_chat_message_notify_msg_state_changed(msg, (LinphoneChatMessageState)state);

	auto listenersCopy = listeners; // To allow listener to be removed while iterating
	for (auto &listener : listenersCopy) {
		listener->onChatMessageStateChanged(q->getSharedFromThis(), state);
	}
	if (state == ChatMessage::State::Displayed) {
		listeners.clear();
	}

	// 3. Specific case, upon reception do not attempt to store in db before asking the user if he wants to do so or not
	if (state == ChatMessage::State::FileTransferDone && direction == ChatMessage::Direction::Incoming) {
		if (!hasFileTransferContent() && isMarkedAsRead()) {
			setParticipantState(me->getAddress(), ChatMessage::State::Displayed, ::ms_time(nullptr));
			return;
		}
	}

	if (state == ChatMessage::State::Delivered && oldState == ChatMessage::State::Idle &&
	    direction == ChatMessage::Direction::Incoming && !q->isValid()) {
		// If we're here it's because message is because we're in the middle of the receive() method and
		// we won't have a valid dbKey until the chat room callback asking if message should be store will be called
		// and that's happen in the notifyReceiving() called at the of the receive() method we're in.
		// This prevents the error log: Invalid db key [%p] associated to message [%p]
		return;
	}

	// 4. update in database for ephemeral message if necessary.
	if (isEphemeral && state == ChatMessage::State::Displayed) {
		lInfo() << "All participants are in displayed state, starting ephemeral countdown";
		startEphemeralCountDown();
	}

	// 5. Update in database if necessary.
	if (state != ChatMessage::State::InProgress && state != ChatMessage::State::FileTransferError &&
	    state != ChatMessage::State::FileTransferInProgress && state != ChatMessage::State::FileTransferCancelling) {
		updateInDb();
	}

	bool needResend = false;
	if ((state == ChatMessage::State::PendingDelivery) || (state == ChatMessage::State::NotDelivered)) {
		if (salOp) {
			const LinphoneErrorInfo *error_info = q->getErrorInfo();
			if (error_info) {
				LinphoneReason reason = linphone_error_info_get_reason(error_info);
				needResend = ((reason == LinphoneReasonIOError) || (reason == LinphoneReasonNoResponse));
			}
			salOp->setUserPointer(nullptr);
			salOp->unref();
			salOp = nullptr;
		}
		restoreFileTransferContentAsFileContent();
	}

	if (state == ChatMessage::State::PendingDelivery) {
		chatRoom->getCore()->getPrivate()->registerListener(q);
		if ((direction == ChatMessage::Direction::Outgoing) && !hasBeenAutomaticallyResent && needResend &&
		    !q->getResendTimer()) {
			lInfo() << "Message [" << q << "]  will be automatically resent due to failure in "
			        << ChatMessage::resendTimerExpiresS << "s";
			q->createResendTimer();
		}
	}
}

void ChatMessagePrivate::startEphemeralCountDown() {
	L_Q();

	shared_ptr<AbstractChatRoom> chatRoom = q->getChatRoom();
	if (!chatRoom) return;

	// set ephemeral message expired time
	ephemeralExpireTime = ::ms_time(NULL) + (long)ephemeralLifetime;
	unique_ptr<MainDb> &mainDb = chatRoom->getCore()->getPrivate()->mainDb;
	mainDb->updateEphemeralMessageInfos(storageId, ephemeralExpireTime);

	const shared_ptr<ChatMessage> &sharedMessage = q->getSharedFromThis();
	chatRoom->getCore()->getPrivate()->updateEphemeralMessages(sharedMessage);

	lInfo() << "Starting ephemeral countdown with life time: " << ephemeralLifetime;

	// notify start !
	shared_ptr<LinphonePrivate::EventLog> event = LinphonePrivate::MainDb::getEvent(mainDb, q->getStorageId());
	if (chatRoom && event) {
		_linphone_chat_room_notify_ephemeral_message_timer_started(chatRoom->toC(), L_GET_C_BACK_PTR(event));
		LinphoneChatMessage *msg = L_GET_C_BACK_PTR(q);
		LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(msg);
		if (cbs && linphone_chat_message_cbs_get_ephemeral_message_timer_started(cbs))
			linphone_chat_message_cbs_get_ephemeral_message_timer_started(cbs)(msg);
		_linphone_chat_message_notify_ephemeral_message_timer_started(msg);
	}
}

// -----------------------------------------------------------------------------

belle_http_request_t *ChatMessagePrivate::getHttpRequest() const {
	return fileTransferChatMessageModifier.getHttpRequest();
}

void ChatMessagePrivate::setHttpRequest(belle_http_request_t *request) {
	fileTransferChatMessageModifier.setHttpRequest(request);
}

// -----------------------------------------------------------------------------

void ChatMessagePrivate::disableDeliveryNotificationRequiredInDatabase() {
	L_Q();
	shared_ptr<AbstractChatRoom> chatRoom = q->getChatRoom();
	if (!chatRoom) return;

	unique_ptr<MainDb> &mainDb = chatRoom->getCore()->getPrivate()->mainDb;
	if (q->isValid()) mainDb->disableDeliveryNotificationRequired(mainDb->getEvent(mainDb, q->getStorageId()));
}

void ChatMessagePrivate::disableDisplayNotificationRequiredInDatabase() {
	L_Q();

	shared_ptr<AbstractChatRoom> chatRoom = q->getChatRoom();
	if (!chatRoom) return;

	unique_ptr<MainDb> &mainDb = chatRoom->getCore()->getPrivate()->mainDb;
	const std::shared_ptr<const EventLog> &eventLog = mainDb->getEvent(mainDb, q->getStorageId());
	if (q->isValid() && eventLog) mainDb->disableDisplayNotificationRequired(eventLog);
}

// -----------------------------------------------------------------------------

SalOp *ChatMessagePrivate::getSalOp() const {
	return salOp;
}

void ChatMessagePrivate::setSalOp(SalOp *op) {
	salOp = op;
}

SalCustomHeader *ChatMessagePrivate::getSalCustomHeaders() const {
	return salCustomHeaders;
}

void ChatMessagePrivate::setSalCustomHeaders(SalCustomHeader *headers) {
	salCustomHeaders = headers;
}

void ChatMessagePrivate::addSalCustomHeader(const string &name, const string &value) {
	salCustomHeaders = sal_custom_header_append(salCustomHeaders, name.c_str(), value.c_str());
}

void ChatMessagePrivate::removeSalCustomHeader(const string &name) {
	salCustomHeaders = sal_custom_header_remove(salCustomHeaders, name.c_str());
}

string ChatMessagePrivate::getSalCustomHeaderValue(const string &name) const {
	return L_C_TO_STRING(sal_custom_header_find(salCustomHeaders, name.c_str()));
}

// -----------------------------------------------------------------------------
// Below methods are only for C API backward compatibility...
// -----------------------------------------------------------------------------

bool ChatMessagePrivate::hasTextContent() const {
	for (const auto &c : getContents()) {
		if (c->getContentType() == ContentType::PlainText) {
			return true;
		}
	}
	return false;
}

const std::shared_ptr<Content> ChatMessagePrivate::getTextContent() const {
	for (const auto &c : getContents()) {
		if (c->getContentType() == ContentType::PlainText) {
			return c;
		}
	}
	return nullptr;
}

bool ChatMessagePrivate::hasConferenceInvitationContent() const {
	for (const auto &c : getContents()) {
		if (c->getContentType().strongEqual(ContentType::Icalendar)) {
			return true;
		}
	}
	return false;
}

bool ChatMessagePrivate::hasFileTransferContent() const {
	for (const auto &c : contents) {
		if (c->isFileTransfer()) {
			return true;
		}
	}
	return false;
}

const std::shared_ptr<Content> ChatMessagePrivate::getFileTransferContent() const {
	for (const auto &c : contents) {
		if (c->isFileTransfer()) {
			return c;
		}
	}
	return nullptr;
}

const string &ChatMessagePrivate::getFileTransferFilepath() const {
	return fileTransferFilePath;
}

void ChatMessagePrivate::setFileTransferFilepath(const string &path) {
	fileTransferFilePath = path;
}

void ChatMessagePrivate::setEphemeralExpireTime(time_t expireTime) {
	ephemeralExpireTime = expireTime;
}

const string &ChatMessagePrivate::getAppdata() const {
	for (const auto &c : getContents()) {
		const auto &legacy = c->getProperty("legacy");
		if (legacy.isValid() && !legacy.getValue<string>().empty()) {
			return legacy.getValue<string>();
		}
	}
	return Utils::getEmptyConstRefObject<string>();
}

void ChatMessagePrivate::setAppdata(const string &data) {
	bool contentFound = false;
	for (auto &c : getContents()) {
		c->setProperty("legacy", Variant{data});
		contentFound = true;
		break;
	}
	if (contentFound) {
		updateInDb();
	}
}

const string &ChatMessagePrivate::getExternalBodyUrl() const {
	if (!externalBodyUrl.empty()) {
		return externalBodyUrl;
	}
	if (hasFileTransferContent()) {
		const auto &content = static_pointer_cast<FileTransferContent>(getFileTransferContent());
		return content->getFileUrl();
	}
	return Utils::getEmptyConstRefObject<string>();
}

void ChatMessagePrivate::setExternalBodyUrl(const string &url) {
	externalBodyUrl = url;
}

const ContentType &ChatMessagePrivate::getContentType() const {
	loadContentsFromDatabase();
	if (direction == ChatMessage::Direction::Incoming) {
		if (!contents.empty()) {
			auto &content = contents.front();
			cContentType = content->getContentType();
		} else {
			cContentType = internalContent.getContentType();
		}
	} else {
		if (internalContent.getContentType().isValid()) {
			cContentType = internalContent.getContentType();
		} else {
			if (!contents.empty()) {
				auto &content = contents.front();
				cContentType = content->getContentType();
			}
		}
	}
	return cContentType;
}

void ChatMessagePrivate::setContentType(const ContentType &contentType) {
	loadContentsFromDatabase();
	if (!contents.empty() && internalContent.getContentType().isEmpty() && internalContent.isEmpty()) {
		internalContent.setBody(contents.front()->getBody());
	}
	internalContent.setContentType(contentType);

	if ((currentSendStep & ChatMessagePrivate::Step::Started) != ChatMessagePrivate::Step::Started) {
		// if not started yet the sending also alter the first content
		if (!contents.empty()) contents.front()->setContentType(contentType);
	}
}

const string &ChatMessagePrivate::getText() const {
	loadContentsFromDatabase();
	if (direction == ChatMessage::Direction::Incoming) {
		if (hasTextContent()) {
			cText = getTextContent()->getBodyAsString();
		} else if (!contents.empty()) {
			auto &content = contents.front();
			cText = content->getBodyAsString();
		} else {
			cText = internalContent.getBodyAsString();
		}
	} else {
		if (!internalContent.isEmpty()) {
			cText = internalContent.getBodyAsString();
		} else {
			if (!contents.empty()) {
				auto &content = contents.front();
				cText = content->getBodyAsString();
			}
		}
	}
	return cText;
}

void ChatMessagePrivate::setText(const string &text) {
	loadContentsFromDatabase();
	if (!contents.empty() && internalContent.getContentType().isEmpty() && internalContent.isEmpty()) {
		internalContent.setContentType(contents.front()->getContentType());
	}
	internalContent.setBodyFromLocale(text);

	if ((currentSendStep & ChatMessagePrivate::Step::Started) != ChatMessagePrivate::Step::Started) {
		// if not started yet the sending also alter the first content
		if (!contents.empty()) contents.front()->setBodyFromLocale(text);
	}
}

const string &ChatMessagePrivate::getUtf8Text() const {
	loadContentsFromDatabase();
	if (direction == ChatMessage::Direction::Incoming) {
		if (hasTextContent()) {
			cText = getTextContent()->getBodyAsUtf8String();
		} else if (!contents.empty()) {
			auto &content = contents.front();
			cText = content->getBodyAsUtf8String();
		} else {
			cText = internalContent.getBodyAsUtf8String();
		}
	} else {
		if (!internalContent.isEmpty()) {
			cText = internalContent.getBodyAsUtf8String();
		} else {
			if (!contents.empty()) {
				auto &content = contents.front();
				cText = content->getBodyAsUtf8String();
			}
		}
	}
	return cText;
}

void ChatMessagePrivate::setUtf8Text(const string &text) {
	loadContentsFromDatabase();
	if (!contents.empty() && internalContent.getContentType().isEmpty() && internalContent.isEmpty()) {
		internalContent.setContentType(contents.front()->getContentType());
	}

	internalContent.setBodyFromUtf8(text);
	if (internalContent.getContentType().isEmpty()) {
		internalContent.setContentType(ContentType::PlainText);
	}

	if ((currentSendStep & ChatMessagePrivate::Step::Started) != ChatMessagePrivate::Step::Started) {
		// if not started yet the sending also alter the first content
		if (!contents.empty()) contents.front()->setBodyFromUtf8(text);
	}
}

const std::shared_ptr<Content> ChatMessagePrivate::getFileTransferInformation() const {
	if (hasFileTransferContent()) {
		return getFileTransferContent();
	}
	for (const auto &c : getContents()) {
		if (c->isFile()) {
			return c;
		}
	}
	return nullptr;
}

bool ChatMessagePrivate::downloadFile() {
	L_Q();

	for (auto &content : getContents())
		if (content->isFileTransfer()) return q->downloadFile(static_pointer_cast<FileTransferContent>(content));

	return false;
}

void ChatMessagePrivate::addContent(std::shared_ptr<Content> content) {
	getContents().push_back(content);
}

void ChatMessagePrivate::removeContent(std::shared_ptr<Content> content) {
	getContents().remove(content);
}

void ChatMessagePrivate::replaceContent(std::shared_ptr<Content> contentToRemove,
                                        std::shared_ptr<Content> contentToAdd) {
	list<std::shared_ptr<Content>>::iterator it = contents.begin();
	while (it != contents.end()) {
		auto &content = *it;
		if (content == contentToRemove) {
			it = contents.erase(it);
			it = contents.insert(it, contentToAdd);
			break;
		} else {
			it++;
		}
	}
}

void ChatMessagePrivate::loadFileTransferUrlFromBodyToContent() {
	L_Q();
	int errorCode = 0;
	fileTransferChatMessageModifier.decode(q->getSharedFromThis(), errorCode);
}

std::string ChatMessagePrivate::createFakeFileTransferFromUrl(const std::string &url) {
	return fileTransferChatMessageModifier.createFakeFileTransferFromUrl(url);
}

void ChatMessagePrivate::setChatRoom(const shared_ptr<AbstractChatRoom> &chatRoom) {
	L_Q();
	mChatRoom = chatRoom;
	const ConferenceId &conferenceId = chatRoom->getConferenceId();
	const auto &account = chatRoom->getAccount();
	const bool isBasicChatRoom =
	    (chatRoom->getCurrentParams()->getChatParams()->getBackend() == ChatParams::Backend::Basic);
	// If an account is attached to a chatroom, use its contact or the identity address otherwise use the local address
	// of the conference ID. For Flexisip based chatrooms, it is paramount to use a contact address as the From header.
	// RFC3428 forbids adding a Contact header to MESSAGE requests, therefore the From header is the only way a
	// conference server knows which device sent the request and can be forwarded to the other chat members as well as
	// other devices of the same participant.
	std::shared_ptr<Address> localAddress = nullptr;
	if (account) {
		if (isBasicChatRoom) {
			localAddress = account->getAccountParams()->getIdentityAddress();
			lInfo() << *chatRoom << " with ID " << conferenceId
			        << " is a basic chatroom therefore set the local address of message [" << q << "] to the account ["
			        << account << "]'s identity address " << *localAddress;
		} else {
			localAddress = account->getContactAddress();
			if (localAddress) {
				lInfo() << *chatRoom << " with ID " << conferenceId
				        << " is not a basic chatroom therefore set the local address of message [" << q
				        << "] to the account contact address " << *localAddress;
			}
		}
	}
	if (!localAddress) {
		localAddress = conferenceId.getLocalAddress();
		if (localAddress) {
			lInfo()
			    << *chatRoom << " with ID " << conferenceId
			    << " has no account associated to or the contact or the identity address is not available yet, setting "
			       "conference ID's local address as message local address "
			    << *localAddress;
		}
	}

	if (localAddress) {
		if (isBasicChatRoom) {
			localAddress = Address::create(localAddress->getUriWithoutGruu());
		} else {
			localAddress = localAddress->clone()->toSharedPtr();
		}
	} else {
		lError() << "Unable to set local address of message [" << q << "] in " << *chatRoom;
	}

	auto peerAddress = conferenceId.getPeerAddress()->clone()->toSharedPtr();
	if (direction == ChatMessage::Direction::Outgoing) {
		fromAddress = localAddress;
		toAddress = peerAddress;
	} else {
		fromAddress = peerAddress;
		toAddress = localAddress;
	}
	const auto &me = chatRoom->getMe();
	if (me) {
		mMeAddress = me->getAddress()->clone()->toSharedPtr();
	}
}

// -----------------------------------------------------------------------------

static void forceUtf8Content(Content &content) {
	// TODO: Deal with other content type in the future.
	ContentType contentType = content.getContentType();
	if (contentType != ContentType::PlainText) return;

	string charset = contentType.getParameter("charset").getValue();
	if (charset.empty()) return;

	size_t n = charset.find("charset=");
	if (n == string::npos) return;

	L_BEGIN_LOG_EXCEPTION

	size_t begin = n + sizeof("charset");
	size_t end = charset.find(";", begin);
	charset = charset.substr(begin, end - begin);

	if (Utils::stringToLower(charset) != "utf-8") {
		string utf8Body = Utils::convertAnyToUtf8(content.getBodyAsUtf8String(), charset);
		if (!utf8Body.empty()) {
			// TODO: use move operator if possible in the future!
			content.setBodyFromUtf8(utf8Body);
			contentType.addParameter("charset", "UTF-8");
			content.setContentType(contentType);
		}
	}

	L_END_LOG_EXCEPTION
}

LinphoneReason ChatMessagePrivate::receive() {
	L_Q();
	int errorCode = 0;
	LinphoneReason reason = LinphoneReasonNone;

	shared_ptr<Core> core = q->getCore();
	shared_ptr<AbstractChatRoom> chatRoom = q->getChatRoom();
	const auto &meAddress = q->getMeAddress();

	bool encryptionMandatory = false;
	const auto &account = chatRoom->getAccount();
	if (account) {
		encryptionMandatory = account->getAccountParams()->isInstantMessagingEncryptionMandatory();
	}

	// Prevent message duplication before decoding
	if (!mMessageId.empty() && chatRoom && chatRoom->findChatMessageFromMessageId(mMessageId)) {
		lInfo() << "Duplicated SIP MESSAGE with Call-ID " << callId << ", ignored.";
		core->getCCore()->number_of_duplicated_messages++;
		return core->getCCore()->chat_deny_code;
	}

	// ---------------------------------------
	// Start of message modification
	// ---------------------------------------

	if ((currentRecvStep & ChatMessagePrivate::Step::Encryption) == ChatMessagePrivate::Step::Encryption) {
		lInfo() << "Encryption step already done, skipping";
	} else {
		EncryptionChatMessageModifier ecmm;
		ChatMessageModifier::Result result = ecmm.decode(q->getSharedFromThis(), errorCode);
		if (result == ChatMessageModifier::Result::Error) {
			/* Unable to decrypt message */
#ifdef HAVE_ADVANCED_IM
			CpimChatMessageModifier ccmm;
			auto from = ccmm.parseFromHeaderCpimContentInLimeMessage(q->getSharedFromThis());
			if (from != nullptr) {
				q->getPrivate()->forceFromAddress(from);
			}
#endif
			chatRoom->notifyUndecryptableChatMessageReceived(q->getSharedFromThis());
			reason = linphone_error_code_to_reason(errorCode);
			if (!chatRoom) return reason;
			setParticipantState(meAddress, ChatMessage::State::NotDelivered, ::ms_time(nullptr), reason);
			return reason;
		} else if (result == ChatMessageModifier::Result::Suspended) {
			currentRecvStep |= ChatMessagePrivate::Step::Encryption;
			return LinphoneReasonNone;
		} else if (result == ChatMessageModifier::Result::Skipped && encryptionMandatory) {
			lError() << "Account receiving this message is configured to have encryption mandatory, refusing message";
			return LinphoneReasonNotAcceptable;
		}

		currentRecvStep |= ChatMessagePrivate::Step::Encryption;
	}

	// Sender Authentication
	// In secured chat rooms, the authenticatedFromAddress is already the decrypted CPIM From Address
	// In plain text basic chat rooms, the authenticatedFromAddress must be set here as the SIP From Address
	// In plain text group chat rooms the sender authentication is disabled
	const auto &chatRoomParams = q->getChatRoom()->getCurrentParams();
	if (!chatRoomParams->getChatParams()->isEncrypted()) {
		const bool isBasicChatRoom = (chatRoomParams->getChatParams()->getBackend() == ChatParams::Backend::Basic);
		if (isBasicChatRoom) {
			const auto &sipFromAddress = q->getSharedFromThis()->getFromAddress();
			setAuthenticatedFromAddress(*sipFromAddress);
		} else {
			lInfo() << "Sender authentication disabled for clear text group chat";
			senderAuthenticationEnabled = false;
		}
	}

	if ((currentRecvStep & ChatMessagePrivate::Step::Cpim) == ChatMessagePrivate::Step::Cpim) {
		lInfo() << "Cpim step already done, skipping";
	} else {
		if (internalContent.getContentType() == ContentType::Cpim) {
#ifdef HAVE_ADVANCED_IM
			CpimChatMessageModifier ccmm;
			ccmm.decode(q->getSharedFromThis(), errorCode);
#else
			lWarning() << "Cpim support disabled.";
#endif
		}
		currentRecvStep |= ChatMessagePrivate::Step::Cpim;
	}

	// Go through multipart otherwise Imdn::isError won't work in case of aggregated IMDN
	if ((currentRecvStep & ChatMessagePrivate::Step::Multipart) == ChatMessagePrivate::Step::Multipart) {
		lInfo() << "Multipart step already done, skipping";
	} else {
		MultipartChatMessageModifier mcmm;
		mcmm.decode(q->getSharedFromThis(), errorCode);
		currentRecvStep |= ChatMessagePrivate::Step::Multipart;
	}

	// Check if incoming message was flagged as unencrypted in an encrypted context
	if (getUnencryptedContentWarning()) {
		lWarning() << "Unencrypted content warning raised by encryption engine";
		// Allow error IMDN exclusively
		if (q->getSharedFromThis()->getInternalContent().getContentType() != ContentType::Imdn &&
		    !Imdn::isError(q->getSharedFromThis())) {
			lWarning() << "Discarding message of type "
			           << q->getSharedFromThis()->getInternalContent().getContentType();
			errorCode = 415;
			return linphone_error_code_to_reason(errorCode);
		}
	}

	if ((currentRecvStep & ChatMessagePrivate::Step::FileDownload) == ChatMessagePrivate::Step::FileDownload) {
		lInfo() << "File download step already done, skipping";
	} else {
		// This will check if internal content is FileTransfer and make the appropriate changes
		loadFileTransferUrlFromBodyToContent();
		currentRecvStep |= ChatMessagePrivate::Step::FileDownload;
	}

	if (contents.empty()) {
		// All previous modifiers only altered the internal content, let's fill the content list
		contents.push_back(Content::create(internalContent));
	}

	for (auto &content : contents) {
		forceUtf8Content(*content);
	}

	// ---------------------------------------
	// End of message modification
	// ---------------------------------------

	// Remove internal content as it is not needed anymore and will confuse some old methods like getText()
	internalContent.setBodyFromUtf8("");
	internalContent.setContentType(ContentType(""));
	// Also remove current step so we go through all modifiers if message is re-received (in case of recursive call from
	// a modifier)
	currentRecvStep = ChatMessagePrivate::Step::None;

	setParticipantState(meAddress, ChatMessage::State::Delivered, ::ms_time(nullptr));

	// Check if this is in fact an outgoing message (case where this is a message sent by us from an other device).
	const bool isFlexisipChatRoom =
	    (chatRoomParams->getChatParams()->getBackend() == ChatParams::Backend::FlexisipChat);
	if (isFlexisipChatRoom && (chatRoom->getLocalAddress()->weakEqual(*fromAddress))) {
		setDirection(ChatMessage::Direction::Outgoing);
	}

	// Check if this is a duplicate message.
	if (!imdnId.empty() && chatRoom && chatRoom->findChatMessage(imdnId, direction)) {
		lInfo() << "Duplicated SIP MESSAGE with Imdn-ID " << imdnId << ", ignored.";
		return core->getCCore()->chat_deny_code;
	}

	if (errorCode <= 0) {
		bool foundSupportContentType = false;
		for (auto &c : contents) {
			ContentType ct(c->getContentType());
			ct.cleanParameters();
			string contenttype = ct.getType() + "/" + ct.getSubType();
			if (linphone_core_is_content_type_supported(core->getCCore(), contenttype.c_str())) {
				foundSupportContentType = true;
				break;
			} else {
				lError() << "Unsupported content-type: " << contenttype;
			}
		}

		if (!foundSupportContentType) {
			errorCode = 415;
			lError() << "No content-type in the contents list is supported...";
		}
	}

	if (chatRoom && (errorCode > 0)) {
		reason = linphone_error_code_to_reason(errorCode);
		setParticipantState(meAddress, ChatMessage::State::NotDelivered, ::ms_time(nullptr), reason);
		return reason;
	}

	if (q->isReaction()) {
		markAsRead();

		auto messageId = q->getReactionToMessageId();
		auto originalMessage = q->getReactionToMessage();
		if (!originalMessage) {
			lError() << "Failed to find original message with ID [" << messageId
			         << "] for which the reaction was received";
			return reason;
		}

		shared_ptr<AbstractChatRoom> chatRoom = q->getChatRoom();
		LinphoneChatRoom *cr = chatRoom->toC();

		LinphoneChatMessage *msg = L_GET_C_BACK_PTR(originalMessage);

		const string &reactionBody = getUtf8Text();
		if (reactionBody.empty()) {
			auto fromAddress = q->getFromAddress();
			lInfo() << "Reaction body for message ID [" << messageId << "] is empty, removing existing reaction from ["
			        << fromAddress->asStringUriOnly() << "] if any";
			unique_ptr<MainDb> &mainDb = chatRoom->getCore()->getPrivate()->mainDb;
			mainDb->removeConferenceChatMessageReactionEvent(messageId, fromAddress);

			const LinphoneAddress *address = q->getFromAddress()->toC();
			_linphone_chat_message_notify_reaction_removed(msg, address);
			linphone_core_notify_message_reaction_removed(q->getCore()->getCCore(), cr, msg, address);

			const char *call_id = getCallId().c_str();
			linphone_core_notify_message_reaction_removed_private(q->getCore()->getCCore(), cr, msg, address, call_id);

			return reason;
		}

		storeInDb();

		LinphoneChatMessageReaction *reaction =
		    ChatMessageReaction::createCObject(messageId, getUtf8Text(), q->getFromAddress(), getCallId());

		_linphone_chat_message_notify_new_message_reaction(msg, reaction);
		_linphone_chat_room_notify_new_reaction_received(cr, msg, reaction);
		linphone_core_notify_new_message_reaction(q->getCore()->getCCore(), cr, msg, reaction);

		linphone_chat_message_reaction_unref(reaction);
		return reason;
	}

	// If message was outgoing, mark it as read
	if (direction == ChatMessage::Direction::Outgoing) {
		markAsRead();
	}

	if (chatRoom && (getContentType() != ContentType::Imdn && getContentType() != ContentType::ImIsComposing)) {
		// If we receive a message that is Outgoing it means we are in a flexisip based chat room and this message was
		// sent by us from another device, storing it
		if (direction == ChatMessage::Direction::Outgoing) {
			toBeStored = true;
		} else {
			_linphone_chat_room_notify_chat_message_should_be_stored(
			    static_pointer_cast<ChatRoom>(q->getChatRoom())->getCChatRoom(),
			    L_GET_C_BACK_PTR(q->getSharedFromThis()));
		}

		if (toBeStored) {
			storeInDb();
		}
	} else {
		toBeStored = false;
		// For compatibility, when CPIM is not used
		positiveDeliveryNotificationRequired = false;
		negativeDeliveryNotificationRequired = false;
		displayNotificationRequired = false;
	}

	chatRoom->onChatMessageReceived(q->getSharedFromThis());
	handleAutoDownload();
	if (isFlexisipChatRoom) {
		setParticipantState(q->getFromAddress(), ChatMessage::State::Displayed, ::ms_time(nullptr));
	}

	return reason;
}

void ChatMessagePrivate::handleAutoDownload() {
	L_Q();

	if ((currentRecvStep & ChatMessagePrivate::Step::AutoFileDownload) == ChatMessagePrivate::Step::AutoFileDownload) {
		lInfo() << "Auto file download step already done, skipping";
	} else {
		int maxSize = linphone_core_get_max_size_for_auto_download_incoming_files(q->getCore()->getCCore());
		bool autoDownloadVoiceRecordings =
		    !!linphone_core_is_auto_download_voice_recordings_enabled(q->getCore()->getCCore());
		bool autoDownloadIcalendars = !!linphone_core_is_auto_download_icalendars_enabled(q->getCore()->getCCore());
		for (auto &c : contents) {
			if (c->isFileTransfer()) {
				auto ftc = static_pointer_cast<FileTransferContent>(c);
				ContentType fileContentType = ftc->getFileContentType();

				if ((maxSize == 0 || (maxSize > 0 && ftc->getFileSize() <= (size_t)maxSize)) ||
				    (autoDownloadVoiceRecordings && fileContentType.strongEqual(ContentType::VoiceRecording)) ||
				    (autoDownloadIcalendars && fileContentType.strongEqual(ContentType::Icalendar))) {
					string downloadPath = q->getCore()->getDownloadPath();
					if (downloadPath.empty()) {
						lWarning() << "Download path is empty, won't be able to do auto download";
						break;
					} else {
						ostringstream sstream;
						size_t randomSize = 12;
						for (size_t i = 0; i < randomSize; i++) {
							sstream << bctbx_random() % 10;
						}
						string file = sstream.str();
						string filepath = downloadPath + file;
						lInfo() << "Automatically downloading file to " << filepath;
						ftc->setFilePath(filepath);
						setAutoFileTransferDownloadInProgress(true);
					}
				}
			}
		}
		// Download all attachments
		if (isAutoFileTransferDownloadInProgress()) {
			q->downloadFiles();
		} else {
			currentRecvStep |= ChatMessagePrivate::Step::AutoFileDownload;
		}
	}

	if (!isAutoFileTransferDownloadInProgress()) {
		endMessageReception();
	}

	for (auto &c : contents) {
		c->setRelatedChatMessageId(imdnId);

		ContentType contentType = c->getContentType();
		if (contentType.strongEqual(ContentType::Icalendar)) {
			LinphoneConferenceInfo *cConfInfo =
			    linphone_factory_create_conference_info_from_icalendar_content(linphone_factory_get(), c->toC());
			if (cConfInfo != nullptr) {
				auto confInfo = ConferenceInfo::toCpp(cConfInfo)->getSharedFromThis();
#ifdef HAVE_DB_STORAGE
				unique_ptr<MainDb> &mainDb = q->getCore()->getPrivate()->mainDb;
				mainDb->insertConferenceInfo(confInfo);
#endif // HAVE_DB_STORAGE
				linphone_core_notify_conference_info_received(q->getCore()->getCCore(), cConfInfo);
				linphone_conference_info_unref(cConfInfo);
			}
		}
	}

	return;
}

void ChatMessagePrivate::endMessageReception() {
	L_Q();
	shared_ptr<AbstractChatRoom> chatRoom = q->getChatRoom();
	if (!chatRoom) return;
	const auto &meAddress = q->getMeAddress();
	chatRoom->removeTransientChatMessage(q->getSharedFromThis());
	// The message is set to delivered here because this code is hit if the attachment cannot be downloaded or the
	// download is aborted The delivered state will be set again message_delivery_update upon reception of 200 Ok or 202
	// Accepted when a message is sent
	setParticipantState(meAddress, ChatMessage::State::Delivered, ::ms_time(NULL));

	const bool isBasicChatRoom = chatRoom->getCapabilities().isSet(ChatRoom::Capabilities::Basic);
	if (!isBasicChatRoom) {
		if (!isInAggregationQueue) {
			// Wait for message to be notified to the app before changing the state to DeliveredToUser
			setParticipantState(meAddress, ChatMessage::State::DeliveredToUser, ::ms_time(nullptr));
		}
	}
}

void ChatMessagePrivate::restoreFileTransferContentAsFileContent() {
	if (contents.size() == 0) {
		lWarning() << "Contents list is empty, nothing to restore";
		return;
	}

	// Restore FileContents and remove FileTransferContents
	list<std::shared_ptr<Content>>::iterator it = contents.begin();
	while (it != contents.end()) {
		auto &content = *it;
		if (content && content->isFileTransfer()) {
			auto fileTransferContent = static_pointer_cast<FileTransferContent>(content);
			auto fileContent = fileTransferContent->getFileContent();

			if (fileContent) {
				fileContent->setRelatedChatMessageId(imdnId);
				it = contents.erase(it);
				it = contents.insert(it, fileContent);
			} else {
				lWarning() << "Found FileTransferContent but no associated FileContent";
				it++;
			}
		} else {
			it++;
		}
	}
}

void ChatMessagePrivate::send() {
	L_Q();

	shared_ptr<AbstractChatRoom> chatRoom = q->getChatRoom();
	if (!chatRoom) return;

	const auto &meAddress = q->getMeAddress();
	const auto &chatRoomState = chatRoom->getState();
	const auto &chatRoomParams = chatRoom->getCurrentParams();
	shared_ptr<Core> core = q->getCore();
	// Postpone the sending of a message through an encrypted chatroom when we don't know yet the full list of
	// participants. However, if the core is shutting down, the message should be sent anyway even though we are
	// potentially send it to an incomplete list of devices
	LinphoneGlobalState coreGlobalState = linphone_core_get_global_state(core->getCCore());
	const auto &chatBackend = chatRoomParams->getChatParams()->getBackend();
	const auto conference = chatRoom->getConference();
	const auto subscriptionUnderway = conference ? conference->isSubscriptionUnderWay() : false;
	if ((coreGlobalState != LinphoneGlobalOff) && (coreGlobalState != LinphoneGlobalShutdown) &&
	    (chatBackend == ChatParams::Backend::FlexisipChat) &&
	    (subscriptionUnderway || (chatRoomState != ConferenceInterface::State::Created))) {
		lInfo() << "Message " << q << " in chat room [" << chatRoom << "] " << chatRoom->getConferenceId()
		        << " is being sent while the subscription is underway (actually subscription is"
		        << std::string(subscriptionUnderway ? " " : " not ")
		        << "underway) or the chat room is not in the created state (actual state "
		        << Utils::toString(chatRoomState) << ")";
		chatRoom->addPendingMessage(q->getSharedFromThis());
		return;
	}

	markAsRead();
	SalOp *op = salOp;
	LinphoneCall *lcall = nullptr;
	int errorCode = 0;
	bool isResend = ((state == ChatMessage::State::PendingDelivery) || (state == ChatMessage::State::NotDelivered));
	// Remove the sent flag so the message will be sent by the OP in case of resend
	currentSendStep &= ~ChatMessagePrivate::Step::Sent;

	currentSendStep |= ChatMessagePrivate::Step::Started;
	chatRoom->addTransientChatMessage(q->getSharedFromThis());

	if (toBeStored && (currentSendStep == (ChatMessagePrivate::Step::Started | ChatMessagePrivate::Step::None))) {
		storeInDb();

		if (!isResend && !q->isReaction() && getContentType() != ContentType::Imdn &&
		    getContentType() != ContentType::ImIsComposing) {
			if ((currentSendStep & ChatMessagePrivate::Step::Sending) != ChatMessagePrivate::Step::Sending) {
				LinphoneChatRoom *cr = chatRoom->toC();
				unique_ptr<MainDb> &mainDb = core->getPrivate()->mainDb;
				shared_ptr<EventLog> eventLog = mainDb->getEvent(mainDb, q->getStorageId());
				_linphone_chat_room_notify_chat_message_sending(cr, L_GET_C_BACK_PTR(eventLog));
				currentSendStep |= ChatMessagePrivate::Step::Sending;
			}
		}
	}

	if ((currentSendStep & ChatMessagePrivate::Step::FileUpload) == ChatMessagePrivate::Step::FileUpload) {
		lInfo() << "File upload step already done, skipping";
	} else {
		ChatMessageModifier::Result result = fileTransferChatMessageModifier.encode(q->getSharedFromThis(), errorCode);
		if (result == ChatMessageModifier::Result::Error) {
			setParticipantState(meAddress, ChatMessage::State::NotDelivered, ::ms_time(nullptr),
			                    linphone_error_code_to_reason(errorCode));
			// Remove current step so we go through all modifiers if message is re-sent
			currentSendStep = ChatMessagePrivate::Step::None;
			return;
		}

		if (result == ChatMessageModifier::Result::Suspended) {
			setParticipantState(meAddress, ChatMessage::State::FileTransferInProgress, ::ms_time(nullptr));
			return;
		}
		currentSendStep |= ChatMessagePrivate::Step::FileUpload;
	}

	const auto toAddr = toAddress->toC();
	if (linphone_config_get_int(core->getCCore()->config, "sip", "chat_use_call_dialogs", 0) != 0) {
		lcall = linphone_core_get_call_by_remote_address2(core->getCCore(), toAddr);
		if (lcall) {
			shared_ptr<Call> call = LinphonePrivate::Call::toCpp(lcall)->getSharedFromThis();
			if ((call->getState() == CallSession::State::Connected) ||
			    (call->getState() == CallSession::State::StreamsRunning) ||
			    (call->getState() == CallSession::State::Paused) || (call->getState() == CallSession::State::Pausing) ||
			    (call->getState() == CallSession::State::PausedByRemote)) {
				lInfo() << "Send SIP msg through the existing call";
				op = call->getOp();
				string identity =
				    linphone_core_find_best_identity(core->getCCore(), linphone_call_get_remote_address(lcall));
				if (identity.empty()) {
					auto account = core->lookupKnownAccount(toAddress, true);
					if (account) {
						identity = account->getAccountParams()->getIdentityAddress()->toString();
					} else {
						identity = linphone_core_get_primary_contact(core->getCCore());
					}
				}
			}
		}
	}

	if (!op) {
		/* Sending out of call */
		salOp = op = new SalMessageOp(core->getCCore()->sal.get());
		if (!fromAddress) {
			lError() << "The from address of ChatMessage [" << q << "] is unknown";
			return;
		}
		const auto fromAddr = fromAddress->toC();
		linphone_configure_op_2(core->getCCore(), op, fromAddr, toAddr, getSalCustomHeaders(),
		                        !!linphone_config_get_int(core->getCCore()->config, "sip", "chat_msg_with_contact", 0));
		op->setUserPointer(q); /* If out of call, directly store msg */
	}
	op->setFromAddress(fromAddress->getImpl());
	op->setToAddress(toAddress->getImpl());

	// ---------------------------------------
	// Start of message modification
	// ---------------------------------------

	if (applyModifiers) {
		// Do not multipart or encapsulate with CPIM in an old ChatRoom to maintain backward compatibility
		if (chatRoom->canHandleMultipart()) {
			if ((currentSendStep & ChatMessagePrivate::Step::Multipart) == ChatMessagePrivate::Step::Multipart) {
				lInfo() << "Multipart step already done, skipping";
			} else {
				if (contents.size() > 1) {
					MultipartChatMessageModifier mcmm;
					mcmm.encode(q->getSharedFromThis(), errorCode);
				}
				currentSendStep |= ChatMessagePrivate::Step::Multipart;
			}
		} else {
			if (contents.size() > 1) {
				lError() << "Chat room doesn't support multipart, but has multiple parts !";
			} else lInfo() << "Chat room doesn't support multipart, skipping this modifier";
		}

#ifdef HAVE_ADVANCED_IM
		if (chatRoom->canHandleCpim()) {
			if ((currentSendStep & ChatMessagePrivate::Step::Cpim) == ChatMessagePrivate::Step::Cpim) {
				lInfo() << "Cpim step already done, skipping";
			} else {
				CpimChatMessageModifier ccmm;
				ccmm.encode(q->getSharedFromThis(), errorCode);
				currentSendStep |= ChatMessagePrivate::Step::Cpim;
			}
		} else {
			lInfo() << "Chat room doesn't support CPIM, skipping this modifier";
		}
#else
		lWarning() << "Cpim support disabled.";
#endif

		if ((currentSendStep & ChatMessagePrivate::Step::Encryption) == ChatMessagePrivate::Step::Encryption) {
			lInfo() << "Encryption step already done, skipping";
		} else {
			if (!encryptionPrevented) {
				currentSendStep |= ChatMessagePrivate::Step::Encryption;
				EncryptionChatMessageModifier ecmm;
				ChatMessageModifier::Result result = ecmm.encode(q->getSharedFromThis(), errorCode);
				if (result == ChatMessageModifier::Result::Error) {
					sal_error_info_set((SalErrorInfo *)op->getErrorInfo(), SalReasonNotAcceptable, "SIP", errorCode,
					                   "Unable to encrypt IM", nullptr);
					// Remove current step so we go through all modifiers if message is re-sent
					currentSendStep = ChatMessagePrivate::Step::None;
					restoreFileTransferContentAsFileContent();
					setParticipantState(meAddress, ChatMessage::State::NotDelivered, ::ms_time(nullptr),
					                    linphone_error_code_to_reason(errorCode));
					chatRoom->removeTransientChatMessage(q->getSharedFromThis());
					return;
				} else if (result == ChatMessageModifier::Result::Suspended) {
					return;
				}
			} else {
				lInfo() << "Encryption has been prevented, skipping this modifier";
			}
		}
	} else if (linphone_core_conference_server_enabled(q->getCore()->getCCore())) {
		if (!encryptionPrevented) {
			EncryptionChatMessageModifier ecmm;
			ChatMessageModifier::Result result = ecmm.encode(q->getSharedFromThis(), errorCode);
			if (result == ChatMessageModifier::Result::Error) return;
		} else {
			lInfo() << "[server] Encryption has been prevented, skipping this modifier";
		}
	}

	// ---------------------------------------
	// End of message modification
	// ---------------------------------------

	if (internalContent.isEmpty()) {
		if (!contents.empty()) {
			internalContent = Content(*contents.front());
		} else if (externalBodyUrl.empty()) { // When using external body url, there is no content
			lError() << "Trying to send a message without any content !";
			return;
		}
	}

	// If message already sent by LIME X3DH synchronous encryption, do not send another one
	if ((currentSendStep & ChatMessagePrivate::Step::Sent) == ChatMessagePrivate::Step::Sent) {
		lInfo() << "Send step already done, skipping";
		return;
	}

	auto msgOp = dynamic_cast<SalMessageOpInterface *>(op);

	if (!externalBodyUrl.empty()) {
		Content content;
		ContentType contentType(ContentType::ExternalBody);
		contentType.addParameter("access-type", "URL");
		contentType.addParameter("URL", "\"" + externalBodyUrl + "\"");
		content.setContentType(contentType);
		currentSendStep |= ChatMessagePrivate::Step::Sent;
		msgOp->sendMessage(content);
	} else {
		if (!internalContent.getContentType().isValid()) internalContent.setContentType(ContentType::PlainText);
		if (!contentEncoding.empty()) internalContent.setContentEncoding(contentEncoding);
		currentSendStep |= ChatMessagePrivate::Step::Sent;
		msgOp->sendMessage(internalContent);
	}

	restoreFileTransferContentAsFileContent();

	// Remove internal content as it is not needed anymore and will confuse some old methods like getContentType()
	internalContent.setBodyFromUtf8("");
	internalContent.setContentType(ContentType(""));

	// Wait for message to be either Sent or NotDelivered unless it is an IMDN or COMPOSING
	if (getContentType() == ContentType::Imdn || getContentType() == ContentType::ImIsComposing) {
		chatRoom->removeTransientChatMessage(q->getSharedFromThis());
	}

	string callId = op->getCallId();
	if (imdnId.empty()) {
		setImdnMessageId(callId); /* must be known at that time */
	}
	setMessageId(callId);
	setCallId(callId);

	if (isResend) {
		// If it is a resend, reset participant states to Idle.
		// Not doing so, it will lead to the message being incorrectly marked as not delivered when at least one
		// participant hasn't received it yet.
		// Use list of participants the client is sure have received the message and not the actual list of participants
		// being part of the chatroom
		for (const auto &imdnState : q->getParticipantsState()) {
			const auto &participant = imdnState.getParticipant();
			setParticipantState(participant->getAddress(), ChatMessage::State::Idle, q->getTime());
		}
		// Update message in DB to store the new IMDN message ID
		updateInDb();
	} else if (toBeStored) {
		// Composing messages and IMDN aren't stored in DB so do not try, it will log an error message Invalid db key
		// for nothing.
		updateInDb();
	}

	if (lcall && linphone_call_get_op(lcall) == op) {
		/* In this case, chat delivery status is not notified, so unrefing chat message right now */
		/* Might be better fixed by delivering status, but too costly for now */
		return;
	}

	/* If operation failed, we should not change message state */
	if (direction == ChatMessage::Direction::Outgoing) {
		setIsReadOnly(true);
	}

	if (q->isReaction()) {
		return; // Do not notify chat message as sent
	}

	// Do not notify message sent callback when it's a resend or an IMDN/Composing
	if (!isResend && getContentType() != ContentType::Imdn && getContentType() != ContentType::ImIsComposing) {
		chatRoom->onChatMessageSent(q->getSharedFromThis());
	}
}

void ChatMessagePrivate::storeInDb() {
	L_Q();

	// TODO: store message in the future
	if (linphone_core_conference_server_enabled(q->getCore()->getCCore())) return;

	if (q->isValid()) {
		updateInDb();
		return;
	}

	shared_ptr<EventLog> eventLog = make_shared<ConferenceChatMessageEvent>(time, q->getSharedFromThis());

	// Avoid transaction in transaction if contents are not loaded.
	loadContentsFromDatabase();

	shared_ptr<AbstractChatRoom> chatRoom = q->getChatRoom();
	if (!chatRoom) return;

	chatRoom->addEvent(eventLog); // From this point forward the chat message will have a valid dbKey
	const auto &chatRoomParams = chatRoom->getCurrentParams();
	const bool isFlexisipChatRoom =
	    (chatRoomParams->getChatParams()->getBackend() == ChatParams::Backend::FlexisipChat);
	if (isFlexisipChatRoom) {
		setParticipantState(q->getMeAddress(), state, ::ms_time(nullptr));
	}

	if (direction == ChatMessage::Direction::Incoming) {
		if (hasFileTransferContent()) {
			// Keep the event in the transient list, message storage can be updated in near future
			chatRoom->addTransientEvent(eventLog);
		}
	} else {
		// Keep event in transient to be able to store in database state changes
		chatRoom->addTransientEvent(eventLog);
	}
}

void ChatMessagePrivate::updateInDb() {
	L_Q();

	if (!q->isValid()) {
		lError() << "Invalid storage ID [" << storageId << "] associated to message [" << q->getSharedFromThis() << "]";
		return;
	}

	shared_ptr<AbstractChatRoom> chatRoom = q->getChatRoom();
	if (!chatRoom) return;

	unique_ptr<MainDb> &mainDb = chatRoom->getCore()->getPrivate()->mainDb;
	shared_ptr<EventLog> eventLog = mainDb->getEvent(mainDb, q->getStorageId());

	if (!eventLog) {
		lError() << "cannot find eventLog for storage ID [" << storageId << "] associated to message ["
		         << q->getSharedFromThis() << "]";
		return;
	}
	// Avoid transaction in transaction if contents are not loaded.
	loadContentsFromDatabase();
	mainDb->updateEvent(eventLog);

	if (direction == ChatMessage::Direction::Incoming) {
		if (!hasFileTransferContent()) {
			// Incoming message doesn't have any download waiting anymore, we can remove it's event from the transients
			chatRoom->removeTransientEvent(eventLog);
		}
	} else {
		if (state == ChatMessage::State::Delivered || state == ChatMessage::State::NotDelivered) {
			// Once message has reached this state it won't change anymore so we can remove the event from the
			// transients
			chatRoom->removeTransientEvent(eventLog);
		}
	}
}

// -----------------------------------------------------------------------------

bool ChatMessagePrivate::isImdnControlledState(ChatMessage::State state) {
	return (state == ChatMessage::State::Displayed || state == ChatMessage::State::DeliveredToUser ||
	        state == ChatMessage::State::NotDelivered);
}

// -----------------------------------------------------------------------------

ChatMessage::ChatMessage(const shared_ptr<AbstractChatRoom> &chatRoom, ChatMessage::Direction direction)
    : Object(*new ChatMessagePrivate(chatRoom, direction)), CoreAccessor(chatRoom->getCore()) {
}

ChatMessage::ChatMessage(ChatMessagePrivate &p) : Object(p), CoreAccessor(p.getPublic()->getChatRoom()->getCore()) {
}

ChatMessage::~ChatMessage() {
	fileUploadEndBackgroundTask();
	deleteChatMessageFromCache();
	contentsToDownload.clear();
	stopResendTimer();
	std::shared_ptr<Core> core = nullptr;
	try {
		core = getCore();
		core->getPrivate()->unregisterListener(this);
	} catch (const bad_weak_ptr &) {
	}
}

bool ChatMessage::isValid() const {
	std::shared_ptr<Core> core = nullptr;
	try {
		core = getCore();
	} catch (const bad_weak_ptr &) {
		return false; // Return false if core is destroyed.
	}

	return (core && (getStorageId() >= 0));
}

void ChatMessage::deleteChatMessageFromCache() {
	if (isValid()) {
		// Delete chat message from the cache
		unique_ptr<MainDb> &mainDb = getCore()->getPrivate()->mainDb;
		mainDb->getPrivate()->storageIdToChatMessage.erase(getStorageId());
	}
}

long long ChatMessage::getStorageId() const {
	L_D();
	return d->storageId;
}

shared_ptr<AbstractChatRoom> ChatMessage::getChatRoom() const {
	L_D();
	shared_ptr<AbstractChatRoom> chatRoom(d->mChatRoom.lock());
	if (!chatRoom) lError() << "Unable to get valid chat room instance for chat message " << this;
	return chatRoom;
}

const std::shared_ptr<Address> &ChatMessage::getMeAddress() const {
	L_D();
	return d->mMeAddress;
}

// -----------------------------------------------------------------------------

time_t ChatMessage::getTime() const {
	L_D();
	return d->time;
}

bool ChatMessage::isSecured() const {
	L_D();
	return d->isSecured;
}

void ChatMessage::setIsSecured(bool isSecured) {
	L_D();
	d->isSecured = isSecured;
}

ChatMessage::Direction ChatMessage::getDirection() const {
	L_D();
	return d->direction;
}

ChatMessage::State ChatMessage::getState() const {
	L_D();
	return d->state;
}

const string &ChatMessage::getImdnMessageId() const {
	L_D();
	return d->imdnId;
}

void ChatMessagePrivate::setImdnMessageId(const string &id) {
	imdnId = id;

	for (auto &content : contents) {
		content->setRelatedChatMessageId(id);
	}
}

const string &ChatMessagePrivate::getCallId() const {
	return callId;
}

void ChatMessagePrivate::setCallId(const string &id) {
	callId = id;
}

const string &ChatMessagePrivate::getMessageId() const {
	return mMessageId;
}

void ChatMessagePrivate::setMessageId(const string &id) {
	mMessageId = id;
}

void ChatMessagePrivate::setForwardInfo(const string &fInfo) {
	forwardInfo = fInfo;
}

void ChatMessagePrivate::setReplyToMessageIdAndSenderAddress(const string &id, const std::shared_ptr<Address> &sender) {
	replyingToMessageId = id;
	replyingToMessageSender = sender;
}

bool ChatMessage::isReply() const {
	L_D();
	return !d->replyingToMessageId.empty();
}

const string &ChatMessage::getReplyToMessageId() const {
	L_D();
	return d->replyingToMessageId;
}

const std::shared_ptr<Address> &ChatMessage::getReplyToSenderAddress() const {
	L_D();
	return d->replyingToMessageSender;
}

shared_ptr<ChatMessage> ChatMessage::getReplyToMessage() const {
	if (!isReply()) return nullptr;
	return getChatRoom()->findChatMessage(getReplyToMessageId());
}

void ChatMessagePrivate::setReactionToMessageId(const string &id) {
	reactionToMessageId = id;
	// Disable delivery / display notifications for reactions
	positiveDeliveryNotificationRequired = false;
	negativeDeliveryNotificationRequired = false;
	displayNotificationRequired = false;
}

bool ChatMessage::isReaction() const {
	L_D();
	return !d->reactionToMessageId.empty();
}

const string &ChatMessage::getReactionToMessageId() const {
	L_D();
	return d->reactionToMessageId;
}

shared_ptr<ChatMessage> ChatMessage::getReactionToMessage() const {
	if (!isReaction()) return nullptr;
	return getChatRoom()->findChatMessage(getReactionToMessageId());
}

const list<shared_ptr<ChatMessageReaction>> ChatMessage::getReactions() const {
	L_D();
	d->reactions = getChatRoom()->getCore()->getPrivate()->mainDb->getChatMessageReactions(
	    const_pointer_cast<ChatMessage>(getSharedFromThis()));
	return d->reactions;
}

const shared_ptr<ChatMessageReaction> ChatMessage::getOwnReaction() const {
	L_D();
	d->reactions = getChatRoom()->getCore()->getPrivate()->mainDb->getChatMessageReactions(
	    const_pointer_cast<ChatMessage>(getSharedFromThis()));

	shared_ptr<ChatMessageReaction> ourReaction = nullptr;
	for (auto reaction : d->reactions) {
		if (reaction->getFromAddress()->weakEqual(*(getChatRoom()->getLocalAddress()))) {
			ourReaction = reaction;
			break;
		}
	}
	return ourReaction;
}

void ChatMessagePrivate::enableEphemeralWithTime(long time) {
	isEphemeral = true;
	ephemeralLifetime = time;
}

void ChatMessagePrivate::loadContentsFromDatabase() const {
	L_Q();

	if (contentsNotLoadedFromDatabase) {
		isReadOnly = false;
		contentsNotLoadedFromDatabase = false;
		q->getChatRoom()->getCore()->getPrivate()->mainDb->loadChatMessageContents(
		    const_pointer_cast<ChatMessage>(q->getSharedFromThis()));

		isReadOnly = true;
	}
}

bool ChatMessage::isRead() const {
	L_D();
	return d->markedAsRead || d->state == State::Displayed;
}

const Address &ChatMessage::getAuthenticatedFromAddress() const {
	L_D();
	return d->authenticatedFromAddress;
}

const std::shared_ptr<Address> &ChatMessage::getFromAddress() const {
	L_D();
	return d->fromAddress;
}

const std::shared_ptr<Address> &ChatMessage::getToAddress() const {
	L_D();
	return d->toAddress;
}

const std::shared_ptr<Address> &ChatMessage::getLocalAddress() const {
	L_D();
	if (getDirection() == Direction::Outgoing) return d->fromAddress;
	else return d->toAddress;
}

const std::shared_ptr<Address> &ChatMessage::getRecipientAddress() const {
	L_D();
	return d->recipientAddress;
}

const std::string &ChatMessage::getForwardInfo() const {
	L_D();
	return d->forwardInfo;
}

bool ChatMessage::getToBeStored() const {
	L_D();
	return d->toBeStored;
}

bool ChatMessage::isEphemeral() const {
	L_D();
	return d->isEphemeral;
}

long ChatMessage::getEphemeralLifetime() const {
	L_D();
	return d->ephemeralLifetime;
}

time_t ChatMessage::getEphemeralExpireTime() const {
	L_D();
	return d->ephemeralExpireTime;
}

void ChatMessage::setToBeStored(bool value) {
	L_D();
	d->toBeStored = value;
}

void ChatMessage::setInAggregationQueue(bool isInQueue) {
	L_D();
	d->isInAggregationQueue = isInQueue;
}

// -----------------------------------------------------------------------------

list<ParticipantImdnState> ChatMessage::getParticipantsState() const {
	list<ParticipantImdnState> result;
	const auto &chatRoom = getChatRoom();
	if (!chatRoom) return result;
	const auto &chatRoomParams = chatRoom->getCurrentParams();
	const bool isBasicChatRoom = (chatRoomParams->getChatParams()->getBackend() == ChatParams::Backend::Basic);
	if (isBasicChatRoom || !isValid()) return result;

	unique_ptr<MainDb> &mainDb = chatRoom->getCore()->getPrivate()->mainDb;
	shared_ptr<EventLog> eventLog = mainDb->getEvent(mainDb, getStorageId());
	list<MainDb::ParticipantState> dbResults = mainDb->getChatMessageParticipantStates(eventLog);
	for (const auto &dbResult : dbResults) {
		auto isMe = chatRoom->isMe(dbResult.address);
		auto participant = isMe ? chatRoom->getMe() : chatRoom->findParticipant(dbResult.address);
		if (participant) {
			result.emplace_back(participant, dbResult.state, dbResult.timestamp);
		}
	}

	return result;
}

list<ParticipantImdnState> ChatMessage::getParticipantsByImdnState(ChatMessage::State state) const {
	list<ParticipantImdnState> result;
	const auto &chatRoom = getChatRoom();
	const auto &chatRoomParams = chatRoom->getCurrentParams();
	const bool isBasicChatRoom = (chatRoomParams->getChatParams()->getBackend() == ChatParams::Backend::Basic);
	if (isBasicChatRoom || !isValid()) return result;

	unique_ptr<MainDb> &mainDb = chatRoom->getCore()->getPrivate()->mainDb;
	shared_ptr<EventLog> eventLog = mainDb->getEvent(mainDb, getStorageId());
	list<MainDb::ParticipantState> dbResults = mainDb->getChatMessageParticipantsByImdnState(eventLog, state);
	const auto &from = getFromAddress();
	const auto &me = chatRoom->getMe();
	auto sender = chatRoom->isMe(from) ? me : chatRoom->findParticipant(from);
	for (const auto &dbResult : dbResults) {
		const auto &pAddress = dbResult.address;
		auto participant = chatRoom->isMe(pAddress) ? me : chatRoom->findParticipant(pAddress);
		// Do not add myself to the result list if I am the sender.
		if (participant && (participant != sender))
			result.emplace_back(participant, dbResult.state, dbResult.timestamp);
	}

	return result;
}

ChatMessage::State ChatMessage::getParticipantState(const shared_ptr<Address> &address) const {
	const auto &chatRoom = getChatRoom();
	ChatMessage::State participantState = ChatMessage::State::Idle;
	if (!chatRoom) {
		return participantState;
	}
	const bool isBasicChatRoom =
	    (chatRoom->getCurrentParams()->getChatParams()->getBackend() == ChatParams::Backend::Basic);
	unique_ptr<MainDb> &mainDb = chatRoom->getCore()->getPrivate()->mainDb;
	shared_ptr<EventLog> eventLog = mainDb->getEvent(mainDb, getStorageId());
	if (isBasicChatRoom) {
		participantState = getState();
	} else if (eventLog) {
		participantState = mainDb->getChatMessageParticipantState(eventLog, address);
	}
	return participantState;
}

// -----------------------------------------------------------------------------

const LinphoneErrorInfo *ChatMessage::getErrorInfo() const {
	L_D();
	if (!d->errorInfo) d->errorInfo = linphone_error_info_new(); // let's do it mutable
	linphone_error_info_from_sal_op(d->errorInfo, d->salOp);
	return d->errorInfo;
}

bool ChatMessage::isReadOnly() const {
	L_D();
	return d->isReadOnly;
}

void ChatMessage::markAsRead() {
	L_D();

	if (!isValid()) {
		lError() << "Invalid storage ID [" << getStorageId() << "] associated to message [" << getSharedFromThis()
		         << "]";
		return;
	}

	shared_ptr<AbstractChatRoom> chatRoom = getChatRoom();

	d->markAsRead();
	// Do not set the message state has displayed if it contains a file transfer (to prevent imdn sending)
	if (!d->hasFileTransferContent()) {
		const auto &meAddress = getMeAddress();
		d->setParticipantState(meAddress, ChatMessage::State::Displayed, ::ms_time(nullptr));
	}
	if (getState() != ChatMessage::State::Displayed) {
		d->updateInDb();
	}

	if (chatRoom->getUnreadChatMessageCount() == 0) {
		LinphoneChatRoom *cChatRoom = static_pointer_cast<ChatRoom>(chatRoom)->getCChatRoom();
		_linphone_chat_room_notify_chat_room_read(cChatRoom);
		linphone_core_notify_chat_room_read(getCore()->getCCore(), cChatRoom);
	}
}

const list<std::shared_ptr<Content>> &ChatMessage::getContents() const {
	L_D();
	return d->getContents();
}

void ChatMessage::addContent(std::shared_ptr<Content> content) {
	L_D();
	if (!d->isReadOnly) d->addContent(content);
}

void ChatMessage::removeContent(std::shared_ptr<Content> content) {
	L_D();
	if (!d->isReadOnly) d->removeContent(content);
}

const Content &ChatMessage::getInternalContent() const {
	L_D();
	return d->internalContent;
}

void ChatMessage::setInternalContent(const Content &content) {
	L_D();
	d->internalContent = content;
}

void ChatMessage::send() {
	L_D();

	// Do not allow sending a message that is already being sent or that has been correctly delivered/displayed
	if ((d->state == State::InProgress) || (d->state == State::Delivered) || (d->state == State::FileTransferDone) ||
	    (d->state == State::DeliveredToUser) || (d->state == State::Displayed) ||
	    (d->state == State::FileTransferInProgress)) {
		lWarning() << "Cannot send chat message in state " << Utils::toString(d->state);
		return;
	}

	// Remove the modifiers flag so the message will go through CPIM, Multipart and Encryption again in case of resent
	d->currentSendStep &= ~ChatMessagePrivate::Step::Multipart;
	d->currentSendStep &= ~ChatMessagePrivate::Step::Cpim;
	d->currentSendStep &= ~ChatMessagePrivate::Step::Encryption;

	d->loadContentsFromDatabase();

	getChatRoom()->sendChatMessage(getSharedFromThis());
}

bool ChatMessage::downloadFile(std::shared_ptr<FileTransferContent> fileTransferContent, bool retry) {
	L_D();
	const auto filepath =
	    fileTransferContent->getFilePath().empty() ? d->getFileTransferFilepath() : fileTransferContent->getFilePath();
	lInfo() << "ChatMessage [" << getSharedFromThis() << "]: Start download of file " << filepath;
	const auto status = d->fileTransferChatMessageModifier.downloadFile(getSharedFromThis(), fileTransferContent);
	if (status == FileTransferChatMessageModifier::DownloadStatus::DownloadInProgress) {
		// There is already another file that it is downloading, therefore add it to the queue
		lWarning() << "ChatMessage [" << getSharedFromThis()
		           << "]: There is already a download in progress, therefore delaying download of file " << filepath
		           << " of content " << fileTransferContent;
		contentsToDownload.push_back(fileTransferContent);
	}

	switch (status) {
		case FileTransferChatMessageModifier::DownloadStatus::DownloadInProgress:
		case FileTransferChatMessageModifier::DownloadStatus::Ok:
			if (!retry) getCore()->incrementRemainingDownloadFileCount();
			break;
		case FileTransferChatMessageModifier::DownloadStatus::BadContent:
		case FileTransferChatMessageModifier::DownloadStatus::BadFile:
		case FileTransferChatMessageModifier::DownloadStatus::HttpFailure:
			break;
	}

	return (status == FileTransferChatMessageModifier::DownloadStatus::Ok);
}

bool ChatMessage::downloadFiles() {
	L_D();
	lInfo() << "Downloading all contents of message [" << getSharedFromThis() << "]";
	const auto &contents = d->getContents();
	bool ret = false;
	for (auto &content : contents) {
		if (content->isFileTransfer()) {
			ret |= downloadFile(dynamic_pointer_cast<FileTransferContent>(content));
		}
	}
	return ret;
}

// This method is call once the downlaod of a file is terminated and it kicks off the download of the following one if
// the user wishes to do so
bool ChatMessage::downloadTerminated() {
	L_D();
	bool ret = false;
	if (contentsToDownload.empty()) {
		if (d->isAutoFileTransferDownloadInProgress()) {
			d->doNotRetryAutoDownload();
			d->endMessageReception();
			d->setAutoFileTransferDownloadInProgress(false);
		}
	} else {
		auto content = contentsToDownload.front();
		contentsToDownload.pop_front();
		ret = downloadFile(dynamic_pointer_cast<FileTransferContent>(content), true);
	}
	return ret;
}

bool ChatMessage::isFileTransferInProgress() const {
	L_D();
	return d->fileTransferChatMessageModifier.isFileTransferInProgressAndValid();
}

void ChatMessage::cancelFileTransfer() {
	L_D();
	if (d->fileTransferChatMessageModifier.isFileTransferInProgressAndValid()) {
		lWarning() << "Canceling file transfer on message [" << getSharedFromThis() << "]";
		d->fileTransferChatMessageModifier.cancelFileTransfer();
		lInfo() << "File transfer on message [" << getSharedFromThis() << "] has been cancelled";

		if (d->state == State::FileTransferInProgress) {
			auto chatRoom = getChatRoom();
			const auto &meAddress = getMeAddress();
			lInfo() << "File transfer on message [" << getSharedFromThis() << "] was in progress, updating state";
			// For auto download messages, set the state back to Delivered
			if (d->isAutoFileTransferDownloadInProgress()) {
				d->setParticipantState(meAddress, State::Delivered, ::ms_time(nullptr));
				chatRoom->removeTransientChatMessage(getSharedFromThis());
			} else {
				d->setParticipantState(meAddress, State::NotDelivered, ::ms_time(nullptr));
			}
		}
	} else {
		lInfo() << "No existing file transfer - nothing to cancel";
	}
}

int ChatMessage::putCharacter(uint32_t character) {
	L_D();

	constexpr uint32_t newLine = 0x2028;
	constexpr uint32_t crlf = 0x0D0A;
	constexpr uint32_t lf = 0x0A;

	shared_ptr<AbstractChatRoom> chatRoom = getChatRoom();
	const auto &chatRoomParams = chatRoom->getCurrentParams();
	const auto isRealTimeText = chatRoomParams->getChatParams()->isRealTimeText();
	shared_ptr<Core> core = getCore();
	bool baudotEnabled = linphone_core_baudot_enabled(core->getCCore());
	if (!isRealTimeText && !baudotEnabled) {
		lError() << "Chat room [" << chatRoom
		         << "] that created the message doesn't have RealTimeText capability and Baudot is not enabled";
		return -1;
	}

	shared_ptr<Call> call = chatRoom->getCall();
	if (!call || (isRealTimeText && !call->getMediaStream(LinphoneStreamTypeText))) {
		lError() << "Failed to find Text stream from call [" << call << "]";
		return -1;
	}

	if (character == newLine || character == crlf || character == lf) {
		const char *parameterName = isRealTimeText ? "store_rtt_messages" : "store_baudot_messages";
		if (linphone_config_get_int(core->getCCore()->config, "misc", parameterName, 1) == 1) {
			lInfo() << "New line sent, forge a message with content " << d->rttMessage;
			d->state = State::Displayed;
			d->setText(d->rttMessage);
			d->storeInDb();
			d->rttMessage = "";
		}
	} else {
		string value = LinphonePrivate::Utils::unicodeToUtf8(character);
		d->rttMessage += value;
		string textType = isRealTimeText ? "RTT" : "Baudot";
		lDebug() << "Sent " << textType << " character: " << value << "(" << (unsigned long)character
		         << "), pending text is " << d->rttMessage;
	}

	if (isRealTimeText) {
		text_stream_putchar32(reinterpret_cast<TextStream *>(call->getMediaStream(LinphoneStreamTypeText)), character);
	} else if (baudotEnabled) {
		call->getMediaSession()->sendBaudotCharacter((char)character);
	}

	return 0;
}

void ChatMessage::fileUploadEndBackgroundTask() {
	L_D();
	d->fileTransferChatMessageModifier.fileUploadEndBackgroundTask();
}

void ChatMessage::addListener(shared_ptr<ChatMessageListener> listener) {
	L_D();
	d->listeners.push_back(listener);
}

void ChatMessage::removeListener(shared_ptr<ChatMessageListener> listener) {
	L_D();
	d->listeners.remove(listener);
}

belle_sip_source_t *ChatMessage::getResendTimer() const {
	return mResendTimer;
}

void ChatMessage::createResendTimer() {
	try {
		if (mResendTimer) {
			stopResendTimer();
		}
		mResendTimer = getCore()->getCCore()->sal->createTimer(
		    resendTimerExpired, this, ChatMessage::resendTimerExpiresS * 1000, "resend message timeout");
	} catch (const bad_weak_ptr &) {
		lError() << "Unable to create resend timer for chat message [" << this << "]";
	}
}

int ChatMessage::resendTimerExpired(void *data, BCTBX_UNUSED(unsigned int revents)) {
	ChatMessage *msg = static_cast<ChatMessage *>(data);
	return msg->handleAutomaticResend();
}

int ChatMessage::handleAutomaticResend() {
	L_D();
	// Keep a reference to this message as it may be freed when trying to send a message on a chat that is about to be
	// terminated (state TerminationPending, Terminated, Deleted)
	auto ref = getSharedFromThis();
	lInfo() << "Message [" << this << "] is automatically resent right now";
	d->setAutomaticallyResent(true);
	send();
	stopResendTimer();
	return 0;
}

void ChatMessage::stopResendTimer() {
	if (mResendTimer) {
		try {
			auto core = getCore()->getCCore();
			if (core && core->sal) core->sal->cancelTimer(mResendTimer);
		} catch (const bad_weak_ptr &) {
		}
		belle_sip_object_unref(mResendTimer);
		mResendTimer = nullptr;
	}
}

void ChatMessage::onNetworkReachable(bool sipNetworkReachable, BCTBX_UNUSED(bool mediaNetworkReachable)) {
	try {
		if ((getState() == State::PendingDelivery) && sipNetworkReachable) {
			// Try to resend message if the account has correctly registered
			auto account = getCore()->findAccountByIdentityAddress(getLocalAddress());
			if (account && (account->getState() == LinphoneRegistrationOk)) {
				lInfo() << "Attempt again to send message " << this << " because the network is again up and running";
				handleAutomaticResend();
			}
		}
	} catch (const bad_weak_ptr &) {
	}
}

void ChatMessage::onAccountRegistrationStateChanged(std::shared_ptr<Account> account,
                                                    LinphoneRegistrationState state,
                                                    BCTBX_UNUSED(const string &message)) {
	try {
		if ((getState() != State::PendingDelivery) || (state != LinphoneRegistrationState::LinphoneRegistrationOk))
			return;
		auto messageAccount = getCore()->findAccountByIdentityAddress(getLocalAddress());
		if ((messageAccount == account) && (state == LinphoneRegistrationOk)) {
			lInfo() << "Attempt again to send message " << this << " because the core registered again";
			handleAutomaticResend();
		}
	} catch (const bad_weak_ptr &) {
	}
}

std::ostream &operator<<(std::ostream &lhs, ChatMessage::State e) {
	switch (e) {
		case ChatMessage::State::Idle:
			lhs << "Idle";
			break;
		case ChatMessage::State::InProgress:
			lhs << "InProgress";
			break;
		case ChatMessage::State::Delivered:
			lhs << "Delivered";
			break;
		case ChatMessage::State::PendingDelivery:
			lhs << "PendingDelivery";
			break;
		case ChatMessage::State::NotDelivered:
			lhs << "NotDelivered";
			break;
		case ChatMessage::State::FileTransferError:
			lhs << "FileTransferError";
			break;
		case ChatMessage::State::FileTransferDone:
			lhs << "FileTransferDone";
			break;
		case ChatMessage::State::DeliveredToUser:
			lhs << "DeliveredToUser";
			break;
		case ChatMessage::State::Displayed:
			lhs << "Displayed";
			break;
		case ChatMessage::State::FileTransferCancelling:
			lhs << "FileTransferCancelling";
			break;
		case ChatMessage::State::FileTransferInProgress:
			lhs << "FileTransferInProgress";
			break;
	}
	return lhs;
}

ChatMessageLogContextualizer::ChatMessageLogContextualizer(const LinphoneChatMessage *msg)
    : CoreLogContextualizer(*L_GET_CPP_PTR_FROM_C_OBJECT(msg)) {
}

LINPHONE_END_NAMESPACE
