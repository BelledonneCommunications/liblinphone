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

#include "linphone/utils/utils.h"

#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "chat/chat-message/chat-message-p.h"
#include "client-chat-room.h"
#include "conference/client-conference.h"
#include "conference/handlers/client-conference-event-handler.h"
#include "conference/handlers/client-conference-list-event-handler.h"
#include "conference/participant-device.h"
#include "conference/participant-info.h"
#include "conference/participant.h"
#include "conference/session/call-session-p.h"
#include "content/content-disposition.h"
#include "content/content-type.h"
#include "core/core-p.h"
#include "factory/factory.h"
#include "linphone/api/c-chat-room.h"
#include "logger/logger.h"
#include "sal/refer-op.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// =============================================================================

ClientChatRoom::ClientChatRoom(const shared_ptr<Core> &core, const std::shared_ptr<Conference> &conf)
    : ChatRoom(core, conf) {
	// if preserve_backward_compatibility, force creation of secure room in all cases
	if (getCurrentParams()->getChatParams()->isEncrypted() ||
	    linphone_config_get_bool(linphone_core_get_config(getCore()->getCCore()), "lime",
	                             "preserve_backward_compatibility", FALSE))
		getCurrentParams()->setSecurityLevel(ConferenceParams::SecurityLevel::EndToEnd);
}

ClientChatRoom::ClientChatRoom(const shared_ptr<Core> &core, bool hasBeenLeft) : ChatRoom(core) {
	auto &chatParams = getCurrentParams()->getChatParams();
	chatParams->setBackend(ChatParams::Backend::FlexisipChat);
	if (chatParams->getEphemeralMode() == AbstractChatRoom::EphemeralMode::AdminManaged) {
		chatParams->enableEphemeral(chatParams->getEphemeralLifetime() > 0);
	}

	if (linphone_core_get_global_state(getCore()->getCCore()) == LinphoneGlobalStartup) {
		lDebug() << "Last notify set to [" << getConference()->getLastNotify() << "] for conference [" << this << "]";
	} else {
		lInfo() << "Last notify set to [" << getConference()->getLastNotify() << "] for conference [" << this << "]";
	}

	if (!hasBeenLeft) {
		getCore()->getPrivate()->clientListEventHandler->addHandler(
		    static_pointer_cast<ClientConference>(getConference())->eventHandler);
		mListHandlerUsed = getCore()->getPrivate()->clientListEventHandler->findHandler(getConferenceId()) != nullptr;
		if (!mListHandlerUsed) {
			static_pointer_cast<ClientConference>(getConference())->eventHandler->subscribe(getConferenceId());
		}
	}
}

// -----------------------------------------------------------------------------
void ClientChatRoom::addPendingMessage(const std::shared_ptr<ChatMessage> &chatMessage) {
	auto it = std::find(mPendingCreationMessages.begin(), mPendingCreationMessages.end(), chatMessage);
	if (it == mPendingCreationMessages.end()) mPendingCreationMessages.push_back(chatMessage);
}

void ClientChatRoom::onChatRoomCreated(const std::shared_ptr<Address> &remoteContact) {
	getConference()->onConferenceCreated(remoteContact);
	if (remoteContact->hasParam("isfocus")) {
		if (!getCore()->getPrivate()->clientListEventHandler->findHandler(getConferenceId())) {
			mBgTask.start(getCore(), 32); // It will be stopped when receiving the first notify
			auto conference = dynamic_pointer_cast<ClientConference>(getConference());
			auto eventHandler = conference->eventHandler;
			if (!eventHandler) {
				conference->initializeHandlers(conference.get(), true);
				eventHandler = conference->eventHandler;
			}
			eventHandler->subscribe(getConferenceId());
		}
	}
}

// -----------------------------------------------------------------------------
shared_ptr<Core> ClientChatRoom::getCore() const {
	return ChatRoom::getCore();
}

void ClientChatRoom::allowCpim(BCTBX_UNUSED(bool value)) {
}

void ClientChatRoom::allowMultipart(BCTBX_UNUSED(bool value)) {
}

bool ClientChatRoom::canHandleCpim() const {
	return true;
}

bool ClientChatRoom::canHandleMultipart() const {
	return true;
}

