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

#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "chat/chat-message/chat-message-p.h"
#include "chat/cpim/message/cpim-message.h"
#include "chat/modifier/cpim-chat-message-modifier.h"
#include "conference/handlers/server-conference-event-handler.h"
#include "conference/handlers/server-conference-list-event-handler.h"
#include "conference/participant-info.h"
#include "conference/participant.h"
#include "conference/server-conference.h"
#include "conference/session/call-session-p.h"
#include "content/content-disposition.h"
#include "content/content-type.h"
#include "core/core-p.h"
#include "event-log/events.h"
#include "factory/factory.h"
#include "linphone/api/c-chat-room-cbs.h"
#include "linphone/api/c-chat-room.h"
#include "linphone/api/c-types.h"
#include "linphone/wrapper_utils.h"
#include "logger/logger.h"
#include "sal/refer-op.h"
#include "server-chat-room.h"
#include "sip-tools/sip-headers.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ServerChatRoom::ServerChatRoom(const std::shared_ptr<Core> &core, const std::shared_ptr<Conference> &conf)
    : ChatRoom(core, conf) {
	mProtocolVersion = CorePrivate::groupChatProtocolVersion;
}

ServerChatRoom::~ServerChatRoom() {
	lInfo() << this << " destroyed.";
};

// =============================================================================

void ServerChatRoom::confirmCreation() {
	static_pointer_cast<ServerConference>(getConference())->confirmCreation();
}

/* This is called only when a participant attempts to create a one to one chatroom that already exists.
 * We just redirect it to the existing chatroom uri. */
void ServerChatRoom::confirmRecreation(SalCallOp *op) {
	const auto from = Address::create(op->getFrom());
	const auto to = Address::create(op->getTo());
	auto participant = getConference()->findInvitedParticipant(from);
	if (!participant) {
		lError() << this << " bug - " << *from << " is not a participant.";
		op->decline(SalReasonInternalError, "");
		return;
	}

	lInfo() << this << " is re-joined by " << *participant->getAddress();
	shared_ptr<Participant> me = getConference()->getMe();
	shared_ptr<CallSession> session = me->createSession(*getConference().get(), nullptr, false);
	session->addListener(getConference());
	session->configure(LinphoneCallIncoming, nullptr, op, from, to);
	session->startIncomingNotification(false);
	auto confAddr = *getConference()->getConferenceAddress();
	confAddr.setParam(Conference::IsFocusParameter);
	session->redirect(confAddr);
}

// -----------------------------------------------------------------------------

LinphoneReason ServerChatRoom::onSipMessageReceived(SalOp *op, const SalMessage *message) {
	// Check that the message is coming from a participant of the chat room
	std::shared_ptr<Address> fromAddr = Address::create(op->getFrom());
	if (!getConference()->findParticipant(fromAddr)) {
		return LinphoneReasonForbidden;
	}

	ContentType contentType(message->content_type);
	std::string contentBody(message->text ? message->text : "");
	// Do not check that we received a CPIM message because ciphered messages are not
	shared_ptr<ServerChatRoom::Message> msg =
	    make_shared<ServerChatRoom::Message>(op->getFrom(), contentType, contentBody, op->getRecvCustomHeaders());
	queueMessage(msg);
	dispatchQueuedMessages();
	return LinphoneReasonNone;
}

void ServerChatRoom::setConferenceAddress(const std::shared_ptr<Address> &conferenceAddress) {
	getConference()->setConferenceAddress(conferenceAddress);
}

void ServerChatRoom::updateProtocolVersionFromDevice(const shared_ptr<ParticipantDevice> &device) {
	auto protocols = Utils::parseCapabilityDescriptor(device->getCapabilityDescriptor());
	auto groupchat = protocols.find("groupchat");
	const auto &deviceAddress = device->getAddress();
	if (groupchat == protocols.end()) {
		lError() << "Device " << *deviceAddress
		         << " has no groupchat capability set: " << device->getCapabilityDescriptor();
		return;
	}
	if (mProtocolVersion > groupchat->second) {
		mProtocolVersion = groupchat->second;
		lWarning() << "Device " << *deviceAddress << " downgrades chatroom's protocol version to "
		           << mProtocolVersion.toString();
	}
}

