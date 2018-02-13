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
#include "conference/local-conference-p.h"
#include "conference/participant-p.h"
#include "conference/session/call-session-p.h"
#include "content/content-type.h"
#include "core/core-p.h"
#include "event-log/events.h"
#include "logger/logger.h"
#include "sal/refer-op.h"
#include "server-group-chat-room-p.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

shared_ptr<Participant> ServerGroupChatRoomPrivate::addParticipant (const IdentityAddress &addr) {
	L_Q();
	L_Q_T(LocalConference, qConference);
	shared_ptr<Participant> participant;
	participant = q->findParticipant(addr);
	if (!participant) {
		participant = make_shared<Participant>(addr);
		qConference->getPrivate()->participants.push_back(participant);
		shared_ptr<ConferenceParticipantEvent> event = qConference->getPrivate()->eventHandler->notifyParticipantAdded(addr);
		q->getCore()->getPrivate()->mainDb->addEvent(event);
	}
	filteredParticipants.push_back(participant);
	return participant;
}

ParticipantDevice::State ServerGroupChatRoomPrivate::getParticipantDeviceState (const shared_ptr<const ParticipantDevice> &device) const {
	return device->getState();
}

void ServerGroupChatRoomPrivate::setParticipantDeviceState (const shared_ptr<ParticipantDevice> &device, ParticipantDevice::State state) {
	L_Q();
	device->setState(state);
	q->getCore()->getPrivate()->mainDb->updateChatRoomParticipantDevice(q->getSharedFromThis(), device);
}

void ServerGroupChatRoomPrivate::acceptSession (const shared_ptr<CallSession> &session) {
	if (session->getState() == CallSession::State::UpdatedByRemote)
		session->acceptUpdate();
	else
		session->accept();
}

void ServerGroupChatRoomPrivate::confirmCreation () {
	L_Q();
	L_Q_T(LocalConference, qConference);

	shared_ptr<Participant> me = q->getMe();
	shared_ptr<CallSession> session = me->getPrivate()->getSession();
	session->startIncomingNotification();

	LinphoneChatRoom *cr = L_GET_C_BACK_PTR(q);
	LinphoneChatRoomCbs *cbs = linphone_chat_room_get_callbacks(cr);
	LinphoneChatRoomCbsConferenceAddressGenerationCb cb = linphone_chat_room_cbs_get_conference_address_generation(cbs);
	if (cb)
		cb(cr);
	else {
		IdentityAddress confAddr(generateConferenceAddress(me));
		qConference->getPrivate()->conferenceAddress = confAddr;
		finalizeCreation();
	}
}

void ServerGroupChatRoomPrivate::confirmJoining (SalCallOp *op) {
	L_Q();
	L_Q_T(LocalConference, qConference);
	shared_ptr<Participant> participant;

	Address contactAddr(op->get_remote_contact());
	if (contactAddr.getUriParamValue("gr").empty()) {
		lError() << "Declining INVITE for ServerGroupChatRoom [" << q
			<< "] because the contact does not have a 'gr' uri parameter [" << contactAddr.asString() << "]";
		op->decline(SalReasonDeclined, nullptr);
		joiningPendingAfterCreation = false;
		return;
	}

	IdentityAddress gruu(contactAddr);
	shared_ptr<ParticipantDevice> device;
	shared_ptr<CallSession> session;
	if (joiningPendingAfterCreation) {
		participant = addParticipant(IdentityAddress(op->get_from()));
		participant->getPrivate()->setAdmin(true);
		device = participant->getPrivate()->addDevice(gruu);
		session = device->getSession();
		shared_ptr<ConferenceParticipantDeviceEvent> deviceEvent = qConference->getPrivate()->eventHandler->notifyParticipantDeviceAdded(participant->getAddress(), gruu);
		shared_ptr<ConferenceParticipantEvent> adminEvent = qConference->getPrivate()->eventHandler->notifyParticipantSetAdmin(participant->getAddress(), true);
		q->getCore()->getPrivate()->mainDb->addEvent(deviceEvent);
		q->getCore()->getPrivate()->mainDb->addEvent(adminEvent);
	} else {
		// INVITE coming from an invited participant
		participant = q->findParticipant(IdentityAddress(op->get_from()));
		if (!participant) {
			lError() << "Declining INVITE coming from someone that does not participate in ServerGroupChatRoom [" << q << "]";
			op->decline(SalReasonDeclined, nullptr);
			joiningPendingAfterCreation = false;
			return;
		}
		device = participant->getPrivate()->addDevice(gruu);
		session = device->getSession();
	}

	if (!session || (session->getPrivate()->getOp() != op)) {
		session = participant->getPrivate()->createSession(*q, nullptr, false, this);
		session->configure(LinphoneCallIncoming, nullptr, op, participant->getAddress(), Address(op->get_to()));
		session->startIncomingNotification();
		Address addr = qConference->getPrivate()->conferenceAddress;
		addr.setParam("isfocus");
		session->getPrivate()->getOp()->set_contact_address(addr.getPrivate()->getInternalAddress());
		device->setSession(session);
	}

	// Changes are only allowed from admin participants
	if (participant->isAdmin()) {
		bool res = update(op);
		if (!res) {
			if (joiningPendingAfterCreation) {
				lError() << "Declining INVITE because we expected a non-empty list of participants to invite in ServerGroupChatRoom [" << q << "], but it was empty";
				op->decline(SalReasonNotAcceptable, nullptr);
			} else {
				acceptSession(session);
			}
			joiningPendingAfterCreation = false;
			return;
		}
	} else {
		acceptSession(session);
	}

	joiningPendingAfterCreation = false;
}