ClientChatRoom::CapabilitiesMask ClientChatRoom::getCapabilities() const {
	AbstractChatRoom::CapabilitiesMask capabilities = ConferenceParams::toCapabilities(getCurrentParams());
	shared_ptr<Call> call = getCall();
	if (call && call->getCurrentParams()->realtimeTextEnabled()) {
		capabilities |= Capabilities::RealTimeText;
	}
	return capabilities;
}

ChatRoom::SecurityLevel ClientChatRoom::getSecurityLevel() const {
	return static_pointer_cast<ClientConference>(getConference())->getSecurityLevelExcept(nullptr);
}

bool ClientChatRoom::hasBeenLeft() const {
	return static_pointer_cast<ClientConference>(getConference())->hasBeenLeft();
}

bool ClientChatRoom::isReadOnly() const {
	if (!getCurrentParams()->isGroup()) {
		return getState() != ConferenceInterface::State::Created &&
		       getState() != ConferenceInterface::State::Terminated;
	}
	return getState() != ConferenceInterface::State::Created;
}

void ClientChatRoom::deleteFromDb() {
	auto ref = getSharedFromThis();
	if (!hasBeenLeft()) {
		setDeletionOnTerminationEnabled(true);
		getConference()->leave();
		return;
	}
	ChatRoom::deleteFromDb();
}

list<shared_ptr<EventLog>> ClientChatRoom::getHistory(int nLast) const {
	return getCore()->getPrivate()->mainDb->getHistory(
	    getConferenceId(), nLast,
	    getCurrentParams()->isGroup() ? MainDb::FilterMask({MainDb::Filter::ConferenceChatMessageFilter,
	                                                        MainDb::Filter::ConferenceInfoNoDeviceFilter})
	                                  : MainDb::Filter::ConferenceChatMessageSecurityFilter);
}

list<shared_ptr<EventLog>> ClientChatRoom::getHistory(int nLast, HistoryFilterMask filters) const {
	return ChatRoom::getHistory(nLast, filters);
}

list<shared_ptr<EventLog>> ClientChatRoom::getHistoryRange(int begin, int end) const {
	return getCore()->getPrivate()->mainDb->getHistoryRange(
	    getConferenceId(), begin, end,
	    getCurrentParams()->isGroup() ? MainDb::FilterMask({MainDb::Filter::ConferenceChatMessageFilter,
	                                                        MainDb::Filter::ConferenceInfoNoDeviceFilter})
	                                  : MainDb::Filter::ConferenceChatMessageSecurityFilter);
}

list<shared_ptr<EventLog>> ClientChatRoom::getHistoryRange(int begin, int end, HistoryFilterMask filters) const {
	return ChatRoom::getHistoryRange(begin, end, filters);
}

int ClientChatRoom::getHistorySize() const {
	return getCore()->getPrivate()->mainDb->getHistorySize(
	    getConferenceId(), getCurrentParams()->isGroup()
	                           ? MainDb::FilterMask({MainDb::Filter::ConferenceChatMessageFilter,
	                                                 MainDb::Filter::ConferenceInfoNoDeviceFilter})
	                           : MainDb::Filter::ConferenceChatMessageSecurityFilter);
}

int ClientChatRoom::getHistorySize(HistoryFilterMask filters) const {
	return ChatRoom::getHistorySize(filters);
}

void ClientChatRoom::exhume() {
	if (getState() != Conference::State::Terminated) {
		lError() << "Cannot exhume a non terminated chat room";
		return;
	}
	if (getCurrentParams()->isGroup()) {
		lError() << "Cannot exhume a non one-to-one chat room";
		return;
	}
	if (getParticipants().size() == 0) {
		lError() << "Cannot exhume a chat room without any participant";
		return;
	}

	const std::shared_ptr<Address> &remoteParticipant = getParticipants().front()->getAddress();
	lInfo() << "Exhuming chat room [" << getConferenceId() << "] with participant [" << *remoteParticipant << "]";
	mLocalExhumePending = true;

	auto content = Content::create();
	list<Address> addresses;
	addresses.push_front(*remoteParticipant);
	content->setBodyFromUtf8(Utils::getResourceLists(addresses));
	content->setContentType(ContentType::ResourceLists);
	content->setContentDisposition(ContentDisposition::RecipientList);
	if (linphone_core_content_encoding_supported(getCore()->getCCore(), "deflate")) {
		content->setContentEncoding("deflate");
	}

	const auto &conferenceFactoryAddress =
	    Core::getConferenceFactoryAddress(getCore(), getConferenceId().getLocalAddress());
	auto session = static_pointer_cast<ClientConference>(getConference())->createSessionTo(conferenceFactoryAddress);
	session->startInvite(nullptr, getConference()->getUtf8Subject(), content);
	setState(ConferenceInterface::State::CreationPending);
}

