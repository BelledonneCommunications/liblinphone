/*
 * server-group-chat-room.cpp
 * Copyright (C) 2010-2018 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <algorithm>

#include "address/address-p.h"
#include "address/address.h"
#include "address/identity-address.h"
#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "chat/chat-message/chat-message-p.h"
#include "chat/modifier/cpim-chat-message-modifier.h"
#include "conference/handlers/local-conference-event-handler.h"
#include "conference/handlers/local-conference-list-event-handler.h"
#include "conference/local-conference-p.h"
#include "conference/participant-p.h"
#include "conference/session/call-session-p.h"
#include "content/content-disposition.h"
#include "content/content-type.h"
#include "core/core-p.h"
#include "event-log/events.h"
#include "logger/logger.h"
#include "sal/refer-op.h"
#include "server-group-chat-room-p.h"

#include "linphone/wrapper_utils.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

#define CALL_CHAT_ROOM_CBS(cr, cbName, functionName, ...) \
	bctbx_list_t *callbacksCopy = bctbx_list_copy(linphone_chat_room_get_callbacks_list(cr)); \
	for (bctbx_list_t *it = callbacksCopy; it; it = bctbx_list_next(it)) { \
		linphone_chat_room_set_current_callbacks(cr, reinterpret_cast<LinphoneChatRoomCbs *>(bctbx_list_get_data(it))); \
		LinphoneChatRoomCbs ## cbName ## Cb cb = linphone_chat_room_cbs_get_ ## functionName (linphone_chat_room_get_current_callbacks(cr)); \
		if (cb) \
			cb(__VA_ARGS__); \
	} \
	linphone_chat_room_set_current_callbacks(cr, nullptr); \
	bctbx_list_free(callbacksCopy);

void ServerGroupChatRoomPrivate::setState (ChatRoom::State state) {
	L_Q();
	L_Q_T(LocalConference, qConference);
	ChatRoomPrivate::setState(state);
	if (state == ChatRoom::State::Created) {
		// Handle transitional states (joining and leaving of participants)
		// This is needed when the chat room is loaded from its state in database
		for (const auto &participant : qConference->getPrivate()->participants) {
			bool atLeastOneDeviceLeaving = false;
			bool atLeastOneDeviceJoining = false;
			bool atLeastOneDevicePresent = false;
			for (const auto &device : participant->getPrivate()->getDevices()) {
				switch (getParticipantDeviceState(device)) {
					case ParticipantDevice::State::Leaving:
						atLeastOneDeviceLeaving = true;
						break;
					case ParticipantDevice::State::Joining:
						atLeastOneDeviceJoining = true;
						break;
					case ParticipantDevice::State::Present:
						atLeastOneDevicePresent = true;
						break;
					case ParticipantDevice::State::Left:
						break;
				}
			}

			if (atLeastOneDeviceLeaving) {
				q->removeParticipant(participant);
			} else {
				if (atLeastOneDeviceJoining) {
					for (const auto &device : participant->getPrivate()->getDevices()) {
						if (getParticipantDeviceState(device) == ParticipantDevice::State::Joining)
							inviteDevice(device);
					}
				}
				if (atLeastOneDevicePresent || atLeastOneDeviceJoining)
					filteredParticipants.push_back(participant);
			}
		}

		// Subscribe to the registration events from the proxy
		for (const auto &participant : qConference->getPrivate()->participants) {
			LinphoneChatRoom *cr = L_GET_C_BACK_PTR(q);
			LinphoneAddress *laddr = linphone_address_new(participant->getAddress().asString().c_str());
			CALL_CHAT_ROOM_CBS(cr, ParticipantRegistrationSubscriptionRequested, participant_registration_subscription_requested, cr, laddr);
			linphone_address_unref(laddr);
		}
	}
}

shared_ptr<Participant> ServerGroupChatRoomPrivate::addParticipant (const IdentityAddress &addr) {
	L_Q();
	L_Q_T(LocalConference, qConference);
	shared_ptr<Participant> participant = q->findParticipant(addr);
	if (!participant) {
		participant = make_shared<Participant>(qConference, addr);
		qConference->getPrivate()->participants.push_back(participant);
		shared_ptr<ConferenceParticipantEvent> event = qConference->getPrivate()->eventHandler->notifyParticipantAdded(addr);
		q->getCore()->getPrivate()->mainDb->addEvent(event);
	}
	filteredParticipants.remove(participant);
	filteredParticipants.push_back(participant);
	return participant;
}

ParticipantDevice::State ServerGroupChatRoomPrivate::getParticipantDeviceState (const shared_ptr<const ParticipantDevice> &device) const {
	return device->getState();
}

void ServerGroupChatRoomPrivate::setParticipantDeviceState (const shared_ptr<ParticipantDevice> &device, ParticipantDevice::State state) {
	L_Q();
	string address(device->getAddress().asString());
	lInfo() << q << ": Set participant device '" << address << "' state to " << state;
	device->setState(state);
	q->getCore()->getPrivate()->mainDb->updateChatRoomParticipantDevice(q->getSharedFromThis(), device);
	if (state == ParticipantDevice::State::Leaving)
		queuedMessages.erase(address);
}

void ServerGroupChatRoomPrivate::acceptSession (const shared_ptr<CallSession> &session) {
	if (session->getState() == CallSession::State::UpdatedByRemote)
		session->acceptUpdate();
	else
		session->accept();
	joiningPendingAfterCreation = false;
}

void ServerGroupChatRoomPrivate::confirmCreation () {
	L_Q();

	shared_ptr<Participant> me = q->getMe();
	shared_ptr<CallSession> session = me->getPrivate()->getSession();
	session->startIncomingNotification(false);

	LinphoneChatRoom *cr = L_GET_C_BACK_PTR(q);
	CALL_CHAT_ROOM_CBS(cr, ConferenceAddressGeneration, conference_address_generation, cr);
}

void ServerGroupChatRoomPrivate::confirmJoining (SalCallOp *op) {
	L_Q();
	L_Q_T(LocalConference, qConference);
	shared_ptr<Participant> participant;

	Address contactAddr(op->getRemoteContact());
	if (contactAddr.getUriParamValue("gr").empty()) {
		lError() << q << ": Declining INVITE because the contact does not have a 'gr' uri parameter [" << contactAddr.asString() << "]";
		op->decline(SalReasonDeclined, nullptr);
		joiningPendingAfterCreation = false;
		return;
	}

	IdentityAddress gruu(contactAddr);
	shared_ptr<ParticipantDevice> device;
	shared_ptr<CallSession> session;
	if (joiningPendingAfterCreation) {
		// Check if the participant is already there, this INVITE may come from an unknown device of an already present participant
		participant = addParticipant(IdentityAddress(op->getFrom()));
		participant->getPrivate()->setAdmin(true);
		device = participant->getPrivate()->addDevice(gruu);
		session = device->getSession();
		shared_ptr<ConferenceParticipantDeviceEvent> deviceEvent = qConference->getPrivate()->eventHandler->notifyParticipantDeviceAdded(participant->getAddress(), gruu);
		q->getCore()->getPrivate()->mainDb->addEvent(deviceEvent);
		if (!(capabilities & ServerGroupChatRoom::Capabilities::OneToOne)) {
			shared_ptr<ConferenceParticipantEvent> adminEvent = qConference->getPrivate()->eventHandler->notifyParticipantSetAdmin(participant->getAddress(), true);
			q->getCore()->getPrivate()->mainDb->addEvent(adminEvent);
		}
	} else {
		// INVITE coming from an invited participant
		participant = q->findParticipant(IdentityAddress(op->getFrom()));
		if (!participant) {
			lError() << q << ": Declining INVITE coming from someone that is not a participant";
			op->decline(SalReasonDeclined, nullptr);
			joiningPendingAfterCreation = false;
			return;
		}
		if (capabilities & ServerGroupChatRoom::Capabilities::OneToOne)
			participant->getPrivate()->setAdmin(true);
		device = participant->getPrivate()->addDevice(gruu);
		session = device->getSession();
	}

	if (!session || (session->getPrivate()->getOp() != op)) {
		session = participant->getPrivate()->createSession(*q, nullptr, false, this);
		session->configure(LinphoneCallIncoming, nullptr, op, participant->getAddress(), Address(op->getTo()));
		session->startIncomingNotification(false);
		Address addr = qConference->getPrivate()->conferenceAddress;
		addr.setParam("isfocus");
		session->getPrivate()->getOp()->setContactAddress(addr.getPrivate()->getInternalAddress());
		device->setSession(session);
	}

	// Changes are only allowed from admin participants
	if (participant->isAdmin()) {
		bool res = update(op);
		if (!res) {
			if (joiningPendingAfterCreation) {
				lError() << q << ": Declining INVITE because we expected a non-empty list of participants to invite";
				op->decline(SalReasonNotAcceptable, nullptr);
				chatRoomListener->onChatRoomDeleteRequested(q->getSharedFromThis());
			} else {
				acceptSession(session);
			}
			return;
		}
	} else {
		acceptSession(session);
	}
}

void ServerGroupChatRoomPrivate::confirmRecreation (SalCallOp *op) {
	L_Q();
	L_Q_T(LocalConference, qConference);
	IdentityAddress confAddr(qConference->getPrivate()->conferenceAddress);
	lInfo() << q << " recreated";
	Address addr(confAddr);
	addr.setParam("isfocus");
	shared_ptr<Participant> me = q->getMe();
	shared_ptr<CallSession> session = me->getPrivate()->createSession(*q, nullptr, false, this);
	session->configure(LinphoneCallIncoming, nullptr, op, Address(op->getFrom()), Address(op->getTo()));
	session->startIncomingNotification(false);
	session->redirect(addr);
	joiningPendingAfterCreation = true;
}

void ServerGroupChatRoomPrivate::declineSession (const shared_ptr<CallSession> &session, LinphoneReason reason) {
	session->decline(reason);
	joiningPendingAfterCreation = false;
}

void ServerGroupChatRoomPrivate::dispatchQueuedMessages () {
	L_Q();
	for (const auto &participant : q->getParticipants()) {
		// Check if some messages has been queued for the participant address
		// This can happen when a prior participant of a one-to-one chatroom if
		// re-invited by the receiving of a chat message. At the moment of the receiving of this
		// chat message, we do not know yet the devices of the re-invited participant.
		auto devices = participant->getPrivate()->getDevices();
		if (!devices.empty()) {
			string participantUri(participant->getAddress().asString());
			while (!queuedMessages[participantUri].empty()) {
				for (const auto &device : participant->getPrivate()->getDevices()) {
					queuedMessages[device->getAddress().asString()].push(queuedMessages[participantUri].front());
				}
				queuedMessages[participantUri].pop();
			}
		}
		// Dispatch messages for each device
		for (const auto &device : participant->getPrivate()->getDevices()) {
			if (getParticipantDeviceState(device) != ParticipantDevice::State::Present)
				continue;
			string uri(device->getAddress().asString());
			size_t nbMessages = queuedMessages[uri].size();
			if (nbMessages > 0)
				lInfo() << q << ": Dispatching " << nbMessages << " queued message(s) for '" << uri << "'";
			while (!queuedMessages[uri].empty()) {
				shared_ptr<Message> msg = queuedMessages[uri].front();
				dispatchMessage(msg, uri);
				queuedMessages[uri].pop();
			}
		}
	}
}

void ServerGroupChatRoomPrivate::removeParticipant (const shared_ptr<const Participant> &participant) {
	L_Q();
	L_Q_T(LocalConference, qConference);

	for (const auto &p : filteredParticipants) {
		if (participant->getAddress() == p->getAddress()) {
			filteredParticipants.remove(p);
			break;
		}
	}

	queuedMessages.erase(participant->getAddress().asString());

	// Do not notify participant removal for one-to-one chat rooms
	if (!(capabilities & ServerGroupChatRoom::Capabilities::OneToOne)) {
		shared_ptr<ConferenceParticipantEvent> event = qConference->getPrivate()->eventHandler->notifyParticipantRemoved(participant->getAddress());
		q->getCore()->getPrivate()->mainDb->addEvent(event);
	}
	if (!isAdminLeft())
		designateAdmin();

	// Cancel ongoing INVITEs, and start new ones with the correct participant list
	for (const auto &p : qConference->getPrivate()->participants) {
		for (const auto &device : p->getPrivate()->getDevices()) {
			if (getParticipantDeviceState(device) == ParticipantDevice::State::Joining)
				inviteDevice(device);
		}
	}
}

shared_ptr<Participant> ServerGroupChatRoomPrivate::findFilteredParticipant (const shared_ptr<const CallSession> &session) const {
	for (const auto &participant : filteredParticipants) {
		shared_ptr<ParticipantDevice> device = participant->getPrivate()->findDevice(session);
		if (device || (participant->getPrivate()->getSession() == session))
			return participant;
	}
	return nullptr;
}

shared_ptr<Participant> ServerGroupChatRoomPrivate::findFilteredParticipant (const IdentityAddress &participantAddress) const {
	IdentityAddress searchedAddr(participantAddress);
	searchedAddr.setGruu("");
	for (const auto &participant : filteredParticipants) {
		if (participant->getAddress() == searchedAddr)
			return participant;
	}
	return nullptr;
}

void ServerGroupChatRoomPrivate::subscribeReceived (LinphoneEvent *event) {
	L_Q_T(LocalConference, qConference);
	qConference->getPrivate()->eventHandler->subscribeReceived(event, !!(capabilities & ServerGroupChatRoom::Capabilities::OneToOne));
}

void ServerGroupChatRoomPrivate::subscriptionStateChanged (LinphoneEvent *event, LinphoneSubscriptionState state) {
	L_Q_T(LocalConference, qConference);
	qConference->getPrivate()->eventHandler->subscriptionStateChanged(event, state);
}

bool ServerGroupChatRoomPrivate::update (SalCallOp *op) {
	L_Q();
	if (sal_custom_header_find(op->getRecvCustomHeaders(), "Subject")) {
		// Handle subject change
		lInfo() << q << ": New subject \"" << op->getSubject() << "\"";
		q->setSubject(op->getSubject());
	}
	// Handle participants addition
	list<IdentityAddress> identAddresses = ServerGroupChatRoom::parseResourceLists(op->getRemoteBody());
	if (identAddresses.empty())
		return false;

	checkCompatibleParticipants(IdentityAddress(op->getRemoteContact()), identAddresses);
	return true;
}

// -----------------------------------------------------------------------------

LinphoneReason ServerGroupChatRoomPrivate::onSipMessageReceived (SalOp *op, const SalMessage *message) {
	L_Q();
	// Check that the message is coming from a participant of the chat room
	IdentityAddress fromAddr(op->getFrom());
	if (!findFilteredParticipant(fromAddr)) {
		return LinphoneReasonNotAcceptable;
	}

	// Do not check that we received a CPIM message because ciphered messages are not
	shared_ptr<Message> msg = make_shared<Message>(
		op->getFrom(),
		ContentType(message->content_type),
		message->text ? message->text : "",
		op->getRecvCustomHeaders()
	);

	// Handle reinvite of potentially missing participant in one-to-one chat room
	if (capabilities & ServerGroupChatRoom::Capabilities::OneToOne) {
		if (q->getParticipantCount() != 2) {
			IdentityAddress missingAddr = q->getCore()->getPrivate()->mainDb->findMissingOneToOneConferenceChatRoomParticipantAddress(
				q->getSharedFromThis(),
				q->getParticipants().front()->getAddress()
			);
			if (queuedMessages[missingAddr.asString()].size() == 0) {
				list<IdentityAddress> identAddresses;
				identAddresses.push_back(missingAddr);
				checkCompatibleParticipants(IdentityAddress(op->getFrom()), identAddresses);
			}
			queueMessage(msg, missingAddr);
			return LinphoneReasonNone;
		}
	}

	queueMessage(msg);
	dispatchQueuedMessages();
	return LinphoneReasonNone;
}

void ServerGroupChatRoomPrivate::setConferenceAddress (const IdentityAddress &conferenceAddress) {
	L_Q();
	L_Q_T(LocalConference, qConference);

	if (!conferenceAddress.isValid()) {
		shared_ptr<CallSession> session = q->getMe()->getPrivate()->getSession();
		session->decline(LinphoneReasonServerTimeout);
		setState(ChatRoom::State::CreationFailed);
		return;
	}

	if (q->getState() != ChatRoom::State::Instantiated) {
		lError() << "Cannot set the conference address of the ServerGroupChatRoom in state " << Utils::toString(q->getState());
		return;
	}
	qConference->getPrivate()->conferenceAddress = conferenceAddress;
	lInfo() << "The ServerGroupChatRoom has been given the address " << conferenceAddress.asString() << ", now finalizing its creation";
	finalizeCreation();
}

void ServerGroupChatRoomPrivate::setParticipantDevices(const IdentityAddress &participantAddress, const list<IdentityAddress> &devices) {
	L_Q();
	shared_ptr<Participant> participant = q->findParticipant(participantAddress);
	if (!participant)
		return;

	lInfo() << q << ": Setting " << devices.size() << " participant device(s) for " << participantAddress.asString();

	// Remove devices that are in the chatroom but no longer in the given list
	list<shared_ptr<ParticipantDevice>> devicesToRemove;
	for (const auto &device : participant->getPrivate()->getDevices()) {
		auto it = find(devices.cbegin(), devices.cend(), device->getAddress());
		if (it == devices.cend())
			devicesToRemove.push_back(device);
	}
	for (auto &device : devicesToRemove)
		removeParticipantDevice(participant, device->getAddress());

	// Add all the devices in the given list
	for (const auto &deviceAddress : devices)
		addParticipantDevice(participant, deviceAddress);

	LinphoneChatRoom *cr = L_GET_C_BACK_PTR(q);
	LinphoneAddress *laddr = linphone_address_new(participantAddress.asString().c_str());
	CALL_CHAT_ROOM_CBS(cr, ParticipantRegistrationSubscriptionRequested, participant_registration_subscription_requested, cr, laddr);
	linphone_address_unref(laddr);
}

void ServerGroupChatRoomPrivate::addCompatibleParticipants (const IdentityAddress &deviceAddr, const list<IdentityAddress> &compatibleParticipants) {
	L_Q();
	shared_ptr<Participant> participant = findFilteredParticipant(deviceAddr);
	if (!participant) {
		lError() << q << ": The device address that asked for compatible participants checking cannot be found in the filtered participants list";
		return;
	}
	shared_ptr<ParticipantDevice> device = participant->getPrivate()->findDevice(deviceAddr);
	shared_ptr<CallSession> session = device->getSession();
	if (compatibleParticipants.size() == 0) {
		lError() << q << ": No compatible participants have been found";
		if (session) {
			bool toDelete = joiningPendingAfterCreation;
			declineSession(session, LinphoneReasonNotAcceptable);
			if (toDelete)
				chatRoomListener->onChatRoomDeleteRequested(q->getSharedFromThis());
		}
	} else {
		lInfo() << q << ": Adding " << compatibleParticipants.size() << " compatible participant(s)";
		if (capabilities & ServerGroupChatRoom::Capabilities::OneToOne) {
			list<IdentityAddress> addressesToAdd(compatibleParticipants);
			addressesToAdd.sort();
			list<IdentityAddress> addresses;
			for (const auto &p : q->getParticipants()) {
				addresses.push_back(p->getAddress());
			}
			addresses.sort();
			addresses.merge(addressesToAdd);
			addresses.unique();
			if (session && (addresses.size() > 2)) {
				// Decline the participants addition to prevent having more than 2 participants in a one-to-one chat room.
				declineSession(session, LinphoneReasonNotAcceptable);
			}
		}
		if (session)
			acceptSession(session);

		lInfo() << q << ": Fetching participant devices";
		LinphoneChatRoom *cr = L_GET_C_BACK_PTR(q);
		LinphoneAddress *laddr = linphone_address_new(participant->getAddress().asString().c_str());
		CALL_CHAT_ROOM_CBS(cr, ParticipantDeviceFetchRequested, participant_device_fetch_requested, cr, laddr);
		linphone_address_unref(laddr);
		q->addParticipants(compatibleParticipants, nullptr, false);
		if ((capabilities & ServerGroupChatRoom::Capabilities::OneToOne) && (q->getParticipantCount() == 2)) {
			// Insert the one-to-one chat room in Db if participants count is 2.
			bool encrypted = ((capabilities & ServerGroupChatRoom::Capabilities::Encrypted) != 0);
			q->getCore()->getPrivate()->mainDb->insertOneToOneConferenceChatRoom(q->getSharedFromThis(), encrypted);
		}
	}
}

void ServerGroupChatRoomPrivate::checkCompatibleParticipants (const IdentityAddress &deviceAddr, const list<IdentityAddress> &addressesToCheck) {
	L_Q();
	list<Address> addresses;
	for (const auto &addr : addressesToCheck) {
		addresses.push_back(Address(addr));
	}

	lInfo() << q << ": Checking compatible participants";
	LinphoneChatRoom *cr = L_GET_C_BACK_PTR(q);
	bctbx_list_t * cAddresses = L_GET_RESOLVED_C_LIST_FROM_CPP_LIST(addresses);
	LinphoneAddress *cDeviceAddr = linphone_address_new(deviceAddr.asString().c_str());
	CALL_CHAT_ROOM_CBS(cr, ParticipantsCapabilitiesChecked, participants_capabilities_checked, cr, cDeviceAddr, cAddresses);
	linphone_address_unref(cDeviceAddr);
	bctbx_list_free_with_data(cAddresses, (bctbx_list_free_func)linphone_address_unref);
}

// -----------------------------------------------------------------------------

void ServerGroupChatRoomPrivate::copyMessageHeaders (const shared_ptr<Message> &fromMessage, const shared_ptr<ChatMessage> &toMessage) {
	string headersToCopy[] = {
		"Content-Encoding",
		"Expires",
		"Priority"
	};
	for (const auto &headerName : headersToCopy) {
		const char *headerValue = sal_custom_header_find(fromMessage->customHeaders, headerName.c_str());
		if (headerValue)
			toMessage->getPrivate()->addSalCustomHeader(headerName, headerValue);
	}
}

void ServerGroupChatRoomPrivate::addParticipantDevice (const shared_ptr<Participant> &participant, const IdentityAddress &deviceAddress) {
	L_Q();
	L_Q_T(LocalConference, qConference);
	shared_ptr<ParticipantDevice> device = participant->getPrivate()->findDevice(deviceAddress);
	if (device) {
		lInfo() << q << ": Adding participant device that is currently in state [" << device->getState() << "]";
		switch (device->getState()) {
			case ParticipantDevice::State::Joining:
				inviteDevice(device);
				break;
			case ParticipantDevice::State::Left:
				if (!findFilteredParticipant(participant->getAddress()))
					inviteDevice(device);
				break;
			case ParticipantDevice::State::Leaving:
				byeDevice(device);
				break;
			default:
				break;
		}
	} else if (findFilteredParticipant(participant->getAddress())) {
		// Add device only if participant is not currently being removed
		device = participant->getPrivate()->addDevice(deviceAddress);
		setParticipantDeviceState(device, ParticipantDevice::State::Joining);
		shared_ptr<ConferenceParticipantDeviceEvent> event = qConference->getPrivate()->eventHandler->notifyParticipantDeviceAdded(participant->getAddress(), deviceAddress);
		q->getCore()->getPrivate()->mainDb->addEvent(event);
		inviteDevice(device);
	}
}

void ServerGroupChatRoomPrivate::byeDevice (const std::shared_ptr<ParticipantDevice> &device) {
	L_Q();
	L_Q_T(LocalConference, qConference);

	shared_ptr<CallSession> session = device->getSession();
	if (session && (session->getState() != CallSession::State::Released)) {
		session->terminate();
		return;
	}
	// If 481 is received when sending the BYE, perform a new INVITE to send the BYE correctly
	if (!session || (session->getReason() == LinphoneReasonNoMatch)) {
		CallSessionParams csp;
		auto participant = device->getParticipant();
		session = participant->getPrivate()->createSession(*q, &csp, false, this);
		session->configure(LinphoneCallOutgoing, nullptr, nullptr, qConference->getPrivate()->conferenceAddress, device->getAddress());
		device->setSession(session);
		session->initiateOutgoing();
		session->getPrivate()->createOp();
		Address contactAddr(qConference->getPrivate()->conferenceAddress);
		contactAddr.setParam("isfocus");
		contactAddr.setParam("text");
		session->getPrivate()->getOp()->setContactAddress(contactAddr.getPrivate()->getInternalAddress());
		session->startInvite(nullptr, q->getSubject(), nullptr);
	}
}

void ServerGroupChatRoomPrivate::designateAdmin () {
	L_Q();
	// Do not designate new admin for one-to-one chat room
	if (!(capabilities & ServerGroupChatRoom::Capabilities::OneToOne) && !filteredParticipants.empty()) {
		q->setParticipantAdminStatus(filteredParticipants.front(), true);
		lInfo() << q << ": New admin designated";
	}
}

void ServerGroupChatRoomPrivate::dispatchMessage (const shared_ptr<Message> &message, const string &uri) {
	L_Q();
	IdentityAddress deviceAddr(uri);
	for (const auto &p : q->getParticipants()) {
		shared_ptr<ParticipantDevice> device = p->getPrivate()->findDevice(deviceAddr);
		if (device) {
			shared_ptr<ChatMessage> msg = q->createChatMessage();
			copyMessageHeaders(message, msg);
			msg->setInternalContent(message->content);
			msg->getPrivate()->forceFromAddress(q->getConferenceAddress());
			msg->getPrivate()->forceToAddress(device->getAddress());
			msg->getPrivate()->setApplyModifiers(false);
			msg->send();
			return;
		}
	}
}

void ServerGroupChatRoomPrivate::finalizeCreation () {
	L_Q();
	L_Q_T(LocalConference, qConference);
	IdentityAddress confAddr(qConference->getPrivate()->conferenceAddress);
	conferenceId = ConferenceId(confAddr, confAddr);
	qConference->getPrivate()->eventHandler->setConferenceId(conferenceId);
	lInfo() << q << " created";
	// Let the SIP stack set the domain and the port
	shared_ptr<Participant> me = q->getMe();
	me->getPrivate()->setAddress(confAddr);
	Address addr(confAddr);
	addr.setParam("isfocus");
	shared_ptr<CallSession> session = me->getPrivate()->getSession();
	session->redirect(addr);
	joiningPendingAfterCreation = true;
	chatRoomListener->onChatRoomInsertRequested(q->getSharedFromThis());
	setState(ChatRoom::State::Created);
	chatRoomListener->onChatRoomInsertInDatabaseRequested(q->getSharedFromThis());
}

void ServerGroupChatRoomPrivate::inviteDevice (const shared_ptr<ParticipantDevice> &device) {
	L_Q();
	L_Q_T(LocalConference, qConference);
	lInfo() << q << ": Inviting device '" << device->getAddress().asString() << "'";
	shared_ptr<Participant> participant = const_pointer_cast<Participant>(device->getParticipant()->getSharedFromThis());
	shared_ptr<CallSession> session = device->getSession();
	if (session && (session->getDirection() == LinphoneCallIncoming))
		return; // Do not try to invite the device that is currently creating the chat room
	device->setState(ParticipantDevice::State::Joining);
	if (!session
		|| (session->getState() == CallSession::State::End)
		|| (session->getState() == CallSession::State::Error)
		|| (session->getState() == CallSession::State::Released)
	) {
		CallSessionParams csp;
		if (capabilities & ServerGroupChatRoom::Capabilities::OneToOne)
			csp.addCustomHeader("One-To-One-Chat-Room", "true");
		if (capabilities & ServerGroupChatRoom::Capabilities::Encrypted)
			csp.addCustomHeader("End-To-End-Encrypted", "true");
		session = participant->getPrivate()->createSession(*q, &csp, false, this);
		session->configure(LinphoneCallOutgoing, nullptr, nullptr, qConference->getPrivate()->conferenceAddress, device->getAddress());
		device->setSession(session);
		session->initiateOutgoing();
		session->getPrivate()->createOp();
	}

	Address contactAddr(qConference->getPrivate()->conferenceAddress);
	contactAddr.setParam("isfocus");
	contactAddr.setParam("text");
	session->getPrivate()->getOp()->setContactAddress(contactAddr.getPrivate()->getInternalAddress());

	list<IdentityAddress> addressesList;
	for (const auto &invitedParticipant : filteredParticipants) {
		if (invitedParticipant != participant)
			addressesList.push_back(invitedParticipant->getAddress());
	}
	Content content;
	content.setBody(q->getResourceLists(addressesList));
	content.setContentType(ContentType::ResourceLists);
	content.setContentDisposition(ContentDisposition::RecipientListHistory);
	if (linphone_core_content_encoding_supported(q->getCore()->getCCore(), "deflate"))
		content.setContentEncoding("deflate");
	session->startInvite(nullptr, q->getSubject(), &content);
}

bool ServerGroupChatRoomPrivate::isAdminLeft () const {
	for (const auto &participant : filteredParticipants) {
		if (participant->isAdmin())
			return true;
	}
	return false;
}

void ServerGroupChatRoomPrivate::queueMessage (const shared_ptr<Message> &msg) {
	L_Q();
	for (const auto &participant : q->getParticipants()) {
		for (const auto &device : participant->getPrivate()->getDevices()) {
			queueMessage(msg, device->getAddress());
		}
	}
}

void ServerGroupChatRoomPrivate::queueMessage (const shared_ptr<Message> &msg, const IdentityAddress &deviceAddress) {
	chrono::system_clock::time_point timestamp = chrono::system_clock::now();
	string uri(deviceAddress.asString());
	// Remove queued messages older than one week
	while (!queuedMessages[uri].empty()) {
		shared_ptr<Message> m = queuedMessages[uri].front();
		chrono::hours age = chrono::duration_cast<chrono::hours>(timestamp - m->timestamp);
		chrono::hours oneWeek(168);
		if (age < oneWeek)
			break;
		queuedMessages[uri].pop();
	}
	// Queue the message for all devices except the one that sent it
	if (msg->fromAddr != deviceAddress)
		queuedMessages[uri].push(msg);
}

void ServerGroupChatRoomPrivate::removeParticipantDevice (const shared_ptr<Participant> &participant, const IdentityAddress &deviceAddress) {
	L_Q();
	L_Q_T(LocalConference, qConference);
	participant->getPrivate()->removeDevice(deviceAddress);
	auto deviceEvent = qConference->getPrivate()->eventHandler->notifyParticipantDeviceRemoved(participant->getAddress(), deviceAddress);
	q->getCore()->getPrivate()->mainDb->addEvent(deviceEvent);
}

// -----------------------------------------------------------------------------

void ServerGroupChatRoomPrivate::onParticipantDeviceLeft (const std::shared_ptr<ParticipantDevice> &device) {
	L_Q();
	L_Q_T(LocalConference, qConference);

	lInfo() << q << ": Participant device '" << device->getAddress().asString() << "' left";
	shared_ptr<Participant> participant = const_pointer_cast<Participant>(device->getParticipant()->getSharedFromThis());
	setParticipantDeviceState(device, ParticipantDevice::State::Left);

	bool allDevicesLeft = true;
	for (const auto &device : participant->getPrivate()->getDevices()) {
		if (getParticipantDeviceState(device) != ParticipantDevice::State::Left) {
			allDevicesLeft = false;
			break;
		}
	}
	if (allDevicesLeft) {
		lInfo() << q << ": Removing participant '" << participant->getAddress().asString() << "' since it has no device left";

		LinphoneChatRoom *cr = L_GET_C_BACK_PTR(q);
		LinphoneAddress *laddr = linphone_address_new(participant->getAddress().asString().c_str());
		CALL_CHAT_ROOM_CBS(cr, ParticipantRegistrationUnsubscriptionRequested, participant_registration_unsubscription_requested, cr, laddr);
		linphone_address_unref(laddr);

		qConference->getPrivate()->participants.remove(participant);
		filteredParticipants.remove(participant);
	}

	if (qConference->getPrivate()->participants.size() == 0) {
		lInfo() << q << ": No participant left, deleting the chat room";
		chatRoomListener->onChatRoomDeleteRequested(q->getSharedFromThis());
	}
}

// -----------------------------------------------------------------------------

void ServerGroupChatRoomPrivate::onChatRoomInsertRequested (const shared_ptr<AbstractChatRoom> &chatRoom) {
	L_Q();
	q->getCore()->getPrivate()->insertChatRoom(chatRoom);
}

void ServerGroupChatRoomPrivate::onChatRoomInsertInDatabaseRequested (const shared_ptr<AbstractChatRoom> &chatRoom) {
	L_Q();
	q->getCore()->getPrivate()->insertChatRoomWithDb(chatRoom);
}

void ServerGroupChatRoomPrivate::onChatRoomDeleteRequested (const shared_ptr<AbstractChatRoom> &chatRoom) {
	L_Q();
	q->deleteFromDb();
}

// -----------------------------------------------------------------------------

void ServerGroupChatRoomPrivate::onCallSessionStateChanged (const shared_ptr<CallSession> &session, CallSession::State newState, const string &message) {
	L_Q();
	if ((newState == CallSession::State::End) && (session->getPreviousState() == CallSession::State::Connected)) {
		auto device = q->findParticipantDevice(session);
		if (device && (device->getState() == ParticipantDevice::State::Present)) {
			// Participant leaves the chat room on its own by sending a BYE
			setParticipantDeviceState(device, ParticipantDevice::State::Leaving);
			q->removeParticipant(device->getParticipant()->getSharedFromThis());
		}
	} else if (newState == CallSession::State::Released) {
		if (session->getReason() == LinphoneReasonNone) {
			auto device = q->findParticipantDevice(session);
			if (device && (device->getState() == ParticipantDevice::State::Leaving))
				onParticipantDeviceLeft(device);
		} else if (session->getReason() == LinphoneReasonNoMatch) {
			auto device = q->findParticipantDevice(session);
			if (device) {
				if (device->getState() == ParticipantDevice::State::Joining)
					inviteDevice(device);
				else if (device->getState() == ParticipantDevice::State::Leaving)
					byeDevice(device);
			}
		}
	} else if (newState == CallSession::State::UpdatedByRemote) {
		shared_ptr<Participant> participant = findFilteredParticipant(session);
		if (participant && participant->isAdmin()) {
			bool res = update(session->getPrivate()->getOp());
			if (res)
				session->deferUpdate();
		}
	} else if (newState == CallSession::State::Connected) {
		auto device = q->findParticipantDevice(session);
		if (device && (device->getState() == ParticipantDevice::State::Leaving))
			device->getSession()->terminate();
	}
}

void ServerGroupChatRoomPrivate::onCallSessionSetReleased (const shared_ptr<CallSession> &session) {
	L_Q();
	shared_ptr<ParticipantDevice> device = q->findParticipantDevice(session);
	if (device)
		device->setSession(nullptr);
}

// =============================================================================

ServerGroupChatRoom::ServerGroupChatRoom (const shared_ptr<Core> &core, SalCallOp *op)
: ChatRoom(*new ServerGroupChatRoomPrivate, core, ConferenceId()),
LocalConference(getCore(), IdentityAddress(linphone_proxy_config_get_conference_factory_uri(linphone_core_get_default_proxy_config(core->getCCore()))), nullptr) {
	L_D();
	L_D_T(LocalConference, dConference);

	LocalConference::setSubject(op->getSubject());
	const char *oneToOneChatRoomStr = sal_custom_header_find(op->getRecvCustomHeaders(), "One-To-One-Chat-Room");
	if (oneToOneChatRoomStr && (strcmp(oneToOneChatRoomStr, "true") == 0))
		d->capabilities |= ServerGroupChatRoom::Capabilities::OneToOne;
	const char *endToEndEncryptedStr = sal_custom_header_find(op->getRecvCustomHeaders(), "End-To-End-Encrypted");
	if (endToEndEncryptedStr && (strcmp(endToEndEncryptedStr, "true") == 0))
		d->capabilities |= ServerGroupChatRoom::Capabilities::Encrypted;
	shared_ptr<CallSession> session = getMe()->getPrivate()->createSession(*this, nullptr, false, d);
	session->configure(LinphoneCallIncoming, nullptr, op, Address(op->getFrom()), Address(op->getTo()));
	getCore()->getPrivate()->localListEventHandler->addHandler(dConference->eventHandler.get());
}

ServerGroupChatRoom::ServerGroupChatRoom (
	const shared_ptr<Core> &core,
	const IdentityAddress &peerAddress,
	AbstractChatRoom::CapabilitiesMask capabilities,
	const string &subject,
	list<shared_ptr<Participant>> &&participants,
	unsigned int lastNotifyId
) : ChatRoom(*new ServerGroupChatRoomPrivate, core, ConferenceId(peerAddress, peerAddress)),
LocalConference(getCore(), peerAddress, nullptr) {
	L_D();
	L_D_T(LocalConference, dConference);

	d->capabilities |= capabilities & ServerGroupChatRoom::Capabilities::OneToOne;
	dConference->subject = subject;
	dConference->participants = move(participants);
	dConference->conferenceAddress = peerAddress;
	dConference->eventHandler->setLastNotify(lastNotifyId);
	dConference->eventHandler->setConferenceId(d->conferenceId);
	getCore()->getPrivate()->localListEventHandler->addHandler(dConference->eventHandler.get());
}

ServerGroupChatRoom::~ServerGroupChatRoom () {
	L_D_T(LocalConference, dConference);

	try {
		if (getCore()->getPrivate()->localListEventHandler)
			getCore()->getPrivate()->localListEventHandler->removeHandler(dConference->eventHandler.get());
	} catch (const bad_weak_ptr &) {
		// Unable to unregister listener here. Core is destroyed and the listener doesn't exist.
	}
};

shared_ptr<Core> ServerGroupChatRoom::getCore () const {
	return ChatRoom::getCore();
}

shared_ptr<Participant> ServerGroupChatRoom::findParticipant (const shared_ptr<const CallSession> &session) const {
	return LocalConference::findParticipant(session);
}

ServerGroupChatRoom::CapabilitiesMask ServerGroupChatRoom::getCapabilities () const {
	L_D();
	return d->capabilities;
}

void ServerGroupChatRoom::allowCpim (bool value) {}

void ServerGroupChatRoom::allowMultipart (bool value) {}


bool ServerGroupChatRoom::canHandleCpim () const {
	return true;
}

bool ServerGroupChatRoom::canHandleMultipart () const {
	return true;
}

bool ServerGroupChatRoom::hasBeenLeft () const {
	return false;
}

// -----------------------------------------------------------------------------

bool ServerGroupChatRoom::addParticipant (const IdentityAddress &addr, const CallSessionParams *params, bool hasMedia) {
	L_D();
	L_D_T(LocalConference, dConference);
	if (d->findFilteredParticipant(addr)) {
		lInfo() << this << ": Not adding participant '" << addr.asString() << "' because it is already a participant";
		return false;
	}

	if ((d->capabilities & ServerGroupChatRoom::Capabilities::OneToOne) && (getParticipantCount() == 2)) {
		lInfo() << this << ": Not adding participant '" << addr.asString() << "' because this OneToOne chat room already has 2 participants";
		return false;
	}

	lInfo() << this << ": Adding participant '" << addr.asString() << "'";
	if (!findParticipant(addr))
		LocalConference::addParticipant(addr, params, hasMedia);
	d->filteredParticipants.push_back(findParticipant(addr));
	shared_ptr<ConferenceParticipantEvent> event = dConference->eventHandler->notifyParticipantAdded(addr);
	getCore()->getPrivate()->mainDb->addEvent(event);

	LinphoneChatRoom *cr = L_GET_C_BACK_PTR(this);
	LinphoneAddress *laddr = linphone_address_new(addr.asString().c_str());
	CALL_CHAT_ROOM_CBS(cr, ParticipantDeviceFetchRequested, participant_device_fetch_requested, cr, laddr);
	linphone_address_unref(laddr);

	return true;
}

bool ServerGroupChatRoom::addParticipants (const list<IdentityAddress> &addresses, const CallSessionParams *params, bool hasMedia) {
	return LocalConference::addParticipants(addresses, params, hasMedia);
}

bool ServerGroupChatRoom::canHandleParticipants () const {
	return LocalConference::canHandleParticipants();
}

shared_ptr<Participant> ServerGroupChatRoom::findParticipant (const IdentityAddress &participantAddress) const {
	return LocalConference::findParticipant(participantAddress);
}

const IdentityAddress &ServerGroupChatRoom::getConferenceAddress () const {
	return LocalConference::getConferenceAddress();
}

shared_ptr<Participant> ServerGroupChatRoom::getMe () const {
	return LocalConference::getMe();
}

int ServerGroupChatRoom::getParticipantCount () const {
	return LocalConference::getParticipantCount();
}

const list<shared_ptr<Participant>> &ServerGroupChatRoom::getParticipants () const {
	L_D();
	return d->filteredParticipants;
}

const string &ServerGroupChatRoom::getSubject () const {
	return LocalConference::getSubject();
}

void ServerGroupChatRoom::join () {}

void ServerGroupChatRoom::leave () {}

void ServerGroupChatRoom::onFirstNotifyReceived (const IdentityAddress &addr) {
	L_D();
	for (const auto &participant : getParticipants()) {
		for (const auto &device : participant->getPrivate()->getDevices()) {
			if (device->getAddress() == addr) {
				d->setParticipantDeviceState(device, ParticipantDevice::State::Present);
				d->dispatchQueuedMessages();
				return;
			}
		}
	}
}

bool ServerGroupChatRoom::removeParticipant (const shared_ptr<Participant> &participant) {
	L_D();
	for (const auto &device : participant->getPrivate()->getDevices()) {
		if ((d->getParticipantDeviceState(device) == ParticipantDevice::State::Leaving)
			|| (d->getParticipantDeviceState(device) == ParticipantDevice::State::Left)
		)
			continue;
		d->setParticipantDeviceState(device, ParticipantDevice::State::Leaving);
		lInfo() << this << ": Asking device '" << device->getAddress().asString() << "' to leave";
		d->byeDevice(device);
	}
	d->removeParticipant(participant);
	return true;
}

bool ServerGroupChatRoom::removeParticipants (const list<shared_ptr<Participant>> &participants) {
	return LocalConference::removeParticipants(participants);
}

void ServerGroupChatRoom::setParticipantAdminStatus (const shared_ptr<Participant> &participant, bool isAdmin) {
	L_D();
	L_D_T(LocalConference, dConference);
	if (isAdmin != participant->isAdmin()) {
		participant->getPrivate()->setAdmin(isAdmin);
		if (!(d->capabilities & ServerGroupChatRoom::Capabilities::OneToOne)) {
			shared_ptr<ConferenceParticipantEvent> event = dConference->eventHandler->notifyParticipantSetAdmin(participant->getAddress(), participant->isAdmin());
			getCore()->getPrivate()->mainDb->addEvent(event);
		}
	}
}

void ServerGroupChatRoom::setSubject (const string &subject) {
	L_D_T(LocalConference, dConference);
	if (subject != getSubject()) {
		LocalConference::setSubject(subject);
		shared_ptr<ConferenceSubjectEvent> event = dConference->eventHandler->notifySubjectChanged();
		getCore()->getPrivate()->mainDb->addEvent(event);
	}
}

// -----------------------------------------------------------------------------

ostream &operator<< (ostream &stream, const ServerGroupChatRoom *chatRoom) {
	return stream << "ServerGroupChatRoom [" << chatRoom->getConferenceId().getPeerAddress().asString() << "]";
}

LINPHONE_END_NAMESPACE