void ServerGroupChatRoomPrivate::confirmRecreation (SalCallOp *op) {
	L_Q();
	L_Q_T(LocalConference, qConference);
	IdentityAddress confAddr(qConference->getPrivate()->conferenceAddress);
	Address addr(confAddr);
	addr.setParam("isfocus");
	shared_ptr<Participant> me = q->getMe();
	shared_ptr<CallSession> session = me->getPrivate()->createSession(*q, nullptr, false, this);
	session->configure(LinphoneCallIncoming, nullptr, op, Address(op->get_from()), Address(op->get_to()));
	session->startIncomingNotification();
	session->redirect(addr);
	joiningPendingAfterCreation = true;
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
			while (!queuedMessages[uri].empty()) {
				shared_ptr<Message> msg = queuedMessages[uri].front();
				dispatchMessage(msg, uri);
				queuedMessages[uri].pop();
			}
		}
	}
}

IdentityAddress ServerGroupChatRoomPrivate::generateConferenceAddress (const shared_ptr<Participant> &me) const {
	L_Q();
	char token[11];
	ostringstream os;
	IdentityAddress conferenceAddress = me->getAddress();
	do {
		belle_sip_random_token(token, sizeof(token));
		os.str("");
		os << "chatroom-" << token;
		conferenceAddress.setUsername(os.str());
	} while (q->getCore()->findChatRoom(chatRoomId));
	me->getPrivate()->setAddress(conferenceAddress);
	return me->getAddress();
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

	// Do not notify participant removal for one-to-one chat rooms
	if (!(capabilities & ServerGroupChatRoom::Capabilities::OneToOne)) {
		shared_ptr<ConferenceParticipantEvent> event = qConference->getPrivate()->eventHandler->notifyParticipantRemoved(participant->getAddress());
		q->getCore()->getPrivate()->mainDb->addEvent(event);
	}
	if (!isAdminLeft())
		designateAdmin();
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
	qConference->getPrivate()->eventHandler->subscribeReceived(event, capabilities & ServerGroupChatRoom::Capabilities::OneToOne);
}

bool ServerGroupChatRoomPrivate::update (SalCallOp *op) {
	L_Q();
	if (sal_custom_header_find(op->get_recv_custom_header(), "Subject")) {
		// Handle subject change
		q->setSubject(L_C_TO_STRING(op->get_subject()));
	}
	// Handle participants addition
	list<IdentityAddress> identAddresses = ServerGroupChatRoom::parseResourceLists(op->get_remote_body());
	if (identAddresses.empty())
		return false;

	checkCompatibleParticipants(IdentityAddress(op->get_remote_contact()), identAddresses);
	return true;
}

// -----------------------------------------------------------------------------

LinphoneReason ServerGroupChatRoomPrivate::onSipMessageReceived (SalOp *op, const SalMessage *message) {
	L_Q();
	// Check that the message is coming from a participant of the chat room
	IdentityAddress fromAddr(op->get_from());
	if (!findFilteredParticipant(fromAddr)) {
		return LinphoneReasonNotAcceptable;
	}

	// Do not check that we received a CPIM message because ciphered messages are not
	shared_ptr<Message> msg = make_shared<Message>(op->get_from(), message->content_type, message->text ? message->text : "");

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
				checkCompatibleParticipants(IdentityAddress(op->get_from()), identAddresses);
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
	if (q->getState() != ChatRoom::State::Instantiated) {
		lError() << "Cannot set the conference address of the ServerGroupChatRoom in state " << Utils::toString(q->getState());
		return;
	}
	qConference->getPrivate()->conferenceAddress = conferenceAddress;
	lInfo() << "The ServerGroupChatRoom has been given the address " << conferenceAddress.asString() << ", now finalizing its creation";
	finalizeCreation();
}