void ClientChatRoom::onExhumedConference(const ConferenceId &oldConfId, const ConferenceId &newConfId) {
	const std::shared_ptr<Address> &addr = newConfId.getPeerAddress();
	auto chatRoom = getCore()->findChatRoom(oldConfId, false);
	auto conference = getConference();
	getCurrentParams()->setConferenceAddress(addr);
	auto focus = static_pointer_cast<ClientConference>(getConference())->mFocus;
	focus->setAddress(addr);
	focus->clearDevices();
	focus->addDevice(addr);

	conference->setConferenceId(newConfId);
	getCore()->getPrivate()->updateChatRoomConferenceId(chatRoom, oldConfId);

	getConference()->setLastNotify(0);
}

// Will be called on A when A is sending a message into a chat room with B previously terminated by B
void ClientChatRoom::onLocallyExhumedConference(const std::shared_ptr<Address> &remoteContact) {
	ConferenceId oldConfId = getConferenceId();
	ConferenceId newConfId = ConferenceId(remoteContact, oldConfId.getLocalAddress());

	lInfo() << "Conference [" << oldConfId << "] has been locally exhumed into [" << newConfId << "]";

	onExhumedConference(oldConfId, newConfId);

	setState(ConferenceInterface::State::Created);
	auto conference = dynamic_pointer_cast<ClientConference>(getConference());
	auto eventHandler = conference->eventHandler;
	if (eventHandler) {
		eventHandler->unsubscribe(); // Required for next subscribe to be sent
	} else {
		conference->initializeHandlers(conference.get(), true);
		eventHandler = conference->eventHandler;
	}
	getCore()->getPrivate()->clientListEventHandler->addHandler(eventHandler);
	eventHandler->subscribe(getConferenceId());

	lInfo() << "Found " << mPendingExhumeMessages.size() << " messages waiting for exhume";
	for (auto &chatMessage : mPendingExhumeMessages) {
		chatMessage->getPrivate()->setChatRoom(getSharedFromThis());
		ChatRoom::sendChatMessage(chatMessage);
	}

	mPendingExhumeMessages.clear();
	mLocalExhumePending = false;
}

// Will be called on A when B exhumes a chat room previously terminated by B
void ClientChatRoom::onRemotelyExhumedConference(SalCallOp *op) {
	ConferenceId oldConfId = getConferenceId();
	ConferenceId newConfId = ConferenceId(Address::create(op->getRemoteContact()), oldConfId.getLocalAddress());

	if (getState() != Conference::State::Terminated) {
		lWarning() << "Conference is being exhumed but wasn't terminated first!";

		if (oldConfId == newConfId) {
			lWarning() << "Conference is being exhumed but with the same conference id " << oldConfId << " !";
		} else {
			addConferenceIdToPreviousList(oldConfId);
		}
	}

	lInfo() << "Conference [" << oldConfId << "] is being remotely exhumed into [" << newConfId << "]";

	onExhumedConference(oldConfId, newConfId);

	if (getState() != Conference::State::Terminated) {
		// Wait for chat room to have been updated before inserting the previous ID in db
		if (oldConfId != newConfId) {
			getCore()->getPrivate()->mainDb->insertNewPreviousConferenceId(newConfId, oldConfId);
		}
	}

	const auto &conference = static_pointer_cast<ClientConference>(getConference());
	conference->confirmJoining(op);

	setState(ConferenceInterface::State::Created);

	auto eventHandler = conference->eventHandler;
	eventHandler->unsubscribe(); // Required for next subscribe to be sent
	getCore()->getPrivate()->clientListEventHandler->addHandler(eventHandler);
	eventHandler->subscribe(getConferenceId());
}

void ClientChatRoom::removeConferenceIdFromPreviousList(const ConferenceId &confId) {
	mPreviousConferenceIds.remove(confId);
	getCore()->getPrivate()->mainDb->removePreviousConferenceId(confId);
}

void ClientChatRoom::addExhumeMessage(const std::shared_ptr<ChatMessage> msg) {
	auto it = std::find(mPendingExhumeMessages.begin(), mPendingExhumeMessages.end(), msg);
	if (it == mPendingExhumeMessages.end()) mPendingExhumeMessages.push_back(msg);
}