void ServerChatRoom::determineProtocolVersion() {
	mProtocolVersion = CorePrivate::groupChatProtocolVersion;
	for (const auto &participant : getParticipants()) {
		for (const auto &device : participant->getDevices()) {
			updateProtocolVersionFromDevice(device);
		}
	}
}

void ServerChatRoom::finalizeCreation() {
	static_pointer_cast<ServerConference>(getConference())->finalizeCreation();
}

/*
 * This method is to be called by the conference server when it is notified that a device has just registered*/
void ServerChatRoom::notifyParticipantDeviceRegistration(const std::shared_ptr<const Address> &participantDevice) {
	shared_ptr<Participant> participant = getConference()->findInvitedParticipant(participantDevice);
	if (!participant) {
		lError() << this << ": " << *participantDevice << " is not part of the chatroom.";
		return;
	}
	shared_ptr<ParticipantDevice> pd = participant->findDevice(participantDevice);
	if (!pd) {
		/* A device that does not have the required capabilities may be notified. */
		lInfo() << this << ": device " << *participantDevice << " is not part of any participant of the chatroom.";
		return;
	}
	static_pointer_cast<ServerConference>(getConference())->updateParticipantDeviceSession(pd, true);
}

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
std::shared_ptr<Participant>
ServerChatRoom::getOtherParticipant(const std::shared_ptr<Participant> someParticipant) const {
	std::shared_ptr<Participant> otherParticipant;
	bool looksSane = false;
	if (!getCurrentParams()->isGroup()) {
		for (auto &p : getParticipants()) {
			if (p == someParticipant) looksSane = true;
			else otherParticipant = p;
		}
		/* Yes I'm paranoid.*/
		if (!looksSane) {
			lError() << "getOtherParticipant() reference participant not found !";
		} else if (!otherParticipant) {
			lError() << "getOtherParticipant() other participant not found !";
		}
	} else lError() << "getOtherParticipant() used for not a 1-1 chatroom.";
	return otherParticipant;
}

void ServerChatRoom::onBye(const shared_ptr<ParticipantDevice> &participantLeaving) {
	bool shouldRemoveParticipant = true;

	if (!getCurrentParams()->isGroup()) {
		if (mProtocolVersion < Utils::Version(1, 1)) {
			/*
			 * In protocol 1.0, unlike normal group chatrooms, a participant can never leave a one to one chatroom.
			 * The receiving of BYE is instead interpreted as a termination of the SIP session specific for the device.
			 */
			shouldRemoveParticipant = false;
		} else {
			/*
			 * In subsequent protocol versions, both participants of a one to one chatroom are removed,
			 * which terminates the chatroom forever.
			 */
			lInfo() << "1-1 chatroom was left by one participant, removing other participant to terminate the chatroom";
			auto otherParticipant = getOtherParticipant(participantLeaving->getParticipant()->getSharedFromThis());
			if (otherParticipant) {
				getConference()->removeParticipant(otherParticipant);
				// Do not wait to delete the chat room from the list stored in the core as the process of terminating it
				// has already started and it cannot be restored
				lInfo() << this << ": Delete chatroom from MainDB as last participant has left";
				unique_ptr<MainDb> &mainDb = getCore()->getPrivate()->mainDb;
				mainDb->deleteChatRoom(getConferenceId());
			}
			setState(ConferenceInterface::State::TerminationPending);
		}
	}
	if (shouldRemoveParticipant) {
		// Participant leaves the chat room on its own by sending a BYE.
		// Set it the Leaving state first, so that removeParticipant() does nothing with it (does not attempt to send
		// a BYE...).
		static_pointer_cast<ServerConference>(getConference())
		    ->setParticipantDeviceState(participantLeaving, ParticipantDevice::State::Leaving);
		// Remove participant will change other devices to Leaving state.
		getConference()->removeParticipant(participantLeaving->getParticipant()->getSharedFromThis());
		// But since we received its BYE, it is actually already left.
	}
	// Notify the lient only if the participant should be removed. Doing so, the client may not be able to send any
	// further message to this device in particular if using an old version of a secured chat room
	static_pointer_cast<ServerConference>(getConference())
	    ->setParticipantDeviceState(participantLeaving, ParticipantDevice::State::Left, shouldRemoveParticipant);
}