void ServerGroupChatRoomPrivate::setParticipantDevices(const IdentityAddress &addr, const list<IdentityAddress> &devices) {
	L_Q();
	L_Q_T(LocalConference, qConference);
	shared_ptr<Participant> participant = findFilteredParticipant(addr);
	if (!participant)
		return;
	lInfo() << "Setting " << devices.size() << " participant devices for " << addr.asString() << " in ServerGroupChatRoom [" << q << "]";
	for (const auto &deviceAddr : devices) {
		if (participant->getPrivate()->findDevice(deviceAddr))
			continue;
		shared_ptr<ParticipantDevice> device = participant->getPrivate()->addDevice(deviceAddr);
		shared_ptr<ConferenceParticipantDeviceEvent> event = qConference->getPrivate()->eventHandler->notifyParticipantDeviceAdded(addr, deviceAddr);
		q->getCore()->getPrivate()->mainDb->addEvent(event);
		inviteDevice(device);
	}
}

void ServerGroupChatRoomPrivate::addCompatibleParticipants (const IdentityAddress &deviceAddr, const list<IdentityAddress> &compatibleParticipants) {
	L_Q();
	shared_ptr<Participant> participant = findFilteredParticipant(deviceAddr);
	if (!participant)
		return;
	shared_ptr<ParticipantDevice> device = participant->getPrivate()->findDevice(deviceAddr);
	if (compatibleParticipants.size() == 0) {
		lError() << "Declining INVITE for ServerGroupChatRoom [" << q << "] because no compatible participants have been found";
		device->getSession()->decline(LinphoneReasonNotAcceptable);
	} else {
		lInfo() << "Adding " << compatibleParticipants.size() << " compatible participants to ServerGroupChatRoom [" << q << "]";
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
			if (addresses.size() > 2) {
				// Decline the participants addition to prevent having more than 2 participants in a one-to-one chat room.
				device->getSession()->decline(LinphoneReasonNotAcceptable);
			}
		}
		acceptSession(device->getSession());
		lInfo() << "Fetching participant devices for ServerGroupChatRoom [" << q << "]";
		LinphoneChatRoom *cr = L_GET_C_BACK_PTR(q);
		LinphoneChatRoomCbs *cbs = linphone_chat_room_get_callbacks(cr);
		LinphoneChatRoomCbsParticipantDeviceFetchedCb cb = linphone_chat_room_cbs_get_participant_device_fetched(cbs);
		if (cb) {
			LinphoneAddress *laddr = linphone_address_new(participant->getAddress().asString().c_str());
			cb(cr, laddr);
			linphone_address_unref(laddr);
		}
		q->addParticipants(compatibleParticipants, nullptr, false);
		if ((capabilities & ServerGroupChatRoom::Capabilities::OneToOne) && (q->getParticipantCount() == 2)) {
			// Insert the one-to-one chat room in Db if participants count is 2.
			q->getCore()->getPrivate()->mainDb->insertOneToOneConferenceChatRoom(q->getSharedFromThis());
		}
	}
}

void ServerGroupChatRoomPrivate::checkCompatibleParticipants (const IdentityAddress &deviceAddr, const list<IdentityAddress> &addressesToCheck) {
	L_Q();
	list<Address> addresses;
	for (const auto &addr : addressesToCheck) {
		addresses.push_back(Address(addr));
	}

	lInfo() << "Checking compatible participants for ServerGroupChatRoom [" << q << "]";
	bctbx_list_t * cAddresses = L_GET_RESOLVED_C_LIST_FROM_CPP_LIST(addresses);
	LinphoneChatRoom *cr = L_GET_C_BACK_PTR(q);
	LinphoneChatRoomCbs *cbs = linphone_chat_room_get_callbacks(cr);
	LinphoneChatRoomCbsParticipantsCapabilitiesCheckedCb cb = linphone_chat_room_cbs_get_participants_capabilities_checked(cbs);
	if (cb) {
		LinphoneAddress *cDeviceAddr = linphone_address_new(deviceAddr.asString().c_str());
		cb(cr, cDeviceAddr, cAddresses);
		linphone_address_unref(cDeviceAddr);
	}
}

// -----------------------------------------------------------------------------

void ServerGroupChatRoomPrivate::designateAdmin () {
	L_Q();
	// Do not designate new admin for one-to-one chat room
	if (!(capabilities & ServerGroupChatRoom::Capabilities::OneToOne) && !filteredParticipants.empty()) {
		q->setParticipantAdminStatus(filteredParticipants.front(), true);
		lInfo() << "New admin designated in ServerGroupChatRoom [" << q << "]";
	}
}