void ClientChatRoom::sendChatMessage(const shared_ptr<ChatMessage> &chatMessage) {
	const auto &conference = getConference();
	const auto &conferenceId = conference->getConferenceId();
	const auto &state = getState();

	if ((state == ConferenceInterface::State::Terminated) && !getCurrentParams()->isGroup()) {
		lInfo() << "Trying to send message into a terminated 1-1 chat room [" << conferenceId << "], exhuming it first";
		exhume();
		addExhumeMessage(chatMessage);
	} else if (state == ConferenceInterface::State::Instantiated ||
	           state == ConferenceInterface::State::CreationPending) {
		lInfo() << "Trying to send a message [" << chatMessage << "] in chat room " << this << " [" << conferenceId
		        << "] in a chat room that's not created yet, queuing the message and it will be sent later";
		auto it = std::find(mPendingCreationMessages.begin(), mPendingCreationMessages.end(), chatMessage);
		if (it == mPendingCreationMessages.end()) addPendingMessage(chatMessage);
	} else if (state == ConferenceInterface::State::Created) {
		auto encryptionEngine = getCore()->getEncryptionEngine();
		if (getCurrentParams()->getChatParams()->isEncrypted() && encryptionEngine &&
		    encryptionEngine->participantListRequired() && conference->getParticipantDevices().empty()) {
			lInfo() << "Delaying sending of message [" << chatMessage << "] in the encrypted chat room " << this << " ["
			        << conferenceId
			        << "] because the list of participant devices has not been received yet and the encryption engine "
			        << encryptionEngine << " requires it";
			auto it = std::find(mPendingCreationMessages.begin(), mPendingCreationMessages.end(), chatMessage);
			if (it == mPendingCreationMessages.end()) mPendingCreationMessages.push_back(chatMessage);
		} else {
			ChatRoom::sendChatMessage(chatMessage);
		}
	} else {
		lError() << "Can't send a chat message in a chat room that is in state " << Utils::toString(state);
	}
}

// -----------------------------------------------------------------------------
void ClientChatRoom::sendPendingMessages() {
	const auto &conferenceId = getConferenceId();
	// Now that chat room has been inserted in database, we can send any pending message
	for (const auto &message : mPendingCreationMessages) {
		lInfo() << "Found message [" << message << "] waiting for chat room " << this << " [" << conferenceId
		        << "] to be created, sending it now";
		// First we need to update from & to address of the message,
		// as it was created at a time where the remote address of the chat room may not have been known
		message->getPrivate()->setChatRoom(getSharedFromThis());
		sendChatMessage(message);
	}
	clearPendingCreationMessages();
}

void ClientChatRoom::sendEphemeralUpdate() {
	auto conference = static_pointer_cast<ClientConference>(getConference());
	auto utf8Subject = conference->getUtf8Subject();
	auto focus = conference->mFocus;
	shared_ptr<MediaSession> session = dynamic_pointer_cast<MediaSession>(focus->getSession());
	if (session) {
		auto csp = session->getMediaParams()->clone();
		csp->removeCustomHeader("Ephemeral-Life-Time");
		csp->addCustomHeader("Ephemeral-Life-Time", (ephemeralEnabled() ? to_string(getEphemeralLifetime()) : "0"));
		session->update(csp, CallSession::UpdateMethod::Default, false, utf8Subject);
		delete csp;
	} else {
		session = dynamic_pointer_cast<MediaSession>(conference->createSession());

		const std::shared_ptr<Address> &remoteParticipant = getParticipants().front()->getAddress();
		lInfo() << "Re-INVITing " << *remoteParticipant << " because ephemeral settings of chat room ["
		        << getConferenceId() << "] have changed";

		session->startInvite(nullptr, utf8Subject, nullptr);
	}
}