shared_ptr<Core> ServerChatRoom::getCore() const {
	return ChatRoom::getCore();
}

ServerChatRoom::CapabilitiesMask ServerChatRoom::getCapabilities() const {
	return ConferenceParams::toCapabilities(getCurrentParams());
}

void ServerChatRoom::unSubscribeRegistrationForParticipant(BCTBX_UNUSED(const std::shared_ptr<Address> &identAddress)) {
	auto p = mRegistrationSubscriptions.find(identAddress->toString());
	if (p == mRegistrationSubscriptions.end()) {
		lError() << "Conference " << *getConference()->getConferenceAddress() << " no active subscription for "
		         << identAddress;
		return;
	}
	mRegistrationSubscriptions.erase(p);
	getConference()->removeInvitedParticipant(identAddress);

	LinphoneAddress *laddr = identAddress->toC();
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, getSharedFromThis(),
	                                  linphone_chat_room_cbs_get_participant_registration_unsubscription_requested,
	                                  laddr);
}

/*
 * This function setups registration subscriptions if not already there.
 * If no registration subscription is started (because they were all running already), it returns false.
 * newInvited specifies whether these participants are in the process of being invited, in which case they will be
 * automatically added to the mRegistrationSubscriptionParticipants list, so that when registration info arrives, they
 * will be added.
 */
bool ServerChatRoom::subscribeRegistrationForParticipants(
    const std::list<std::shared_ptr<const Address>> &identAddresses, bool newInvited) {
	bool subscriptionsPending = false;
	LinphoneGlobalState gs = linphone_core_get_global_state(getCore()->getCCore());
	if (gs != LinphoneGlobalOn) {
		lError() << "The core is currently in state " << std::string(linphone_global_state_to_string(gs))
		         << " but registration subscriptions can only be set up when the core is in state GlobalOn";
		return false;
	}

	std::list<Address> requestedAddresses;

	// Subscribe to the registration events from the proxy
	for (const auto &addr : identAddresses) {
		const auto cleanedAddr = addr->getUri();
		if (mRegistrationSubscriptions.find(cleanedAddr.toString()) == mRegistrationSubscriptions.end()) {
			requestedAddresses.emplace_back(cleanedAddr);
			if (newInvited) mRegistrationSubscriptionParticipants.emplace_back(cleanedAddr);
			mUnnotifiedRegistrationSubscriptions++;
			subscriptionsPending = true;
		}
	}

	for (const auto &addr : requestedAddresses) {
		const auto laddr = addr.toC();
		// we 'll put here later a context pointer returned by the callback.
		mRegistrationSubscriptions[addr.toString()].context = nullptr;
		LINPHONE_HYBRID_OBJECT_INVOKE_CBS(ChatRoom, getSharedFromThis(),
		                                  linphone_chat_room_cbs_get_participant_registration_subscription_requested,
		                                  laddr);
	}
	return subscriptionsPending;
}

bool ServerChatRoom::removeRegistrationSubscriptionParticipant(const std::shared_ptr<Address> &participantAddress) {
	// Check if this registration information is for a participant in the process of being added.
	auto it = std::find(mRegistrationSubscriptionParticipants.begin(), mRegistrationSubscriptionParticipants.end(),
	                    *participantAddress);
	if (it != mRegistrationSubscriptionParticipants.end()) {
		mRegistrationSubscriptionParticipants.erase(it);
		mUnnotifiedRegistrationSubscriptions--;
		return true;
	}
	return false;
}

void ServerChatRoom::allowCpim(BCTBX_UNUSED(bool value)) {
}

void ServerChatRoom::allowMultipart(BCTBX_UNUSED(bool value)) {
}

bool ServerChatRoom::canHandleCpim() const {
	return true;
}

bool ServerChatRoom::canHandleMultipart() const {
	return true;
}

bool ServerChatRoom::hasBeenLeft() const {
	return false;
}

bool ServerChatRoom::isReadOnly() const {
	return false;
}

void ServerChatRoom::queueMessage(const shared_ptr<ServerChatRoom::Message> &msg) {
	for (const auto &participant : getParticipants()) {
		for (const auto &device : participant->getDevices()) {
			// Queue the message for all devices except the one that sent it
			if (*msg->fromAddr != *device->getAddress()) {
				queueMessage(msg, device->getAddress());
			}
		}
	}
}

