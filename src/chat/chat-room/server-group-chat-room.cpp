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

#include "address/address.h"
#include "address/identity-address.h"
#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "chat/chat-message/chat-message-p.h"
#include "chat/modifier/cpim-chat-message-modifier.h"
#include "conference/handlers/local-conference-event-handler.h"
#include "conference/handlers/local-conference-list-event-handler.h"
#include "conference/local-conference.h"
#include "conference/participant.h"
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

// -----------------------------------------------------------------------------

ParticipantDeviceIdentity::ParticipantDeviceIdentity (const Address &address, const string &name) : ClonableObject(*new ParticipantDeviceIdentityPrivate) {
	L_D();
	d->deviceAddress = address;
	d->deviceName = name;
}

ParticipantDeviceIdentity::ParticipantDeviceIdentity (const ParticipantDeviceIdentity &other) : ClonableObject(*new ParticipantDeviceIdentityPrivate) {
	L_D();
	d->deviceAddress = other.getAddress();
	d->deviceName = other.getName();
}

const Address &ParticipantDeviceIdentity::getAddress () const {
	L_D();
	return d->deviceAddress;
}

const string &ParticipantDeviceIdentity::getName () const {
	L_D();
	return d->deviceName;
}

// -----------------------------------------------------------------------------

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
	L_Q_T(LocalConference, qConference);
	ChatRoomPrivate::setState(state);
	if (state == ChatRoom::State::Created) {
		// Handle transitional states (joining and leaving of participants)
		// This is needed when the chat room is loaded from its state in database
		list<IdentityAddress> participantAddresses;
		for (const auto &participant : qConference->participants) {
			participantAddresses.emplace_back(participant->getAddress());

			if (capabilities & ServerGroupChatRoom::Capabilities::OneToOne){
				// Even if devices can BYE and get rid of their session, actually no one can leave a one to one chatroom.
				authorizedParticipants.push_back(participant);
			}else{
				bool atLeastOneDeviceJoining = false;
				bool atLeastOneDevicePresent = false;
				bool atLeastOneDeviceLeaving = false;
				for (const auto &device : participant->getDevices()) {
					switch (device->getState()) {
						case ParticipantDevice::State::ScheduledForLeaving:
						case ParticipantDevice::State::Leaving:
							atLeastOneDeviceLeaving = true;
							break;
						case ParticipantDevice::State::ScheduledForJoining:
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
				//Basically the only case where this participant is not authorized is in case it was removed from the room but not all
				//its devices were "BYEed" yet. This is what the line below is testing. Might be better to add a new state in the participant Class,
				// but it's not the case yet.
				if (atLeastOneDevicePresent || atLeastOneDeviceJoining || atLeastOneDeviceLeaving == false ){
					authorizedParticipants.push_back(participant);
				}
			}
		}
		updateParticipantsSessions();
		// Subscribe to the registration events from the proxy
		subscribeRegistrationForParticipants(participantAddresses, false);
	}
}

shared_ptr<Participant> ServerGroupChatRoomPrivate::addParticipant (const IdentityAddress &addr) {
	L_Q();
	L_Q_T(LocalConference, qConference);

	shared_ptr<Participant> participant = q->findParticipant(addr);
	if (!participant) {
		participant = Participant::create(qConference,addr);
		qConference->participants.push_back(participant);
	}
	/* Case of participant that is still referenced in the chatroom, but no longer authorized because it has been removed
	 * previously OR a totally new participant. */
	if (findAuthorizedParticipant(addr) == nullptr){
		authorizedParticipants.push_back(participant);
		shared_ptr<ConferenceParticipantEvent> event = qConference->notifyParticipantAdded(time(nullptr), false, addr);
		q->getCore()->getPrivate()->mainDb->addEvent(event);
	}
	return participant;
}

/* This function is used to re-join devices of a participant that has left previously. Its device are still referenced until they 're all left. */
void ServerGroupChatRoomPrivate::resumeParticipant(const std::shared_ptr<Participant> &participant){
	addParticipant(participant->getAddress());
	for (auto device : participant->getDevices()){
		switch(device->getState()){
			case ParticipantDevice::State::Leaving:
			case ParticipantDevice::State::Left:
			case ParticipantDevice::State::ScheduledForLeaving:
				setParticipantDeviceState(device, ParticipantDevice::State::ScheduledForJoining);
				updateParticipantDeviceSession(device);
			break;
			default:
			break;
		}
	}
}

void ServerGroupChatRoomPrivate::setParticipantDeviceState (const shared_ptr<ParticipantDevice> &device, ParticipantDevice::State state) {
	L_Q();
	string address(device->getAddress().asString());
	lInfo() << q << ": Set participant device '" << address << "' state to " << state;
	device->setState(state);
	q->getCore()->getPrivate()->mainDb->updateChatRoomParticipantDevice(q->getSharedFromThis(), device);
	switch (state){
		case ParticipantDevice::State::ScheduledForLeaving:
		case ParticipantDevice::State::Leaving:
			queuedMessages.erase(address);
		break;
		case ParticipantDevice::State::Left:
			queuedMessages.erase(address);
			onParticipantDeviceLeft(device);
		break;
		default:
		break;
	}
}

void ServerGroupChatRoomPrivate::acceptSession (const shared_ptr<CallSession> &session) {
	if (session->getState() == CallSession::State::UpdatedByRemote)
		session->acceptUpdate();
	else
		session->accept();
}

void ServerGroupChatRoomPrivate::confirmCreation () {
	L_Q();

	shared_ptr<Participant> me = q->getMe();
	shared_ptr<CallSession> session = me->getSession();
	session->startIncomingNotification(false);

	LinphoneChatRoom *cr = L_GET_C_BACK_PTR(q);
	CALL_CHAT_ROOM_CBS(cr, ConferenceAddressGeneration, conference_address_generation, cr);
}

void ServerGroupChatRoomPrivate::requestDeletion(){
	L_Q();

	/*
	 * Our ServerGroupChatRoom is registered as listener to the numerous CallSession it manages,
	 * and is subscribing for registration events.
	 * It is time to remove this listener and unsubscribe, because we are going to disapear and we don't want
	 * to be notified anymore of anything.
	 */
	for (auto participant : q->getParticipants()){
		unSubscribeRegistrationForParticipant(participant->getAddress());
		for (auto devices : participant->getDevices()){
			auto session = devices->getSession();
			if (session) session->setListener(nullptr);
		}
	}
	if (!registrationSubscriptions.empty()){
		lError() << q << " still " << registrationSubscriptions.size() << " registration subscriptions pending while deletion is requested.";
	}
	chatRoomListener->onChatRoomDeleteRequested(q->getSharedFromThis());
	/*
	 * The LinphoneChatRoom (C object) is also built when the ServerGroupChatRoom is created, which is unnecessary here, but
	 * this is consistent with other kind of chatrooms. Consequence is that we have to free it in order to remove the last reference
	 * to the C++ object.
	 * The destruction is defered to next main loop iteration, in order to make the self-destruction outside of the call stack that leaded to it.
	 */
	//just in case ServerGroupChatRoom is destroyed before lambda is executed
	std::weak_ptr<ChatRoom> cppChatRoom(q->getSharedFromThis());
	q->getCore()->doLater([cppChatRoom](){
		auto obj = cppChatRoom.lock();
		if (obj) {
			LinphoneChatRoom * cChatRoom = L_GET_C_BACK_PTR(obj);
			if (cChatRoom) linphone_chat_room_unref(cChatRoom);
		}
	});
}

/*
 * We go in this method in two cases:
 * - when the client who created the conference INVITEs the conference address that was specified
 *   in the redirect response consecutive to the first INVITE that was made to the factory uri.
 *   In this case, joiningPendingAfterCreation is set to true.
 * - when an already joined participant device reconnects for whatever reason.
 */
void ServerGroupChatRoomPrivate::confirmJoining (SalCallOp *op) {
	L_Q();
	L_Q_T(LocalConference, qConference);
	shared_ptr<Participant> participant;

	Address contactAddr(op->getRemoteContact());
	if (contactAddr.getUriParamValue("gr").empty()) {
		lError() << q << ": Declining INVITE because the contact does not have a 'gr' uri parameter [" << contactAddr.asString() << "]";
		op->decline(SalReasonDeclined, "");
		joiningPendingAfterCreation = false;
		return;
	}

	IdentityAddress gruu(contactAddr);
	shared_ptr<ParticipantDevice> device;
	shared_ptr<CallSession> session;
	if (joiningPendingAfterCreation) {
		// Check if the participant is already there, this INVITE may come from an unknown device of an already present participant
		participant = addParticipant(IdentityAddress(op->getFrom()));
		participant->setAdmin(true);
		device = participant->addDevice(gruu);
		session = device->getSession();
		mInitiatorDevice = device;

		/*Since the initiator of the chatroom has not yet subscribed at this stage, this won't generate NOTIFY, the events will be queued. */
		shared_ptr<ConferenceParticipantDeviceEvent> deviceEvent = qConference->notifyParticipantDeviceAdded(time(nullptr), false, participant->getAddress(), gruu);
		q->getCore()->getPrivate()->mainDb->addEvent(deviceEvent);
		if (!(capabilities & ServerGroupChatRoom::Capabilities::OneToOne)) {
			shared_ptr<ConferenceParticipantEvent> adminEvent = qConference->notifyParticipantSetAdmin(time(nullptr), false, participant->getAddress(), true);
			q->getCore()->getPrivate()->mainDb->addEvent(adminEvent);
		}
	} else {
		// INVITE coming from an invited participant
		participant = q->findParticipant(IdentityAddress(op->getFrom()));
		if (!participant) {
			lError() << q << ": Declining INVITE coming from someone that is not a participant";
			op->decline(SalReasonDeclined, "");
			return;
		}
		// One to one chatroom can be resurected by a participant, but the participant actually never leaves from server's standpoint.
		if (!(capabilities & ServerGroupChatRoom::Capabilities::OneToOne) && op->getRemoteBody().getContentType() == ContentType::ResourceLists){
			lError() << q << "Receiving ressource list body while not in creation step.";
			op->decline(SalReasonNotAcceptable);
			return;
		}
		device = participant->addDevice(gruu);
		if (capabilities & ServerGroupChatRoom::Capabilities::OneToOne){
			if (device->getState() == ParticipantDevice::State::Left){
				lInfo() << q << " " << gruu << " is reconnected to the one to one chatroom.";
				setParticipantDeviceState(device, ParticipantDevice::State::Joining);
			}
			participant->setAdmin(true);
		}
		session = device->getSession();
	}

	if (!session || (session->getPrivate()->getOp() != op)) {
		CallSessionParams params;
		//params.addCustomContactParameter("isfocus");
		session = participant->createSession(*q, &params, false, this);
		session->configure(LinphoneCallIncoming, nullptr, op, participant->getAddress(), Address(op->getTo()));
		session->startIncomingNotification(false);
		Address addr = qConference->conferenceAddress;
		addr.setParam("isfocus");
		//to force is focus to be added
		session->getPrivate()->getOp()->setContactAddress(addr.getInternalAddress());
		device->setSession(session);
	}

	// Changes are only allowed from admin participants
	if (participant->isAdmin()) {
		if (joiningPendingAfterCreation){
			if (!initializeParticipants(participant, op)){
 				op->decline(SalReasonNotAcceptable, "");
 				requestDeletion();
 			}
			/* we don't accept the session yet: initializeParticipants() has launched queries for device information
			 * that will later populate the chatroom*/
		}else{
			/* after creation, only subject change is allowed*/
			handleSubjectChange(op);
			acceptSession(session);
 		}
 	} else {
		/*it is a non-admin participant that reconnected to the chatroom*/
 		acceptSession(session);
 	}
}

/* This is called only when a participant attempts to create a one to one chatroom that already exists.
 * We just redirect it to the existing chatroom uri. */
void ServerGroupChatRoomPrivate::confirmRecreation (SalCallOp *op) {
	L_Q();
	L_Q_T(LocalConference, qConference);

	auto participant = q->findParticipant(Address(op->getFrom()));
	if (!participant){
		lError() << q << " bug - " << op->getFrom() << " is not a participant.";
		op->decline(SalReasonInternalError, "");
		return;
	}

	IdentityAddress confAddr(qConference->conferenceAddress);

	lInfo() << q << " is re-joined by " << participant->getAddress();
	Address addr(confAddr);
	addr.setParam("isfocus");
	shared_ptr<Participant> me = q->getMe();
	shared_ptr<CallSession> session = me->createSession(*q, nullptr, false, this);
	session->configure(LinphoneCallIncoming, nullptr, op, Address(op->getFrom()), Address(op->getTo()));
	session->startIncomingNotification(false);
	session->redirect(addr);
}

void ServerGroupChatRoomPrivate::declineSession (const shared_ptr<CallSession> &session, LinphoneReason reason) {
	session->decline(reason);
}

void ServerGroupChatRoomPrivate::dispatchQueuedMessages () {
	L_Q();
	for (const auto &participant : q->getParticipants()) {
		/*
		 * Dispatch messages for each device in Present state. In a one to one chatroom, if a device
		 * is found is Left state, it must be invited first.
		 */

		for (const auto &device : participant->getDevices()) {

			string uri(device->getAddress().asString());
			auto & msgQueue = queuedMessages[uri];

			if (!msgQueue.empty()){
				if ( (capabilities & ServerGroupChatRoom::Capabilities::OneToOne) && device->getState() == ParticipantDevice::State::Left){
					lInfo() << "There is a message to transmit to a participant in left state in a one to one chatroom, so inviting first.";
					inviteDevice(device);
					continue;
				}
				if (device->getState() != ParticipantDevice::State::Present)
					continue;
				size_t nbMessages = msgQueue.size();
				lInfo() << q << ": Dispatching " << nbMessages << " queued message(s) for '" << uri << "'";
				while (!msgQueue.empty()) {
					shared_ptr<Message> msg = msgQueue.front();
					sendMessage(msg, device->getAddress());
					msgQueue.pop();
				}
			}
		}
	}
}

void ServerGroupChatRoomPrivate::removeParticipant (const shared_ptr<const Participant> &participant) {
	L_Q();
	L_Q_T(LocalConference, qConference);

	for (const auto &device : participant->getDevices()) {
		if ((device->getState() == ParticipantDevice::State::Leaving)
			|| (device->getState() == ParticipantDevice::State::Left)
		)
			continue;
		setParticipantDeviceState(device, ParticipantDevice::State::ScheduledForLeaving);
		updateParticipantDeviceSession(device);
	}

	for (const auto &p : authorizedParticipants) {
		if (participant->getAddress() == p->getAddress()) {
			lInfo() << q <<" 'participant ' "<< p->getAddress() <<" no more authorized'";
			authorizedParticipants.remove(p);
			break;
		}
	}

	queuedMessages.erase(participant->getAddress().asString());

	shared_ptr<ConferenceParticipantEvent> event = qConference->notifyParticipantRemoved(time(nullptr), false, participant->getAddress());
	q->getCore()->getPrivate()->mainDb->addEvent(event);

	if (!isAdminLeft())
		designateAdmin();
}

shared_ptr<Participant> ServerGroupChatRoomPrivate::findAuthorizedParticipant (const shared_ptr<const CallSession> &session) const {
	for (const auto &participant : authorizedParticipants) {
		shared_ptr<ParticipantDevice> device = participant->findDevice(session);
		if (device || (participant->getSession() == session))
			return participant;
	}
	return nullptr;
}

shared_ptr<Participant> ServerGroupChatRoomPrivate::findAuthorizedParticipant (const IdentityAddress &participantAddress) const {
	IdentityAddress searchedAddr(participantAddress);
	searchedAddr.setGruu("");
	for (const auto &participant : authorizedParticipants) {
		if (participant->getAddress() == searchedAddr)
			return participant;
	}
	return nullptr;
}

void ServerGroupChatRoomPrivate::subscriptionStateChanged (LinphoneEvent *event, LinphoneSubscriptionState state) {
	L_Q_T(LocalConference, qConference);
	qConference->eventHandler->subscriptionStateChanged(event, state);
}

void ServerGroupChatRoomPrivate::handleSubjectChange(SalCallOp *op){
	L_Q();
	if (sal_custom_header_find(op->getRecvCustomHeaders(), "Subject")) {
		// Handle subject change
		lInfo() << q << ": New subject \"" << op->getSubject() << "\"";
		q->setSubject(op->getSubject());
	}
}

/*
 * This function setups registration subscriptions if not already there.
 * If no registration subscription is started (because they were all running already), it returns false.
 * newInvited specifies whether these participants are in the process of being invited, in which case they will be
 * automatically added to the invitedParticipants list, so that when registration info arrives, they
 * will be added.
 */
bool ServerGroupChatRoomPrivate::subscribeRegistrationForParticipants(const std::list<IdentityAddress> &identAddresses, bool newInvited){
	L_Q();
	std::list<IdentityAddress> requestedAddresses;
	bool subscriptionsPending = false;

	// Subscribe to the registration events from the proxy
	for (const auto &addr : identAddresses) {
		if (registrationSubscriptions.find(addr.asString()) == registrationSubscriptions.end()){
			requestedAddresses.emplace_back(addr);
			if (newInvited) invitedParticipants.emplace_back(addr);
			unnotifiedRegistrationSubscriptions++;
			subscriptionsPending = true;
		}
	}

	for (const auto &addr : requestedAddresses){
		LinphoneChatRoom *cr = L_GET_C_BACK_PTR(q);
		LinphoneAddress *laddr = linphone_address_new(addr.asString().c_str());
		registrationSubscriptions[addr.asString()].context = nullptr; // we 'll put here later a context pointer returned by the callback.
		CALL_CHAT_ROOM_CBS(cr, ParticipantRegistrationSubscriptionRequested, participant_registration_subscription_requested, cr, laddr);
		linphone_address_unref(laddr);
	}
	return subscriptionsPending;
}

void ServerGroupChatRoomPrivate::unSubscribeRegistrationForParticipant(const IdentityAddress &identAddress){
	L_Q();
	auto p = registrationSubscriptions.find(identAddress.asString());
	if (p == registrationSubscriptions.end()){
		lError() << q << " no active subscription for " << identAddress;
		return;
	}
	registrationSubscriptions.erase(p);

	LinphoneChatRoom *cr = L_GET_C_BACK_PTR(q);
	LinphoneAddress *laddr = linphone_address_new(identAddress.asString().c_str());
	CALL_CHAT_ROOM_CBS(cr, ParticipantRegistrationUnsubscriptionRequested, participant_registration_unsubscription_requested, cr, laddr);
	linphone_address_unref(laddr);
}

bool ServerGroupChatRoomPrivate::initializeParticipants (const shared_ptr<Participant> & initiator, SalCallOp *op) {

	handleSubjectChange(op);
	// Handle participants addition
	list<IdentityAddress> identAddresses = ServerGroupChatRoom::parseResourceLists(op->getRemoteBody());
	if (identAddresses.empty()){
		lError()<<"ServerGroupChatRoomPrivate::initializeParticipants(): empty list !";
		return false;
	}

	identAddresses.unique(); //Protection for the case where the client has sent a list with duplicates

	if (capabilities & ServerGroupChatRoom::Capabilities::OneToOne){
		if (identAddresses.size() > 1){
			lError()<<"ServerGroupChatRoomPrivate::initializeParticipants(): chatroom is one to one but the list contains multiple participants !";
			return false;
		}
	}
	identAddresses.push_back(initiator->getAddress());
	if (!subscribeRegistrationForParticipants(identAddresses, true)){
		/* If we are not waiting for any registration information, then we can conclude immediately. */
		conclude();
	}
	return true;
}

// -----------------------------------------------------------------------------

LinphoneReason ServerGroupChatRoomPrivate::onSipMessageReceived (SalOp *op, const SalMessage *message) {
	// Check that the message is coming from a participant of the chat room
	IdentityAddress fromAddr(op->getFrom());
	if (!findAuthorizedParticipant(fromAddr)) {
		return LinphoneReasonForbidden;
	}

	// Do not check that we received a CPIM message because ciphered messages are not
	shared_ptr<Message> msg = make_shared<Message>(
		op->getFrom(),
		ContentType(message->content_type),
		message->text ? message->text : "",
		op->getRecvCustomHeaders()
	);

	queueMessage(msg);
	dispatchQueuedMessages();
	return LinphoneReasonNone;
}

void ServerGroupChatRoomPrivate::setConferenceAddress (const ConferenceAddress &conferenceAddress) {
	L_Q();
	L_Q_T(LocalConference, qConference);

	if (!conferenceAddress.isValid()) {
		shared_ptr<CallSession> session = q->getMe()->getSession();
		LinphoneErrorInfo *ei = linphone_error_info_new();
		linphone_error_info_set(ei, "SIP", LinphoneReasonUnknown, 500, "Server internal error", NULL);
		session->decline(ei);
		linphone_error_info_unref(ei);
		setState(ChatRoom::State::CreationFailed);
		return;
	}

	if (q->getState() != ChatRoom::State::Instantiated) {
		lError() << "Cannot set the conference address of the ServerGroupChatRoom in state " << q->getState();
		return;
	}
	qConference->conferenceAddress = conferenceAddress;
	lInfo() << "The ServerGroupChatRoom has been given the address " << conferenceAddress.asString() << ", now finalizing its creation";
	finalizeCreation();
}


void ServerGroupChatRoomPrivate::updateParticipantDevices(const IdentityAddress &participantAddress, const list<ParticipantDeviceIdentity> &devices){
	L_Q();
	bool newParticipantReginfo = false;

	auto it = registrationSubscriptions.find(participantAddress.asString());

	/*
	* For security, since registration information might come from outside, make sure that the device list we are asked to add
	* are from a participant for which we have requested device information, ie a participant that is in the process of being
	* added to the chatroom
	*/
	if (it == registrationSubscriptions.end()){
		lError() << "updateParticipantDevices(): " << participantAddress << " registration info was not requested.";
		return;
	}else{
		// Check if this registration information is for a participant in the process of being added.
		auto it = find(invitedParticipants.begin(), invitedParticipants.end(), participantAddress);
		if (it != invitedParticipants.end()){
			invitedParticipants.erase(it);
			unnotifiedRegistrationSubscriptions--;
			newParticipantReginfo = true;
		}
	}
	shared_ptr<Participant> participant;

	if (newParticipantReginfo){
		if (!devices.empty()){
			participant = addParticipant(participantAddress);
		}else{
			lInfo() << q << participantAddress << " has no compatible devices.";
			unSubscribeRegistrationForParticipant(participantAddress);
			return;
		}
	}else{
		participant = q->findParticipant(participantAddress);
	}

	if (!participant){
		lError() << q << " participant devices updated for unknown participant, ignored.";
		return;
	}
	lInfo() << q << ": Setting " << devices.size() << " participant device(s) for " << participantAddress.asString();

	// Remove devices that are in the chatroom but no longer in the given list
	list<shared_ptr<ParticipantDevice>> devicesToRemove;
	for (const auto &device : participant->getDevices()) {
		auto predicate = [device] (const ParticipantDeviceIdentity & deviceIdentity) {
			return device->getAddress() == deviceIdentity.getAddress();
		};
		auto it = find_if(devices.cbegin(), devices.cend(), predicate);
		if (it == devices.cend()){
			lInfo() << q << "Device " << device << " is no longer registered, it will be removed from the chatroom.";
			devicesToRemove.push_back(device);
		}
	}
	// Add all the devices in the given list, if already present they will be ignored
	for (const auto &device : devices)
		addParticipantDevice(participant, device);

	// Remove all devices that are no longer existing.
	for (auto &device : devicesToRemove)
		removeParticipantDevice(participant, device->getAddress());
}

void ServerGroupChatRoomPrivate::conclude(){
	L_Q();
	lInfo() << q << "All devices are known, the chatroom creation can be concluded.";
	shared_ptr<CallSession> session = mInitiatorDevice->getSession();

	if (q->getParticipants().size() < 2){
		lError() << q << ": there are less than 2 participants in this chatroom, refusing creation.";
		declineSession(session, LinphoneReasonNotAcceptable);
		requestDeletion();
	}else{
		/* Ok we are going to accept the session with 200Ok. However we want to wait for the ACK to be sure
			* that the initiator is aware that he's now part of the conference, before we invite the others.
			*/
		acceptSession(session);
		if ((capabilities & ServerGroupChatRoom::Capabilities::OneToOne) && (q->getParticipantCount() == 2)) {
			// Insert the one-to-one chat room in Db if participants count is 2.
			q->getCore()->getPrivate()->mainDb->insertOneToOneConferenceChatRoom(q->getSharedFromThis(),
				!!(capabilities & ServerGroupChatRoom::Capabilities::Encrypted) );
		}
	}
}

void ServerGroupChatRoomPrivate::setParticipantDevicesAtCreation(const IdentityAddress &participantAddress, const list<ParticipantDeviceIdentity> &devices) {

	updateParticipantDevices(participantAddress, devices);
	if (unnotifiedRegistrationSubscriptions == 0){
		conclude();
	}
}

void ServerGroupChatRoomPrivate::setParticipantDevices(const IdentityAddress &participantAddress, const list<ParticipantDeviceIdentity> &devices) {

	if (joiningPendingAfterCreation){
		setParticipantDevicesAtCreation(participantAddress, devices);
	}else{
		updateParticipantDevices(participantAddress, devices);
		updateParticipantsSessions();
	}
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

/*
 * This method is in charge of applying the state of a participant device to the SIP session
 */
void ServerGroupChatRoomPrivate::updateParticipantDeviceSession(const shared_ptr<ParticipantDevice> &device, bool freshlyRegistered){
	switch (device->getState()) {
		case ParticipantDevice::State::ScheduledForJoining:
			inviteDevice(device);
		break;
		case ParticipantDevice::State::Joining:
			if (freshlyRegistered) inviteDevice(device);
		break;
		case ParticipantDevice::State::Left:
		break;
		case ParticipantDevice::State::ScheduledForLeaving:
			byeDevice(device);
		break;
		case ParticipantDevice::State::Leaving:
			if (freshlyRegistered) byeDevice(device);
		break;
		case ParticipantDevice::State::Present:
			//nothing to do
		break;
	}
}

/*
 * This method is in charge of applying to the SIP session
 * the state of all participant devices belonging to a participant.
 */
void ServerGroupChatRoomPrivate::updateParticipantsSessions(){
	L_Q();

	for (const auto &p : q->getParticipants()){
		for (const auto &device : p->getDevices()){
			updateParticipantDeviceSession(device);
		}
	}
}

void ServerGroupChatRoomPrivate::addParticipantDevice (const shared_ptr<Participant> &participant, const ParticipantDeviceIdentity &deviceInfo) {
	L_Q();
	L_Q_T(LocalConference, qConference);
	shared_ptr<ParticipantDevice> device = participant->findDevice(deviceInfo.getAddress());

	if (device) {
		// Nothing to do, but set the name because the user-agent is not known for the initiator device.
		device->setName(deviceInfo.getName());
	} else if (findAuthorizedParticipant(participant->getAddress())) {
		bool allDevLeft = !participant->getDevices().empty() && allDevicesLeft(participant);
		/*
		 * This is a really new device.
		 */
		device = participant->addDevice(deviceInfo.getAddress(), deviceInfo.getName());

		shared_ptr<ConferenceParticipantDeviceEvent> event = qConference->notifyParticipantDeviceAdded(time(nullptr), false, participant->getAddress(), deviceInfo.getAddress());
		q->getCore()->getPrivate()->mainDb->addEvent(event);

		if (capabilities & ServerGroupChatRoom::Capabilities::OneToOne && allDevLeft){
			/* If all other devices have left, let this new device to left state too, it will be invited to join if a message is sent to it. */
			setParticipantDeviceState(device, ParticipantDevice::State::Left);
		}else{
			setParticipantDeviceState(device, ParticipantDevice::State::ScheduledForJoining);
		}
	} else {
		lWarning()<< q << ": Participant device " << participant << " cannot be added because not authorized";
	}
}

void ServerGroupChatRoomPrivate::designateAdmin () {
	L_Q();
	// Do not designate new admin for one-to-one chat room
	if (!(capabilities & ServerGroupChatRoom::Capabilities::OneToOne) && !authorizedParticipants.empty()) {
		q->setParticipantAdminStatus(authorizedParticipants.front(), true);
		lInfo() << q << ": New admin designated";
	}
}

void ServerGroupChatRoomPrivate::sendMessage (const shared_ptr<Message> &message, const IdentityAddress &deviceAddr){
	L_Q();

	shared_ptr<ChatMessage> msg = q->createChatMessage();
	copyMessageHeaders(message, msg);
	msg->getPrivate()->addSalCustomHeader("Session-mode", "true"); // Special custom header to identify MESSAGE that belong to server group chatroom
	msg->setInternalContent(message->content);
	msg->getPrivate()->forceFromAddress(q->getConferenceAddress());
	msg->getPrivate()->forceToAddress(deviceAddr);
	msg->getPrivate()->setApplyModifiers(false);
	msg->send();
}

void ServerGroupChatRoomPrivate::finalizeCreation () {
	L_Q();
	L_Q_T(LocalConference, qConference);
	ConferenceAddress confAddr(qConference->conferenceAddress);
	const ConferenceId conferenceId = ConferenceId(confAddr, confAddr);
	qConference->setConferenceId(conferenceId);
	q->getCore()->getPrivate()->localListEventHandler->addHandler(qConference->eventHandler.get());
	lInfo() << q << " created";
	// Let the SIP stack set the domain and the port
	shared_ptr<Participant> me = q->getMe();
	me->setAddress(confAddr);
	Address addr(confAddr);
	addr.setParam("isfocus");
	shared_ptr<CallSession> session = me->getSession();
	if (session->getState() == CallSession::State::Idle) {
		lInfo() << " Scheduling redirection to [" << addr <<"] for Call session ["<<session<<"]" ;
		q->getCore()->doLater([session,addr] {
			session->redirect(addr);
		});
	} else {
			session->redirect(addr);
	}
	joiningPendingAfterCreation = true;
	chatRoomListener->onChatRoomInsertRequested(q->getSharedFromThis());
	setState(ChatRoom::State::Created);
	chatRoomListener->onChatRoomInsertInDatabaseRequested(q->getSharedFromThis());
}

//returns true if a new session has been created, false if there is already an existing and valid one.
shared_ptr<CallSession> ServerGroupChatRoomPrivate::makeSession(const std::shared_ptr<ParticipantDevice> &device){
	L_Q();
	L_Q_T(LocalConference, qConference);
	shared_ptr<CallSession> session = device->getSession();

	if (session){
		switch (session->getState()){
			case CallSession::State::End:
			case CallSession::State::Error:
			case CallSession::State::Released:
				session = nullptr; //our session is dead, we'll make a new one.
			break;
			default:
			break;
		}
	}
	if (!session) {
		CallSessionParams csp;
		if (capabilities & ServerGroupChatRoom::Capabilities::OneToOne)
			csp.addCustomHeader("One-To-One-Chat-Room", "true");
		if (capabilities & ServerGroupChatRoom::Capabilities::Encrypted)
			csp.addCustomHeader("End-To-End-Encrypted", "true");
		//csp.addCustomContactParameter("isfocus");
		//csp.addCustomContactParameter("text");
		shared_ptr<Participant> participant = const_pointer_cast<Participant>(device->getParticipant()->getSharedFromThis());
		session = participant->createSession(*q, &csp, false, this);
		session->configure(LinphoneCallOutgoing, nullptr, nullptr, qConference->conferenceAddress, device->getAddress());
		device->setSession(session);
		session->initiateOutgoing();
		session->getPrivate()->createOp();
		//FIXME jehan check conference server  potential impact
		Address contactAddr(qConference->conferenceAddress);
		contactAddr.setParam("isfocus");
		contactAddr.setParam("text");
		session->getPrivate()->getOp()->setContactAddress(contactAddr.getInternalAddress());

	}
	return session;
}

void ServerGroupChatRoomPrivate::inviteDevice (const shared_ptr<ParticipantDevice> &device) {
	L_Q();

	lInfo() << q << ": Inviting device '" << device->getAddress().asString() << "'";
	shared_ptr<Participant> participant = const_pointer_cast<Participant>(device->getParticipant()->getSharedFromThis());
	shared_ptr<CallSession> session = makeSession(device);
	if (device->getState() == ParticipantDevice::State::Joining && (
		session->getState() == CallSession::State::OutgoingProgress
		|| session->getState() == CallSession::State::Connected)){
		lInfo() << q << ": outgoing INVITE already in progress.";
		return;
	}
	setParticipantDeviceState(device, ParticipantDevice::State::Joining);
	if (session && session->getState() == CallSession::State::IncomingReceived){
		lInfo() << q << ": incoming INVITE in progress.";
		return;
	}

	list<IdentityAddress> addressesList;
	for (const auto &invitedParticipant : authorizedParticipants) {
		if (invitedParticipant != participant)
			addressesList.push_back(invitedParticipant->getAddress());
	}
	if (addressesList.empty()){
		// Having an empty participant list shall never happen, but should this happen don't spread the bug to clients.
		lError() << q << ": empty participant list, this should never happen, INVITE not sent.";
		return;
	}

	Content content;
	content.setBody(q->getResourceLists(addressesList));
	content.setContentType(ContentType::ResourceLists);
	content.setContentDisposition(ContentDisposition::RecipientListHistory);
	if (linphone_core_content_encoding_supported(q->getCore()->getCCore(), "deflate"))
		content.setContentEncoding("deflate");
	session->startInvite(nullptr, q->getSubject(), &content);
}

void ServerGroupChatRoomPrivate::byeDevice (const std::shared_ptr<ParticipantDevice> &device) {
	L_Q();

	lInfo() << q << ": Asking device '" << device->getAddress().asString() << "' to leave";
	setParticipantDeviceState(device, ParticipantDevice::State::Leaving);
	shared_ptr<CallSession> session = makeSession(device);
	switch(session->getState()){
		case CallSession::State::OutgoingInit:
			session->startInvite(nullptr, q->getSubject(), nullptr);
		break;
		case CallSession::State::Connected:
		case CallSession::State::StreamsRunning:
			session->terminate();
		break;
		default:
		break;
	}
}

/*
 * This method is to be called by the conference server when it is notified that a device has just registered*/
void ServerGroupChatRoomPrivate::notifyParticipantDeviceRegistration(const IdentityAddress &participantDevice){
	L_Q();

	shared_ptr<Participant> participant = q->findParticipant(participantDevice);
	if (!participant){
		lError() << q << ": " << participantDevice << " is not part of the chatroom.";
		return;
	}
	shared_ptr<ParticipantDevice> pd = participant->findDevice(participantDevice);
	if (!pd){
		/* A device that does not have the required capabilities may be notified. */
		lInfo() << q << ": device " << participantDevice << " is not part of any participant of the chatroom.";
		return;
	}
	updateParticipantDeviceSession(pd, true);
}

bool ServerGroupChatRoomPrivate::isAdminLeft () const {
	for (const auto &participant : authorizedParticipants) {
		if (participant->isAdmin())
			return true;
	}
	return false;
}

void ServerGroupChatRoomPrivate::queueMessage (const shared_ptr<Message> &msg) {
	L_Q();
	for (const auto &participant : q->getParticipants()) {
		for (const auto &device : participant->getDevices()) {
			// Queue the message for all devices except the one that sent it
			if (msg->fromAddr != device->getAddress()){
				queueMessage(msg, device->getAddress());
			}
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
	queuedMessages[uri].push(msg);
}

/* The removal of participant device is done only when such device disapears from registration database, ie when a device unregisters explicitely
 * or removed by an administrator.
 */
void ServerGroupChatRoomPrivate::removeParticipantDevice (const shared_ptr<Participant> &participant, const IdentityAddress &deviceAddress) {
	L_Q();
	L_Q_T(LocalConference, qConference);
	shared_ptr<Participant> participantCopy = participant; // make a copy of the shared_ptr because the participant may be removed by setParticipantDeviceState().
	lInfo() << q << " device " << deviceAddress << " is removed because it is has unregistered.";
	auto participantDevice = participant->findDevice(deviceAddress);
	if (!participantDevice){
		lError() << q << " device " << deviceAddress << " is removed, but we can't find it in this chatroom.";
		return;
	}
	// Notify to everyone the retirement of this device.
	auto deviceEvent = qConference->notifyParticipantDeviceRemoved(time(nullptr), false, participant->getAddress(), deviceAddress);
	q->getCore()->getPrivate()->mainDb->addEvent(deviceEvent);
	// First set it as left, so that it may eventually trigger the destruction of the chatroom if no device are present for any participant.
	setParticipantDeviceState(participantDevice, ParticipantDevice::State::Left);
	participantCopy->removeDevice(deviceAddress);
}

// -----------------------------------------------------------------------------

bool ServerGroupChatRoomPrivate::allDevicesLeft(const std::shared_ptr<Participant> &participant){
	bool allDevicesLeft = true;

	for (const auto &device : participant->getDevices()) {
		if (device->getState() != ParticipantDevice::State::Left) {
			allDevicesLeft = false;
			break;
		}
	}
	return allDevicesLeft;
}

void ServerGroupChatRoomPrivate::onParticipantDeviceLeft (const std::shared_ptr<ParticipantDevice> &device) {
	L_Q();

	lInfo() << q << ": Participant device '" << device->getAddress().asString() << "' left";

	if (! (capabilities & ServerGroupChatRoom::Capabilities::OneToOne) ){
		shared_ptr<Participant> participant = const_pointer_cast<Participant>(device->getParticipant()->getSharedFromThis());
		if (allDevicesLeft(participant) && findAuthorizedParticipant(participant->getAddress()) == nullptr) {
			lInfo() << q << ": Participant '" << participant->getAddress().asString() << "'removed and last device left, unsubscribing";
			unSubscribeRegistrationForParticipant(participant->getAddress());
		}
	}
	
	//device left, we no longuer need to receive subscription info from it
	if (device->isSubscribedToConferenceEventPackage()) {
		lError() << q << " still subscription pending for [" << device << "], terminating in emergency";
		//try to terminate subscription if any, but do not wait for anser.
		LinphoneEventCbs *cbs = linphone_event_get_callbacks(device->getConferenceSubscribeEvent());
		linphone_event_cbs_set_user_data(cbs, nullptr);
		linphone_event_cbs_set_notify_response(cbs, nullptr);
		linphone_event_terminate(device->getConferenceSubscribeEvent());
	}
	
	/* if all devices of participants are left we'll delete the chatroom*/
	bool allLeft = true;
	for (const auto &participant : q->LocalConference::getParticipants()){
		if (!allDevicesLeft(participant)){
			allLeft = false;
			break;
		}
	}
	if (allLeft){
		lInfo() << q << ": No participant left, deleting the chat room";
		requestDeletion();
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
	auto device = q->findParticipantDevice(session);
	if (!device) {
		lInfo() << q << "onCallSessionStateChanged on unknown device (maybe not yet).";
		return;
	}
	switch(newState){
		case CallSession::State::Connected:
			if (device->getState() == ParticipantDevice::State::Leaving)
				byeDevice(device);
		break;
		case CallSession::State::End:
			if (device->getState() == ParticipantDevice::State::Present){
				lInfo() << q << device->getParticipant()->getAddress().asString() << " is leaving the chatroom.";
				/* Unlike normal group chatrooms, a participant can never leave a one to one chatroom.
				 * In real life you cannot prevent someone to send you a message.
				 * The receiving of BYE is then interpreted as a termination of the SIP session specific for the device.
				 */
				if (! (capabilities & ServerGroupChatRoom::Capabilities::OneToOne) ){
					// Participant leaves the chat room on its own by sending a BYE.
					// Set it the Leaving state first, so that q->removeParticipant() does nothing with it.
					setParticipantDeviceState(device, ParticipantDevice::State::Leaving);
					// Remove participant will change other devices to Leaving state.
					q->removeParticipant(device->getParticipant()->getSharedFromThis());
					// But since we received its BYE, it is actually already left.
				}
				setParticipantDeviceState(device, ParticipantDevice::State::Left);
			}
		break;
		case CallSession::State::Released:
			/* Handle the case of participant we've send a BYE. */
			if (device->getState() == ParticipantDevice::State::Leaving && session->getPreviousState() == CallSession::State::End){
				if (session->getReason() == LinphoneReasonNone){
					/* We've received a 200 Ok for our BYE, so it is assumed to be left. */
					setParticipantDeviceState(device, ParticipantDevice::State::Left);
				}else if (session->getReason() == LinphoneReasonNoMatch){
					/* Our current session was lost, but the device is currently reachable, so retry to send the BYE now. */
					byeDevice(device);
				}
			}
		break;
		case CallSession::State::UpdatedByRemote:
		{
			shared_ptr<Participant> participant = findAuthorizedParticipant(session);
			if (participant && participant->isAdmin()) {
				/* The only thing that a participant can change with re-INVITE is the subject. */
				handleSubjectChange(session->getPrivate()->getOp());
			}
		}
		break;
		default:
		break;
	}
}

void ServerGroupChatRoomPrivate::onCallSessionSetReleased (const shared_ptr<CallSession> &session) {
	L_Q();
	shared_ptr<ParticipantDevice> device = q->findParticipantDevice(session);
	if (device)
		device->setSession(nullptr);
}

void ServerGroupChatRoomPrivate::onAckReceived (const std::shared_ptr<CallSession> &session, LinphoneHeaders *headers){
	L_Q();
	if (joiningPendingAfterCreation && mInitiatorDevice && mInitiatorDevice->getSession() == session){
		lInfo() << q << " got ACK from initiator of the chatroom, things can start now.";
		joiningPendingAfterCreation = false;
		updateParticipantsSessions();
	}
}

// =============================================================================

ServerGroupChatRoom::ServerGroupChatRoom (const shared_ptr<Core> &core, SalCallOp *op)
	: ChatRoom(*new ServerGroupChatRoomPrivate, core, ChatRoomParams::getDefaults(core)),
LocalConference(getCore(), IdentityAddress(linphone_proxy_config_get_conference_factory_uri(linphone_core_get_default_proxy_config(core->getCCore()))), nullptr, ConferenceParams::create()) {
	L_D();

	LocalConference::setSubject(op->getSubject());

	this->conferenceId = ConferenceId();

	const char *oneToOneChatRoomStr = sal_custom_header_find(op->getRecvCustomHeaders(), "One-To-One-Chat-Room");
	if (oneToOneChatRoomStr && (strcmp(oneToOneChatRoomStr, "true") == 0))
		d->capabilities |= ServerGroupChatRoom::Capabilities::OneToOne;
	string endToEndEncrypted = L_C_TO_STRING(sal_custom_header_find(op->getRecvCustomHeaders(), "End-To-End-Encrypted"));
	if (endToEndEncrypted == "true")
		d->capabilities |= ServerGroupChatRoom::Capabilities::Encrypted;

	d->params = ChatRoomParams::fromCapabilities(d->capabilities);

	shared_ptr<CallSession> session = getMe()->createSession(*this, nullptr, false, d);
	session->configure(LinphoneCallIncoming, nullptr, op, Address(op->getFrom()), Address(op->getTo()));
}

ServerGroupChatRoom::ServerGroupChatRoom (
	const shared_ptr<Core> &core,
	const IdentityAddress &peerAddress,
	AbstractChatRoom::CapabilitiesMask capabilities,
	const shared_ptr<ChatRoomParams> &params,
	const string &subject,
	list<shared_ptr<Participant>> &&participants,
	unsigned int lastNotifyId
) : ChatRoom(*new ServerGroupChatRoomPrivate(capabilities), core, params),
LocalConference(getCore(), peerAddress, nullptr, ConferenceParams::create()) {
	this->subject = subject;
	this->participants = move(participants);
	this->conferenceAddress = peerAddress;
	this->lastNotify = lastNotifyId;
	this->conferenceId = ConferenceId(peerAddress, peerAddress);
	getCore()->getPrivate()->localListEventHandler->addHandler(eventHandler.get());
}

ServerGroupChatRoom::~ServerGroupChatRoom () {
	lInfo() << this << " destroyed.";
	if (conferenceId.isValid()){
		try {
			if (getCore()->getPrivate()->localListEventHandler)
				getCore()->getPrivate()->localListEventHandler->removeHandler(eventHandler.get());
		} catch (const bad_weak_ptr &) {
			// Unable to unregister listener here. Core is destroyed and the listener doesn't exist.
		}
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

bool ServerGroupChatRoom::addParticipant (const IdentityAddress &addr, const CallSessionParams *params, bool hasMedia) {
	L_D();

	if (addr.hasGruu()){
		lInfo() << this << ": Not adding participant '" << addr.asString() << "' because it is a gruu address.";
		return false;
	}

	if (d->findAuthorizedParticipant(addr)) {
		lInfo() << this << ": Not adding participant '" << addr.asString() << "' because it is already a participant";
		return false;
	}

	shared_ptr<Participant> participant = findParticipant(addr);

	if (participant == nullptr && (d->capabilities & ServerGroupChatRoom::Capabilities::OneToOne) && getParticipantCount() == 2) {
		lInfo() << this << ": Not adding participant '" << addr.asString() << "' because this OneToOne chat room already has 2 participants";
		return false;
	}

	/* Handle the case where a participant is removed then re-added to chat room. In such case, until all devices have left the chatroom,
	 * the participant is still referenced in the chatroom, with devices either left or leaving.
	 * Furthermore, registration subscription is still active, so we don't need to wait for a notify, and we can instead
	 * proceed immediately with the INVITE of its devices.
	 */
	if (participant){
		d->resumeParticipant(participant);
	}else{
		lInfo() << this << ": Requested to add participant '" << addr.asString() << "', checking capabilities first.";
		list<IdentityAddress> participantsList;
		participantsList.push_back(addr);
		d->subscribeRegistrationForParticipants(participantsList, true);
	}
	return true;
}

bool ServerGroupChatRoom::addParticipants (const list<IdentityAddress> &addresses, const CallSessionParams *params, bool hasMedia) {
	return LocalConference::addParticipants(addresses, params, hasMedia);
}

shared_ptr<Participant> ServerGroupChatRoom::findParticipant (const IdentityAddress &participantAddress) const {
	return LocalConference::findParticipant(participantAddress);
}

const ConferenceAddress &ServerGroupChatRoom::getConferenceAddress () const {
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
	return d->authorizedParticipants;
}

const string &ServerGroupChatRoom::getSubject () const {
	return LocalConference::getSubject();
}

void ServerGroupChatRoom::join () {}

void ServerGroupChatRoom::leave () {}

void ServerGroupChatRoom::onFirstNotifyReceived (const IdentityAddress &addr) {
	L_D();
	for (const auto &participant : getParticipants()) {
		for (const auto &device : participant->getDevices()) {
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
	if (participant->isAdmin()) setParticipantAdminStatus(participant, false);
	d->removeParticipant(participant);
	return true;
}

bool ServerGroupChatRoom::removeParticipants (const list<shared_ptr<Participant>> &participants) {
	return LocalConference::removeParticipants(participants);
}

void ServerGroupChatRoom::setParticipantAdminStatus (const shared_ptr<Participant> &participant, bool isAdmin) {
	L_D();
	if (isAdmin != participant->isAdmin()) {
		participant->setAdmin(isAdmin);
		if (!(d->capabilities & ServerGroupChatRoom::Capabilities::OneToOne)) {
			shared_ptr<ConferenceParticipantEvent> event = notifyParticipantSetAdmin(time(nullptr), false, participant->getAddress(), participant->isAdmin());
			getCore()->getPrivate()->mainDb->addEvent(event);
		}
	}
}

void ServerGroupChatRoom::setSubject (const string &subject) {
	if (subject != getSubject()) {
		LocalConference::setSubject(subject);
		shared_ptr<ConferenceSubjectEvent> event = notifySubjectChanged(time(nullptr), false, getSubject());
		getCore()->getPrivate()->mainDb->addEvent(event);
	}
}

// -----------------------------------------------------------------------------

ostream &operator<< (ostream &stream, const ServerGroupChatRoom *chatRoom) {
	// TODO: Is conference ID needed to be stored in both remote conference and chat room base classes?
	return stream << "ServerGroupChatRoom [" << chatRoom->getConferenceId().getPeerAddress().asString() << "]";
}

LINPHONE_END_NAMESPACE
