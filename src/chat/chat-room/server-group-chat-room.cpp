/*
 * server-group-chat-room.cpp
 * Copyright (C) 2010-2017 Belledonne Communications SARL
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
	shared_ptr<Participant> participant = make_shared<Participant>(addr);
	qConference->getPrivate()->participants.push_back(participant);
	shared_ptr<ConferenceParticipantEvent> event = qConference->getPrivate()->eventHandler->notifyParticipantAdded(addr);
	q->getCore()->getPrivate()->mainDb->addEvent(event);
	return participant;
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
		op->decline(SalReasonDeclined, nullptr);
		return;
	}

	bool isCreation = q->getParticipantCount() == 0;
	if (isCreation) {
		// First participant (creator of the chat room)
		participant = addParticipant(IdentityAddress(op->get_from()));
		participant->getPrivate()->setAdmin(true);
	} else {
		// INVITE coming from an invited participant
		participant = q->findParticipant(IdentityAddress(op->get_from()));
		if (!participant) {
			op->decline(SalReasonDeclined, nullptr);
			return;
		}
	}

	IdentityAddress gruu(contactAddr);
	shared_ptr<ParticipantDevice> device = participant->getPrivate()->addDevice(gruu);
	shared_ptr<CallSession> session = device->getSession();
	if (!session) {
		session = participant->getPrivate()->createSession(*q, nullptr, false, this);
		session->configure(LinphoneCallIncoming, nullptr, op, participant->getAddress(), Address(op->get_to()));
		session->startIncomingNotification();
		Address addr = qConference->getPrivate()->conferenceAddress;
		addr.setParam("isfocus");
		session->getPrivate()->getOp()->set_contact_address(addr.getPrivate()->getInternalAddress());
		device->setSession(session);
	}
	if (!isCreation)
		session->accept();

	// Changes are only allowed from admin participants
	if (participant->isAdmin())
		update(op);
}

shared_ptr<Participant> ServerGroupChatRoomPrivate::findRemovedParticipant (const shared_ptr<const CallSession> &session) const {
	for (const auto &participant : removedParticipants) {
		for (const auto &device : participant->getPrivate()->getDevices()) {
			if (device->getSession() == session)
				return participant;
		}
	}
	return nullptr;
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

	// Remove participant before notifying so that the removed participant is not notified of its own removal
	for (const auto &p : qConference->getPrivate()->participants) {
		if (participant->getAddress() == p->getAddress()) {
			// Keep the participant in the removedParticipants list so that to keep the CallSession alive and
			// be able to answer to the BYE request.
			removedParticipants.push_back(p);
			qConference->getPrivate()->participants.remove(p);
			break;
		}
	}

	shared_ptr<ConferenceParticipantEvent> event = qConference->getPrivate()->eventHandler->notifyParticipantRemoved(participant->getAddress());
	q->getCore()->getPrivate()->mainDb->addEvent(event);
	if (q->getParticipantCount() == 0)
		chatRoomListener->onChatRoomDeleteRequested(q->getSharedFromThis());
	else if (!isAdminLeft())
		designateAdmin();
}

void ServerGroupChatRoomPrivate::subscribeReceived (LinphoneEvent *event) {
	L_Q_T(LocalConference, qConference);
	qConference->getPrivate()->eventHandler->subscribeReceived(event);
}

void ServerGroupChatRoomPrivate::update (SalCallOp *op) {
	L_Q();
	if (sal_custom_header_find(op->get_recv_custom_header(), "Subject")) {
		// Handle subject change
		q->setSubject(L_C_TO_STRING(op->get_subject()));
	}
	// Handle participants addition
	const Content &content = op->get_remote_body();
	if ((content.getContentType() == ContentType::ResourceLists)
		&& (content.getContentDisposition() == "recipient-list")) {
		list<IdentityAddress> identAddresses = q->parseResourceLists(content.getBodyAsString());
		list<Address> addresses;
		for (const auto &addr : identAddresses) {
			addresses.push_back(Address(addr));
		}

		bctbx_list_t * cAddresses = L_GET_RESOLVED_C_LIST_FROM_CPP_LIST(addresses);

		LinphoneChatRoom *cr = L_GET_C_BACK_PTR(q);
		LinphoneChatRoomCbs *cbs = linphone_chat_room_get_callbacks(cr);
		LinphoneChatRoomCbsParticipantsCapabilitiesCheckedCb cb = linphone_chat_room_cbs_get_participants_capabilities_checked(cbs);
		if (cb) {
			LinphoneAddress *deviceAddr = linphone_address_new(op->get_remote_contact());
			cb(cr, deviceAddr, cAddresses);
			linphone_address_unref(deviceAddr);
		}
	}
}

// -----------------------------------------------------------------------------

void ServerGroupChatRoomPrivate::dispatchMessage (const IdentityAddress &fromAddress, const Content &content) {
	L_Q();
	L_Q_T(LocalConference, qConference);
	for (const auto &p : qConference->getPrivate()->participants) {
		for (const auto &device : p->getPrivate()->getDevices()) {
			if (fromAddress != device->getAddress()) {
				shared_ptr<ChatMessage> msg = q->createChatMessage();
				msg->setInternalContent(content);
				msg->getPrivate()->forceFromAddress(q->getConferenceAddress());
				msg->getPrivate()->forceToAddress(device->getAddress());
				msg->getPrivate()->setApplyModifiers(false);
				msg->send();
			}
		}
	}
}

LinphoneReason ServerGroupChatRoomPrivate::onSipMessageReceived (SalOp *op, const SalMessage *message) {
	L_Q();
	// Check that the message is coming from a participant of the chat room
	IdentityAddress fromAddr(op->get_from());
	if (!q->findParticipant(fromAddr)) {
		return LinphoneReasonNotAcceptable;
	}
	// Check that we received a CPIM message
	ContentType contentType(message->content_type);
	if (contentType != ContentType::Cpim)
		return LinphoneReasonNotAcceptable;
	Content content;
	content.setContentType(message->content_type);
	content.setBody(message->text ? message->text : "");
	dispatchMessage(fromAddr, content);
	return LinphoneReasonNone;
}

void ServerGroupChatRoomPrivate::setConferenceAddress (const IdentityAddress &conferenceAddress) {
	L_Q();
	L_Q_T(LocalConference, qConference);
	if (q->getState() != ChatRoom::State::Instantiated)
		return;
	qConference->getPrivate()->conferenceAddress = conferenceAddress;
	finalizeCreation();
}

void ServerGroupChatRoomPrivate::setParticipantDevices(const IdentityAddress &addr, const list<IdentityAddress> &devices) {
	L_Q();
	shared_ptr<Participant> participant = q->findParticipant(addr);
	for (const auto &deviceAddr : devices) {
		if (participant->getPrivate()->findDevice(deviceAddr))
			continue;
		shared_ptr<ParticipantDevice> device = participant->getPrivate()->addDevice(deviceAddr);
		SalReferOp *referOp = new SalReferOp(q->getCore()->getCCore()->sal);
		LinphoneAddress *lAddr = linphone_address_new(device->getAddress().asString().c_str());
		linphone_configure_op(q->getCore()->getCCore(), referOp, lAddr, nullptr, false);
		linphone_address_unref(lAddr);
		Address referToAddr = q->getConferenceAddress();
		referToAddr.setParam("text");
		referOp->send_refer(referToAddr.getPrivate()->getInternalAddress());
		referOp->unref();
	}
}

void ServerGroupChatRoomPrivate::addCompatibleParticipants(const IdentityAddress &deviceAddr, const list<IdentityAddress> &participantCompatible) {
	L_Q();
	shared_ptr<Participant> participant = q->findParticipant(deviceAddr);
	shared_ptr<ParticipantDevice> device = participant->getPrivate()->findDevice(deviceAddr);
	if (participantCompatible.empty()) {
		device->getSession()->decline(LinphoneReasonNotAcceptable);
	} else {
		device->getSession()->accept();
		LinphoneChatRoom *cr = L_GET_C_BACK_PTR(q);
		LinphoneChatRoomCbs *cbs = linphone_chat_room_get_callbacks(cr);
		LinphoneChatRoomCbsParticipantDeviceFetchedCb cb = linphone_chat_room_cbs_get_participant_device_fetched(cbs);
		if (cb) {
			LinphoneAddress *laddr = linphone_address_new(participant->getAddress().asString().c_str());
			cb(cr, laddr);
			linphone_address_unref(laddr);
		}
		q->addParticipants(participantCompatible, nullptr, false);
	}
}

// -----------------------------------------------------------------------------

void ServerGroupChatRoomPrivate::designateAdmin () {
	L_Q();
	L_Q_T(LocalConference, qConference);
	q->setParticipantAdminStatus(qConference->getPrivate()->participants.front(), true);
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
	chatRoomListener->onChatRoomInsertRequested(q->getSharedFromThis());
	setState(ChatRoom::State::Created);
	chatRoomListener->onChatRoomInsertInDatabaseRequested(q->getSharedFromThis());
}

bool ServerGroupChatRoomPrivate::isAdminLeft () const {
	L_Q_T(LocalConference, qConference);
	for (const auto &p : qConference->getPrivate()->participants) {
		if (p->isAdmin())
			return true;
	}
	return false;
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
	Core::deleteChatRoom(chatRoom);
}

// -----------------------------------------------------------------------------

void ServerGroupChatRoomPrivate::onCallSessionStateChanged (const std::shared_ptr<const CallSession> &session, LinphoneCallState newState, const std::string &message) {
	L_Q();
	if (newState == LinphoneCallEnd) {
		shared_ptr<Participant> participant = q->findParticipant(session);
		if (participant)
			removeParticipant(participant);
		participant = findRemovedParticipant(session);
		if (participant)
			removedParticipants.remove(participant);
	} else if (newState == LinphoneCallUpdatedByRemote) {
		shared_ptr<Participant> participant = q->findParticipant(session);
		if (participant && participant->isAdmin())
			update(session->getPrivate()->getOp());
	}
}

// =============================================================================

ServerGroupChatRoom::ServerGroupChatRoom (const std::shared_ptr<Core> &core, SalCallOp *op)
: ChatRoom(*new ServerGroupChatRoomPrivate, core, ChatRoomId()),
LocalConference(getCore(), IdentityAddress(linphone_core_get_conference_factory_uri(core->getCCore())), nullptr) {
	L_D();
	LocalConference::setSubject(op->get_subject() ? op->get_subject() : "");
	getMe()->getPrivate()->createSession(*this, nullptr, false, d);
	getMe()->getPrivate()->getSession()->configure(LinphoneCallIncoming, nullptr, op, Address(op->get_from()), Address(op->get_to()));
}

ServerGroupChatRoom::ServerGroupChatRoom (
	const std::shared_ptr<Core> &core,
	const IdentityAddress &peerAddress,
	const std::string &subject,
	std::list<std::shared_ptr<Participant>> &&participants,
	unsigned int lastNotifyId
) : ChatRoom(*new ServerGroupChatRoomPrivate, core, ChatRoomId(peerAddress, peerAddress)),
LocalConference(getCore(), peerAddress, nullptr) {
	L_D();
	L_D_T(LocalConference, dConference);

	dConference->subject = subject;
	dConference->participants = move(participants);
	dConference->eventHandler->setLastNotify(lastNotifyId);
	dConference->eventHandler->setChatRoomId(d->chatRoomId);
}

shared_ptr<Core> ServerGroupChatRoom::getCore () const {
	return ChatRoom::getCore();
}

shared_ptr<Participant> ServerGroupChatRoom::findParticipant (const shared_ptr<const CallSession> &session) const {
	L_D_T(LocalConference, dConference);

	for (const auto &participant : dConference->participants) {
		shared_ptr<ParticipantDevice> device = participant->getPrivate()->findDevice(session);
		if (device)
			return participant;
	}

	return nullptr;
}

ServerGroupChatRoom::CapabilitiesMask ServerGroupChatRoom::getCapabilities () const {
	return Capabilities::Conference;
}

bool ServerGroupChatRoom::hasBeenLeft () const {
	return false;
}

// -----------------------------------------------------------------------------

void ServerGroupChatRoom::addParticipant (const IdentityAddress &addr, const CallSessionParams *params, bool hasMedia) {
	L_D_T(LocalConference, dConference);
	if (findParticipant(addr)) {
		lInfo() << "Not adding participant '" << addr.asString() << "' because it is already a participant of the ServerGroupChatRoom";
		return;
	}

	LocalConference::addParticipant(addr, params, hasMedia);
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

shared_ptr<Participant> ServerGroupChatRoom::findParticipant (const IdentityAddress &addr) const {
	return LocalConference::findParticipant(addr);
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
	return LocalConference::getParticipants();
}

const string &ServerGroupChatRoom::getSubject () const {
	return LocalConference::getSubject();
}

void ServerGroupChatRoom::join () {}

void ServerGroupChatRoom::leave () {}

void ServerGroupChatRoom::removeParticipant (const shared_ptr<const Participant> &participant) {
	L_D();
	for (const auto &device : participant->getPrivate()->getDevices()) {
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
	// TODO: Wait for the response to the REFER to really remove the participant
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

void ServerGroupChatRoom::setSubject (const std::string &subject) {
	L_D_T(LocalConference, dConference);
	if (subject != getSubject()) {
		LocalConference::setSubject(subject);
		shared_ptr<ConferenceSubjectEvent> event = dConference->eventHandler->notifySubjectChanged();
		getCore()->getPrivate()->mainDb->addEvent(event);
	}
}

LINPHONE_END_NAMESPACE