void ServerChatRoom::queueMessage(const shared_ptr<ServerChatRoom::Message> &msg,
                                  const std::shared_ptr<Address> &deviceAddress) {
	chrono::system_clock::time_point timestamp = chrono::system_clock::now();
	string uri(deviceAddress->toString());
	// Remove queued messages older than one week
	while (!mQueuedMessages[uri].empty()) {
		shared_ptr<ServerChatRoom::Message> m = mQueuedMessages[uri].front();
		chrono::hours age = chrono::duration_cast<chrono::hours>(timestamp - m->timestamp);
		chrono::hours oneWeek(168);
		if (age < oneWeek) break;
		mQueuedMessages[uri].pop();
	}
	mQueuedMessages[uri].push(msg);
}

void ServerChatRoom::removeQueuedParticipantMessages(const shared_ptr<Participant> &participant) {
	if (participant) mQueuedMessages.erase(participant->getAddress()->toString());
}

void ServerChatRoom::sendMessage(BCTBX_UNUSED(const shared_ptr<ServerChatRoom::Message> &message),
                                 BCTBX_UNUSED(const std::shared_ptr<Address> &deviceAddr)) {
	shared_ptr<ChatMessage> msg = createChatMessage();
	copyMessageHeaders(message, msg);
	// Special custom header to identify MESSAGE that belong to server group chatroom
	msg->getPrivate()->addSalCustomHeader("Session-mode", "true");
	msg->setInternalContent(message->content);
	msg->getPrivate()->forceFromAddress(getConference()->getConferenceAddress());
	msg->getPrivate()->forceToAddress(deviceAddr);
	msg->getPrivate()->setApplyModifiers(false);
	if (message->fromAddr->getUsername() == msg->getToAddress()->getUsername() &&
	    message->fromAddr->getDomain() == msg->getToAddress()->getDomain()) {
		// If FROM and TO are the same user (with a different device for example, gruu is not checked), set the
		// X-fs-message-type header to "chat-service". This lead to disabling push notification for this message.
		msg->getPrivate()->addSalCustomHeader(XFsMessageTypeHeader::HeaderName, XFsMessageTypeHeader::ChatService);
	}

	if (message->contentsList.size() > 0) {
		msg->setProperty("content-list", message->contentsList);
	}

	msg->send();
}

bool ServerChatRoom::dispatchMessagesAfterFullState(BCTBX_UNUSED(const shared_ptr<CallSession> &session)) const {
#ifdef HAVE_ADVANCED_IM
	const auto &conference = getConference();
	auto device = conference->findInvitedParticipantDevice(session);
	if (!device) {
		lWarning() << *conference << ": unable to dispatch messages to a device attached to " << *session
		           << " (local address " << *session->getLocalAddress() << " remote address "
		           << *session->getRemoteAddress() << ")";
		return false; // Assume it is a recent device.
	}
	return dispatchMessagesAfterFullState(device);
#else
	return false;
#endif // HAVE_ADVANCED_IM
}

bool ServerChatRoom::dispatchMessagesAfterFullState(const shared_ptr<ParticipantDevice> &device) const {
	auto protocols = Utils::parseCapabilityDescriptor(device->getCapabilityDescriptor());
	auto groupchat = protocols.find("groupchat");
	return ((groupchat == protocols.end()) || (groupchat->second < Utils::Version(1, 2)));
}

void ServerChatRoom::dispatchQueuedMessages() {
	for (const auto &participant : getParticipants()) {
		/*
		 * Dispatch messages for each device in Present state. In a one to one chatroom, if a device
		 * is found is Left state, it must be invited first.
		 */
		for (const auto &device : participant->getDevices()) {
			string uri(device->getAddress()->toString());
			auto &msgQueue = mQueuedMessages[uri];

			if (!msgQueue.empty()) {
				if (!getCurrentParams()->isGroup() && (device->getState() == ParticipantDevice::State::Left)) {
					// Happens only with protocol < 1.1
					lInfo() << "There is a message to transmit to a participant in left state in a one to one "
					           "chatroom, so inviting first.";
					static_pointer_cast<ServerConference>(getConference())->inviteDevice(device);
					continue;
				}
				if (device->getState() != ParticipantDevice::State::Present) continue;
				size_t nbMessages = msgQueue.size();
				lInfo() << "Conference " << *getConference()->getConferenceAddress() << ": Dispatching " << nbMessages
				        << " queued message(s) for '" << uri << "'";
				while (!msgQueue.empty()) {
					shared_ptr<ServerChatRoom::Message> msg = msgQueue.front();
					sendMessage(msg, device->getAddress());
					msgQueue.pop();
				}
			}
		}
	}
}