void ServerGroupChatRoomPrivate::dispatchMessage (const shared_ptr<Message> &message, const string &uri) {
	L_Q();
	IdentityAddress deviceAddr(uri);
	for (const auto &p : q->getParticipants()) {
		shared_ptr<ParticipantDevice> device = p->getPrivate()->findDevice(deviceAddr);
		if (device) {
			shared_ptr<ChatMessage> msg = q->createChatMessage();
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
	chatRoomId = ChatRoomId(confAddr, confAddr);
	qConference->getPrivate()->eventHandler->setChatRoomId(chatRoomId);
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
	SalReferOp *referOp = new SalReferOp(q->getCore()->getCCore()->sal);
	LinphoneAddress *lAddr = linphone_address_new(device->getAddress().asString().c_str());
	linphone_configure_op(q->getCore()->getCCore(), referOp, lAddr, nullptr, false);
	linphone_address_unref(lAddr);
	Address referToAddr = q->getConferenceAddress();
	referToAddr.setParam("text");
	referOp->send_refer(referToAddr.getPrivate()->getInternalAddress());
	referOp->unref();
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
	std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
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

// -----------------------------------------------------------------------------

void ServerGroupChatRoomPrivate::onParticipantDeviceLeft (const std::shared_ptr<const CallSession> &session) {
	L_Q();
	L_Q_T(LocalConference, qConference);

	shared_ptr<Participant> participant = q->findParticipant(session);
	if (!participant)
		return;
	shared_ptr<ParticipantDevice> device = participant->getPrivate()->findDevice(session);
	if (!device)
		return;

	if (getParticipantDeviceState(device) == ParticipantDevice::State::Present) {
		setParticipantDeviceState(device, ParticipantDevice::State::Left);
		q->removeParticipant(participant);
	} else {
		setParticipantDeviceState(device, ParticipantDevice::State::Left);
	}

	bool allDevicesLeft = true;
	for (const auto &device : participant->getPrivate()->getDevices()) {
		if (getParticipantDeviceState(device) != ParticipantDevice::State::Left) {
			allDevicesLeft = false;
			break;
		}
	}
	if (allDevicesLeft) {
		qConference->getPrivate()->participants.remove(participant);
		filteredParticipants.remove(participant);
	}

	if (q->getParticipantCount() == 0)
		chatRoomListener->onChatRoomDeleteRequested(q->getSharedFromThis());
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
	if (newState == CallSession::State::End) {
		onParticipantDeviceLeft(session);
	} else if (newState == CallSession::State::UpdatedByRemote) {
		shared_ptr<Participant> participant = findFilteredParticipant(session);
		if (participant && participant->isAdmin()) {
			bool res = update(session->getPrivate()->getOp());
			if (res)
				session->deferUpdate();
		}
	}
}

// =============================================================================

ServerGroupChatRoom::ServerGroupChatRoom (const shared_ptr<Core> &core, SalCallOp *op)
: ChatRoom(*new ServerGroupChatRoomPrivate, core, ChatRoomId()),
LocalConference(getCore(), IdentityAddress(linphone_core_get_conference_factory_uri(core->getCCore())), nullptr) {
	L_D();
	LocalConference::setSubject(op->get_subject() ? op->get_subject() : "");
	const char *oneToOneChatRoomStr = sal_custom_header_find(op->get_recv_custom_header(), "One-To-One-Chat-Room");
	if (oneToOneChatRoomStr && (strcmp(oneToOneChatRoomStr, "true") == 0))
		d->capabilities |= ServerGroupChatRoom::Capabilities::OneToOne;
	shared_ptr<CallSession> session = getMe()->getPrivate()->createSession(*this, nullptr, false, d);
	session->configure(LinphoneCallIncoming, nullptr, op, Address(op->get_from()), Address(op->get_to()));
}

ServerGroupChatRoom::ServerGroupChatRoom (
	const shared_ptr<Core> &core,
	const IdentityAddress &peerAddress,
	AbstractChatRoom::CapabilitiesMask capabilities,
	const string &subject,
	list<shared_ptr<Participant>> &&participants,
	unsigned int lastNotifyId
) : ChatRoom(*new ServerGroupChatRoomPrivate, core, ChatRoomId(peerAddress, peerAddress)),
LocalConference(getCore(), peerAddress, nullptr) {
	L_D();
	L_D_T(LocalConference, dConference);

	d->capabilities |= capabilities & ServerGroupChatRoom::Capabilities::OneToOne;
	dConference->subject = subject;
	dConference->participants = move(participants);
	dConference->conferenceAddress = peerAddress;
	dConference->eventHandler->setLastNotify(lastNotifyId);
	dConference->eventHandler->setChatRoomId(d->chatRoomId);

	// Handle transitional states (joining and leaving of participants)
	d->filteredParticipants = getParticipants();
	for (const auto &participant : getParticipants()) {
		ParticipantDevice::State state = ParticipantDevice::State::Present;
		for (const auto &device : participant->getPrivate()->getDevices()) {
			if (d->getParticipantDeviceState(device) == ParticipantDevice::State::Leaving) {
				state = ParticipantDevice::State::Leaving;
				break;
			} else if (d->getParticipantDeviceState(device) == ParticipantDevice::State::Joining)
				state = ParticipantDevice::State::Joining;
		}

		if (state == ParticipantDevice::State::Leaving) {
			removeParticipant(participant);
		} else if (state == ParticipantDevice::State::Joining) {
			for (const auto &device : participant->getPrivate()->getDevices()) {
				if (d->getParticipantDeviceState(device) == ParticipantDevice::State::Joining)
					d->inviteDevice(device);
			}
		}
	}
}

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

void ServerGroupChatRoom::addParticipant (const IdentityAddress &addr, const CallSessionParams *params, bool hasMedia) {
	L_D();
	L_D_T(LocalConference, dConference);
	if (findParticipant(addr)) {
		lInfo() << "Not adding participant '" << addr.asString() << "' because it is already a participant of the ServerGroupChatRoom";
		return;
	}

	if ((d->capabilities & ServerGroupChatRoom::Capabilities::OneToOne) && (getParticipantCount() == 2)) {
		lInfo() << "Not adding participant '" << addr.asString() << "' because the OneToOne ServerGroupChatRoom already has 2 participants";
		return;
	}

	LocalConference::addParticipant(addr, params, hasMedia);
	d->filteredParticipants.push_back(findParticipant(addr));
	shared_ptr<ConferenceParticipantEvent> event = dConference->eventHandler->notifyParticipantAdded(addr);
	getCore()->getPrivate()->mainDb->addEvent(event);

	LinphoneChatRoom *cr = L_GET_C_BACK_PTR(this);
	LinphoneChatRoomCbs *cbs = linphone_chat_room_get_callbacks(cr);
	LinphoneChatRoomCbsParticipantDeviceFetchedCb cb = linphone_chat_room_cbs_get_participant_device_fetched(cbs);
	if (cb) {
		LinphoneAddress *laddr = linphone_address_new(addr.asString().c_str());
		cb(cr, laddr);
		linphone_address_unref(laddr);
	}
}

void ServerGroupChatRoom::addParticipants (const list<IdentityAddress> &addresses, const CallSessionParams *params, bool hasMedia) {
	LocalConference::addParticipants(addresses, params, hasMedia);
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

void ServerGroupChatRoom::removeParticipant (const shared_ptr<const Participant> &participant) {
	L_D();
	for (const auto &device : participant->getPrivate()->getDevices()) {
		if (d->getParticipantDeviceState(device) != ParticipantDevice::State::Present)
			continue;
		d->setParticipantDeviceState(device, ParticipantDevice::State::Leaving);
		SalReferOp *referOp = new SalReferOp(getCore()->getCCore()->sal);
		LinphoneAddress *lAddr = linphone_address_new(device->getAddress().asString().c_str());
		linphone_configure_op(getCore()->getCCore(), referOp, lAddr, nullptr, false);
		linphone_address_unref(lAddr);
		Address referToAddr = getConferenceAddress();
		referToAddr.setParam("text");
		referToAddr.setUriParam("method", "BYE");
		referOp->send_refer(referToAddr.getPrivate()->getInternalAddress());
		referOp->unref();
	}
	d->removeParticipant(participant);
}

void ServerGroupChatRoom::removeParticipants (const list<shared_ptr<Participant>> &participants) {
	LocalConference::removeParticipants(participants);
}

void ServerGroupChatRoom::setParticipantAdminStatus (const shared_ptr<Participant> &participant, bool isAdmin) {
	L_D_T(LocalConference, dConference);
	if (isAdmin != participant->isAdmin()) {
		participant->getPrivate()->setAdmin(isAdmin);
		shared_ptr<ConferenceParticipantEvent> event = dConference->eventHandler->notifyParticipantSetAdmin(participant->getAddress(), participant->isAdmin());
		getCore()->getPrivate()->mainDb->addEvent(event);
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

LINPHONE_END_NAMESPACE