void ClientChatRoom::setEphemeralMode(AbstractChatRoom::EphemeralMode mode, bool updateDb) {
	if (!getConference()->getMe()->isAdmin()) {
		lError() << "Only admins can choose who can manage ephemeral messages on chatroom "
		         << *getConference()->getConferenceAddress();
		return;
	}

	if (getEphemeralMode() == mode) {
		lWarning() << "Ephemeral messages are already managed by "
		           << ((mode == AbstractChatRoom::EphemeralMode::AdminManaged) ? "the admins" : "each participant");
		return;
	}

	if (!getCurrentParams()->getChatParams()->ephemeralAllowed()) {
		lWarning() << "Ephemeral message mode cannot be changed if chatroom has capability Ephemeral disabled";
		return;
	}

	getCurrentParams()->getChatParams()->setEphemeralMode(mode);

	const auto &lifetime = getEphemeralLifetime();

	if (getState() == ConferenceInterface::State::Created) {
		auto conference = static_pointer_cast<ClientConference>(getConference());
		auto utf8Subject = conference->getUtf8Subject();
		auto focus = conference->mFocus;
		shared_ptr<MediaSession> session = dynamic_pointer_cast<MediaSession>(focus->getSession());
		auto csp = session->getMediaParams()->clone();
		csp->removeCustomHeader("Ephemeral-Life-Time");
		if (mode == AbstractChatRoom::EphemeralMode::AdminManaged) {
			csp->addCustomHeader("Ephemeral-Life-Time", to_string(lifetime));
		}
		lInfo() << "Changing ephemeral mode to " << Utils::toString(mode);
		session->update(csp, CallSession::UpdateMethod::Default, false, utf8Subject);
		delete csp;
	} else {
		lError() << "Cannot change the ClientConference ephemeral lifetime in a state other than Created";
	}

	if (updateDb) {
		shared_ptr<ConferenceEphemeralMessageEvent> event;
		if (mode == AbstractChatRoom::EphemeralMode::AdminManaged) {
			event = make_shared<ConferenceEphemeralMessageEvent>(
			    EventLog::Type::ConferenceEphemeralMessageManagedByAdmin, time(nullptr), getConferenceId(), lifetime);
		} else {
			event = make_shared<ConferenceEphemeralMessageEvent>(
			    EventLog::Type::ConferenceEphemeralMessageManagedByParticipants, time(nullptr), getConferenceId(),
			    lifetime);
		}
		addEvent(event);
		_linphone_chat_room_notify_ephemeral_event(toC(), L_GET_C_BACK_PTR(event));
	}
}

AbstractChatRoom::EphemeralMode ClientChatRoom::getEphemeralMode() const {
	return getCurrentParams()->getChatParams()->getEphemeralMode();
}

void ClientChatRoom::enableEphemeral(bool ephem, bool updateDb) {
	if (ephemeralEnabled() == ephem) {
		if (linphone_core_get_global_state(getCore()->getCCore()) != LinphoneGlobalStartup) {
			lWarning() << "Ephemeral messages of chat room " << getConferenceId() << " are already "
			           << (ephem ? "enabled" : "disabled");
		}
		return;
	}

	LinphoneImNotifPolicy *policy = linphone_core_get_im_notif_policy(getCore()->getCCore());
	if (!linphone_im_notif_policy_get_send_imdn_displayed(policy) && ephem) {
		lWarning() << "Ephemeral messages may not work correctly because IMDN messages are disabled";
	}

	getCurrentParams()->getChatParams()->enableEphemeral(ephem);
	const string active = ephem ? "enabled" : "disabled";
	lDebug() << "Ephemeral message is " << active << " in chat room [" << getConferenceId() << "]";

	auto lifetime = getEphemeralLifetime();
	if (getEphemeralMode() == AbstractChatRoom::EphemeralMode::AdminManaged) {
		if (!getConference()->getMe()->isAdmin()) {
			lError() << "Only admins can enable or disable ephemeral messages on chatroom "
			         << *getConference()->getConferenceAddress();
			return;
		}

		if (getState() == ConferenceInterface::State::Created) {
			if (ephem && (lifetime == 0)) {
				lifetime = linphone_core_get_default_ephemeral_lifetime(getCore()->getCCore());
				getCurrentParams()->getChatParams()->setEphemeralLifetime(lifetime);
				if (updateDb) {
					lInfo() << "Reset ephemeral lifetime of chat room " << getConferenceId()
					        << " to the default value of " << lifetime
					        << " because ephemeral messages were enabled with a time equal to 0.";
					getCore()->getPrivate()->mainDb->updateChatRoomEphemeralLifetime(getConferenceId(), lifetime);

					shared_ptr<ConferenceEphemeralMessageEvent> event = make_shared<ConferenceEphemeralMessageEvent>(
					    EventLog::Type::ConferenceEphemeralMessageLifetimeChanged, time(nullptr), getConferenceId(),
					    lifetime);
					addEvent(event);
				}
			}
			sendEphemeralUpdate();
		} else {
			lError() << "Cannot change the ClientConference ephemeral lifetime in a state other than Created";
		}
	}

	if (updateDb) {
		getCore()->getPrivate()->mainDb->updateChatRoomEphemeralEnabled(getConferenceId(), ephem);
		shared_ptr<ConferenceEphemeralMessageEvent> event;
		if (ephem)
			event = make_shared<ConferenceEphemeralMessageEvent>(EventLog::Type::ConferenceEphemeralMessageEnabled,
			                                                     time(nullptr), getConferenceId(), lifetime);
		else
			event = make_shared<ConferenceEphemeralMessageEvent>(EventLog::Type::ConferenceEphemeralMessageDisabled,
			                                                     time(nullptr), getConferenceId(), lifetime);
		addEvent(event);
		_linphone_chat_room_notify_ephemeral_event(toC(), L_GET_C_BACK_PTR(event));
	}
}