std::shared_ptr<ServerChatRoom::Message> ServerChatRoom::createMessage(const std::string &from,
                                                                       const ContentType &contentType,
                                                                       const std::string &text,
                                                                       const SalCustomHeader *salCustomHeaders) {
	return std::make_shared<ServerChatRoom::Message>(from, contentType, text, salCustomHeaders);
}

void ServerChatRoom::copyMessageHeaders(const shared_ptr<ServerChatRoom::Message> &fromMessage,
                                        const shared_ptr<ChatMessage> &toMessage) {
	static const string headersToCopy[] = {"Content-Encoding", "Expires", "Priority", XFsEventIdHeader::HeaderName};
	for (const auto &headerName : headersToCopy) {
		const char *headerValue = sal_custom_header_find(fromMessage->customHeaders, headerName.c_str());
		if (headerValue) toMessage->getPrivate()->addSalCustomHeader(headerName, headerValue);
	}
}

void ServerChatRoom::handleEphemeralSettingsChange(const shared_ptr<CallSession> &session) {
	if (getCurrentParams()->getChatParams()->ephemeralAllowed()) {
		const auto op = session->getPrivate()->getOp();
		string ephemeralLifeTime =
		    L_C_TO_STRING(sal_custom_header_find(op->getRecvCustomHeaders(), "Ephemeral-Life-Time"));
		if (ephemeralLifeTime.empty()) {
			setEphemeralModeForDevice(AbstractChatRoom::EphemeralMode::DeviceManaged, session);
		} else {
			setEphemeralLifetimeForDevice(std::stol(ephemeralLifeTime, nullptr), session);
		}
	}
}

void ServerChatRoom::setEphemeralModeForDevice(AbstractChatRoom::EphemeralMode mode,
                                               const shared_ptr<CallSession> &session) {
	lInfo() << "Conference " << *getConference()->getConferenceAddress() << ": New mode is: " << mode;
	getCurrentParams()->getChatParams()->setEphemeralMode(mode);

	const auto device = getConference()->findParticipantDevice(session);
	if (device) {
		time_t creationTime = time(nullptr);
		const auto eventType = (mode == AbstractChatRoom::EphemeralMode::AdminManaged)
		                           ? EventLog::Type::ConferenceEphemeralMessageManagedByAdmin
		                           : EventLog::Type::ConferenceEphemeralMessageManagedByParticipants;
		static_pointer_cast<ServerConference>(getConference())
		    ->notifyEphemeralModeChanged(creationTime, false, eventType);
	} else {
		lWarning() << "Unable to find device among those of the participants that changed ephemeral message mode to "
		           << mode;
	}
}

void ServerChatRoom::setEphemeralLifetimeForDevice(long lifetime, const shared_ptr<CallSession> &session) {
	lInfo() << "Conference " << *getConference()->getConferenceAddress() << ": New ephemeral time: " << lifetime;
	getCurrentParams()->getChatParams()->setEphemeralLifetime(lifetime);

	const auto device = getConference()->findParticipantDevice(session);
	if (device) {
		time_t creationTime = time(nullptr);
		static_pointer_cast<ServerConference>(getConference())
		    ->notifyEphemeralLifetimeChanged(creationTime, false, lifetime);
	} else {
		lWarning()
		    << "Unable to find device among those of the participants that changed ephemeral message lifetime to "
		    << lifetime;
	}
}

// -----------------------------------------------------------------------------

ostream &operator<<(ostream &stream, const ServerChatRoom *chatRoom) {
	// TODO: Is conference ID needed to be stored in both client conference and chat room base classes?
	const auto peerAddress = chatRoom->getConferenceId().getPeerAddress();
	return stream << "ServerChatRoom [" << (peerAddress ? peerAddress->toString() : std::string("unknown address"))
	              << "]";
}

LINPHONE_END_NAMESPACE