bool ClientChatRoom::ephemeralEnabled() const {
	return getCurrentParams()->getChatParams()->ephemeralEnabled();
}

void ClientChatRoom::setEphemeralLifetime(long lifetime, bool updateDb) {
	if (lifetime == getEphemeralLifetime()) {
		if (updateDb)
			lWarning() << "Ephemeral lifetime of chat room " << getConferenceId()
			           << " will not be changed! Trying to set the same ephemaral lifetime as before : " << lifetime;
		return;
	}

	if ((getState() == ConferenceInterface::State::Instantiated) ||
	    (getState() == ConferenceInterface::State::CreationPending)) {
		// Do not print this log when creating chat room from DB
		if (updateDb)
			lInfo() << "Set new ephemeral lifetime of chat room " << getConferenceId() << " to " << lifetime
			        << " while creating the chat room, used to be " << getEphemeralLifetime() << ".";
		getCurrentParams()->getChatParams()->setEphemeralLifetime(lifetime);
		return;
	}

	if (getEphemeralMode() == AbstractChatRoom::EphemeralMode::AdminManaged) {
		if (!getConference()->getMe()->isAdmin()) {
			lError() << "Cannot change the ClientConference ephemeral lifetime because I am not admin";
			return;
		}

		const auto &state = getState();
		if (state == ConferenceInterface::State::Created) {
			if (updateDb)
				lInfo() << "Set new ephemeral lifetime of chat room " << getConferenceId() << " to " << lifetime
				        << ", used to be " << getEphemeralLifetime() << ".";
			getCurrentParams()->getChatParams()->setEphemeralLifetime(lifetime);
			const bool enable = (lifetime != 0);
			// If only changing the value of the message lifetime
			if (ephemeralEnabled() == enable) {
				sendEphemeralUpdate();
			} else {
				enableEphemeral(enable, updateDb);
			}
		} else {
			lError() << "Cannot change the ephemeral lifetime of chat room " << getConferenceId() << " to " << lifetime
			         << " in a state other than Created - currently it is in state " << Utils::toString(state);
		}
	} else {
		if (updateDb)
			lInfo() << "Set new ephemeral lifetime to " << lifetime << ", used to be " << getEphemeralLifetime() << ".";
		getCurrentParams()->getChatParams()->setEphemeralLifetime(lifetime);
	}

	if (updateDb) {
		getCore()->getPrivate()->mainDb->updateChatRoomEphemeralLifetime(getConferenceId(), lifetime);

		if (getCurrentParams()
		        ->getChatParams()
		        ->ephemeralEnabled()) { // Do not create event if ephemeral feature is disabled
			shared_ptr<ConferenceEphemeralMessageEvent> event = make_shared<ConferenceEphemeralMessageEvent>(
			    EventLog::Type::ConferenceEphemeralMessageLifetimeChanged, time(nullptr), getConferenceId(), lifetime);
			addEvent(event);
			_linphone_chat_room_notify_ephemeral_event(toC(), L_GET_C_BACK_PTR(event));
		}
	}
}

long ClientChatRoom::getEphemeralLifetime() const {
	return getCurrentParams()->getChatParams()->getEphemeralLifetime();
}

bool ClientChatRoom::ephemeralSupportedByAllParticipants() const {
	// TODO
	return false;
}

LINPHONE_END_NAMESPACE
