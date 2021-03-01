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

ParticipantDeviceIdentity::ParticipantDeviceIdentity (const Address &address, const string &name) : mDeviceAddress(address), mDeviceName(name) {
	mDeviceAddressCache = linphone_address_new(address.asString().c_str());
}

void ParticipantDeviceIdentity::setCapabilityDescriptor(const string &capabilities){
	mCapabilityDescriptor = capabilities;
}

ParticipantDeviceIdentity::~ParticipantDeviceIdentity(){
	linphone_address_destroy(mDeviceAddressCache);
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

shared_ptr<Participant> ServerGroupChatRoomPrivate::addParticipant (const IdentityAddress &addr) {
	L_Q();

	shared_ptr<Participant> participant = q->findCachedParticipant(addr);
	if (!participant) {
		participant = Participant::create(q->getConference().get(),addr);
		q->cachedParticipants.push_back(participant);
	}
	/* Case of participant that is still referenced in the chatroom, but no longer authorized because it has been removed
	 * previously OR a totally new participant. */
	if (q->findParticipant(addr) == nullptr){
		q->getConference()->participants.push_back(participant);
		shared_ptr<ConferenceParticipantEvent> event = q->getConference()->notifyParticipantAdded(time(nullptr), false, participant);
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

	// Do not change state of participants if the core is shutting down.
	// If a participant is about to leave and its call session state is End, it will be released during shutdown event though the participant may not be notified yet as it is offline
	if (linphone_core_get_global_state(q->getCore()->getCCore()) ==  LinphoneGlobalOn) {
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
	
	/* Assign a random conference address to this new chatroom, with domain
	 * set according to the proxy config used to receive the INVITE.
	 */
	
	LinphoneProxyConfig *cfg = session->getPrivate()->getDestProxy();
	if (!cfg) cfg = linphone_core_get_default_proxy_config(L_GET_C_BACK_PTR(q->getCore()));
	LinphoneAddress *addr = linphone_address_clone(linphone_proxy_config_get_identity_address(cfg));
	
	char token[17];
	ostringstream os;
	
	belle_sip_random_token(token, sizeof(token));
	os << "chatroom-" << token;
	linphone_address_set_username(addr, os.str().c_str());
	q->getConference()->confParams->setConferenceAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(addr));
	linphone_address_destroy(addr);

	/* Application (conference server) callback to register the name.
	 * In response, the conference server will call setConferenceAddress().
	 * It has the possibility to change the conference address.
	 */
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
	
	shared_ptr<ChatRoom> chatRoom(q->getSharedFromThis()); // Take a shared_ptr here, because onChatRoomDeleteRequested() may destroy our chatroom otherwise.
	chatRoomListener->onChatRoomDeleteRequested(chatRoom);
	/*
	 * The LinphoneChatRoom (C object) is also built when the ServerGroupChatRoom is created upon receiving an INVITE. The "wrap mode" is set to external
	 * in this case. Not really a good choice.
	 * However, when the chatroom is created while loading data from mysql database, the C object is not created and then the "wrap mode" will be set to
	 * internal.
	 * The best solution would have been to change the creation of server group chat room when receiving an INVITE.
	 * To minimize risks, I prefer to create a dirty hack here. The C wrapping will be moved to HybridObject soon, with which no "external" or "internal" 
	 * concepts exists (because we don't care about it). So that explicit unref() shall be removed after moving to HybridObject<>.
	 * The destruction is defered to next main loop iteration, in order to make the self-destruction outside of the call stack that leaded to it.
	 * TODO: remove this after switching chatrooms to HybridObject.
	 */
	if (needsUnref){
		LinphoneChatRoom * cChatRoom = L_GET_C_BACK_PTR(chatRoom);
		/* If the chatroom was created as "external" mode, explicitely unref the C object to destroy it.*/
		if (cChatRoom){
			q->getCore()->doLater([cChatRoom](){
				linphone_chat_room_unref(cChatRoom);
			});
		}
	}
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
		shared_ptr<ConferenceParticipantDeviceEvent> deviceEvent = q->getConference()->notifyParticipantDeviceAdded(time(nullptr), false, participant, device);
		q->getCore()->getPrivate()->mainDb->addEvent(deviceEvent);
		if (!(capabilities & ServerGroupChatRoom::Capabilities::OneToOne)) {
			shared_ptr<ConferenceParticipantEvent> adminEvent = q->getConference()->notifyParticipantSetAdmin(time(nullptr), false, participant, true);
			q->getCore()->getPrivate()->mainDb->addEvent(adminEvent);
		}
	} else {
		// INVITE coming from an invited participant
		participant = q->findCachedParticipant(IdentityAddress(op->getFrom()));
		if (!participant) {
			lError() << q << ": Declining INVITE coming from someone that is not a participant";
			op->decline(SalReasonDeclined, "");
			return;
		}
		// In protocol < 1.1, one to one chatroom can be resurected by a participant, but the participant actually never leaves from server's standpoint.
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
		session = participant->createSession(*q->getConference().get(), &params, false, this);
		session->configure(LinphoneCallIncoming, nullptr, op, participant->getAddress(), Address(op->getTo()));
		session->startIncomingNotification(false);
		Address addr = q->getConference()->getConferenceAddress();
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

	auto participant = q->findCachedParticipant(Address(op->getFrom()));
	if (!participant){
		lError() << q << " bug - " << op->getFrom() << " is not a participant.";
		op->decline(SalReasonInternalError, "");
		return;
	}

	IdentityAddress confAddr(q->getConference()->getConferenceAddress());

	lInfo() << q << " is re-joined by " << participant->getAddress();
	Address addr(confAddr);
	addr.setParam("isfocus");
	shared_ptr<Participant> me = q->getMe();
	shared_ptr<CallSession> session = me->createSession(*q->getConference().get(), nullptr, false, this);
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
					// Happens only with protocol < 1.1
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

void ServerGroupChatRoomPrivate::removeParticipant (const shared_ptr<Participant> &participant) {
	L_Q();

	for (const auto &device : participant->getDevices()) {
		if ((device->getState() == ParticipantDevice::State::Leaving)
			|| (device->getState() == ParticipantDevice::State::Left)
		)
			continue;
		setParticipantDeviceState(device, ParticipantDevice::State::ScheduledForLeaving);
		updateParticipantDeviceSession(device);
	}

	for (const auto &p : q->getParticipants()) {
		if (participant->getAddress() == p->getAddress()) {
			lInfo() << q <<" 'participant ' "<< p->getAddress() <<" no more authorized'";
			q->getConference()->removeParticipant(p);
			break;
		}
	}

	queuedMessages.erase(participant->getAddress().asString());

	shared_ptr<ConferenceParticipantEvent> event = q->getConference()->notifyParticipantRemoved(time(nullptr), false, participant);
	q->getCore()->getPrivate()->mainDb->addConferenceParticipantEventToDb(event);

	if (!isAdminLeft())
		designateAdmin();
}

void ServerGroupChatRoomPrivate::subscriptionStateChanged (LinphoneEvent *event, LinphoneSubscriptionState state) {
	L_Q();
	static_pointer_cast<LocalConference>(q->getConference())->eventHandler->subscriptionStateChanged(event, state);
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
	L_Q();
	// Check that the message is coming from a participant of the chat room
	IdentityAddress fromAddr(op->getFrom());
	if (!q->findParticipant(fromAddr)) {
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

// TODO: Move to conference.cpp?
// If conference address is not valid, then the conference fails to create
void ServerGroupChatRoomPrivate::setConferenceAddress (const ConferenceAddress &conferenceAddress) {
	L_Q();

	if (!conferenceAddress.isValid()) {
		shared_ptr<CallSession> session = q->getMe()->getSession();
		LinphoneErrorInfo *ei = linphone_error_info_new();
		linphone_error_info_set(ei, "SIP", LinphoneReasonUnknown, 500, "Server internal error", NULL);
		session->decline(ei);
		linphone_error_info_unref(ei);
		q->setState(ConferenceInterface::State::CreationFailed);
		return;
	}

	if (q->getState() != ConferenceInterface::State::Instantiated) {
		lError() << "Cannot set the conference address of the ServerGroupChatRoom in state " << q->getState();
		return;
	}
	q->getConference()->confParams->setConferenceAddress(conferenceAddress);
	lInfo() << "The ServerGroupChatRoom has been given the address " << conferenceAddress.asString() << ", now finalizing its creation";
	finalizeCreation();
}


void ServerGroupChatRoomPrivate::updateParticipantDevices(const IdentityAddress &participantAddress, const list<shared_ptr<ParticipantDeviceIdentity>> &devices){
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
		participant = q->findCachedParticipant(participantAddress);
	}

	if (!participant){
		lError() << q << " participant devices updated for unknown participant, ignored.";
		return;
	}
	lInfo() << q << ": Setting " << devices.size() << " participant device(s) for " << participantAddress.asString();

	// Remove devices that are in the chatroom but no longer in the given list
	list<shared_ptr<ParticipantDevice>> devicesToRemove;
	for (const auto &device : participant->getDevices()) {
		auto predicate = [device] (const shared_ptr<ParticipantDeviceIdentity> & deviceIdentity) {
			return device->getAddress() == deviceIdentity->getAddress();
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
	
	if (protocolVersion < CorePrivate::groupChatProtocolVersion){
		/* we need to recheck in case some devices have upgraded. */
		determineProtocolVersion();
		if (protocolVersion == CorePrivate::groupChatProtocolVersion){
			lInfo() << "It's marvellous, all devices are now up to date !";
		}
	}
}

void ServerGroupChatRoomPrivate::conclude(){
	L_Q();
	lInfo() << q << "All devices are known, the chatroom creation can be concluded.";
	shared_ptr<CallSession> session = mInitiatorDevice->getSession();
	
	if (!session){
		lError() << "ServerGroupChatRoomPrivate::conclude(): initiator's session died.";
		requestDeletion();
		return;
	}

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
			// This is necessary for protocol version < 1.1, and for backward compatibility in case these prior versions 
			// are subsequently used by device that gets joined to the chatroom.
			q->getCore()->getPrivate()->mainDb->insertOneToOneConferenceChatRoom(q->getSharedFromThis(),
				!!(capabilities & ServerGroupChatRoom::Capabilities::Encrypted) );
		}
	}
}

void ServerGroupChatRoomPrivate::setParticipantDevicesAtCreation(const IdentityAddress &participantAddress, const list<shared_ptr<ParticipantDeviceIdentity>> &devices) {

	updateParticipantDevices(participantAddress, devices);
	if (unnotifiedRegistrationSubscriptions == 0){
		conclude();
	}
}

void ServerGroupChatRoomPrivate::setParticipantDevices(const IdentityAddress &participantAddress, const list<shared_ptr<ParticipantDeviceIdentity>> &devices) {

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

void ServerGroupChatRoomPrivate::updateProtocolVersionFromDevice(const shared_ptr<ParticipantDevice> &device){
	auto protocols = Utils::parseCapabilityDescriptor(device->getCapabilityDescriptor());
	auto groupchat = protocols.find("groupchat");
	if (groupchat == protocols.end()){
		lError() << "Device " << device->getAddress().asString() << " has no groupchat capability set: " << device->getCapabilityDescriptor();
		return;
	}
	if (protocolVersion > groupchat->second){
		protocolVersion = groupchat->second;
		lWarning() << "Device " << device->getAddress().asString() << " downgrades chatroom's protocol version to " << protocolVersion;
	}
}

void ServerGroupChatRoomPrivate::determineProtocolVersion(){
	L_Q();
	protocolVersion = CorePrivate::groupChatProtocolVersion;
	for (const auto &participant : q->getParticipants()) {
		for (const auto &device : participant->getDevices()) {
			updateProtocolVersionFromDevice(device);
		}
	}
}

void ServerGroupChatRoomPrivate::addParticipantDevice (const shared_ptr<Participant> &participant, const shared_ptr<ParticipantDeviceIdentity> &deviceInfo) {
	L_Q();
	shared_ptr<ParticipantDevice> device = participant->findDevice(deviceInfo->getAddress(), false);

	if (device) {
		// Nothing to do, but set the name and capabilities because they are not known for the initiator device.
		device->setName(deviceInfo->getName());
		device->setCapabilityDescriptor(deviceInfo->getCapabilityDescriptor());
		updateProtocolVersionFromDevice(device);
	} else if (q->findParticipant(participant->getAddress())) {
		bool allDevLeft = !participant->getDevices().empty() && allDevicesLeft(participant);
		/*
		 * This is a really new device.
		 */
		device = participant->addDevice(deviceInfo->getAddress(), deviceInfo->getName());
		device->setCapabilityDescriptor(deviceInfo->getCapabilityDescriptor());
		updateProtocolVersionFromDevice(device);
		shared_ptr<ConferenceParticipantDeviceEvent> event = q->getConference()->notifyParticipantDeviceAdded(time(nullptr), false, participant, device);
		q->getCore()->getPrivate()->mainDb->addEvent(event);

		if (protocolVersion < Utils::Version(1, 1) && (capabilities & ServerGroupChatRoom::Capabilities::OneToOne) && allDevLeft){
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
	if (!(capabilities & ServerGroupChatRoom::Capabilities::OneToOne) && !q->getParticipants().empty()) {
		q->setParticipantAdminStatus(q->getConference()->participants.front(), true);
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
	ConferenceAddress confAddr(q->getConference()->getConferenceAddress());
	const ConferenceId conferenceId = ConferenceId(confAddr, confAddr);
	q->getConference()->setConferenceId(conferenceId);
	q->getCore()->getPrivate()->localListEventHandler->addHandler(static_pointer_cast<LocalConference>(q->getConference())->eventHandler.get());
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
	q->setState(ConferenceInterface::State::Created);
	chatRoomListener->onChatRoomInsertInDatabaseRequested(q->getSharedFromThis());
}

//returns true if a new session has been created, false if there is already an existing and valid one.
shared_ptr<CallSession> ServerGroupChatRoomPrivate::makeSession(const std::shared_ptr<ParticipantDevice> &device){
	L_Q();
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
		session = participant->createSession(*q->getConference().get(), &csp, false, this);
		session->configure(LinphoneCallOutgoing, nullptr, nullptr, q->getConference()->getConferenceAddress(), device->getAddress());
		device->setSession(session);
		session->initiateOutgoing();
		session->getPrivate()->createOp();
		//FIXME jehan check conference server  potential impact
		Address contactAddr(q->getConference()->getConferenceAddress());
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
	for (const auto &invitedParticipant : q->getParticipants()) {
		if (invitedParticipant != participant)
			addressesList.push_back(invitedParticipant->getAddress());
	}
	if (addressesList.empty()){
		// Having an empty participant list shall never happen, but should this happen don't spread the bug to clients.
		lError() << q << ": empty participant list, this should never happen, INVITE not sent.";
		return;
	}

	Content content;
	content.setBodyFromUtf8(q->getConference()->getResourceLists(addressesList));
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

	shared_ptr<Participant> participant = q->findCachedParticipant(participantDevice);
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
	L_Q();
	for (const auto &participant : q->getParticipants()) {
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
	shared_ptr<Participant> participantCopy = participant; // make a copy of the shared_ptr because the participant may be removed by setParticipantDeviceState().
	lInfo() << q << " device " << deviceAddress << " is removed because it is has unregistered.";
	auto participantDevice = participant->findDevice(deviceAddress);
	if (!participantDevice){
		lError() << q << " device " << deviceAddress << " is removed, but we can't find it in this chatroom.";
		return;
	}
	// Notify to everyone the retirement of this device.
	auto deviceEvent = q->getConference()->notifyParticipantDeviceRemoved(time(nullptr), false, participant, participantDevice);
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

	unique_ptr<MainDb> &mainDb = q->getCore()->getPrivate()->mainDb;
	lInfo() << q << ": Participant device '" << device->getAddress().asString() << "' left";

	if (! (capabilities & ServerGroupChatRoom::Capabilities::OneToOne) || protocolVersion >= Utils::Version(1, 1)){
		shared_ptr<Participant> participant = const_pointer_cast<Participant>(device->getParticipant()->getSharedFromThis());
		if (allDevicesLeft(participant) && q->findParticipant(participant->getAddress()) == nullptr) {
			lInfo() << q << ": Participant '" << participant->getAddress().asString() << "'removed and last device left, unsubscribing";
			unSubscribeRegistrationForParticipant(participant->getAddress());
			mainDb->deleteChatRoomParticipant(q->getSharedFromThis(), participant->getAddress());
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
		device->setConferenceSubscribeEvent(nullptr);
	}
	
	/* if all devices of participants are left we'll delete the chatroom*/
	bool allLeft = true;
	for (const auto &participant : q->cachedParticipants){
		if (!allDevicesLeft(participant)){
			allLeft = false;
			break;
		}
	}
	if (allLeft){
		// Delete the chat room from the main DB as its termination process started and it cannot be retrieved in the future
		lInfo() << q << ": Delete chatroom from MainDB as not participant is left";
		mainDb->deleteChatRoom(q->getConferenceId());
		if (q->getState() != ConferenceInterface::State::TerminationPending) {
			q->setState(ConferenceInterface::State::TerminationPending);
		}
		q->setState(ConferenceInterface::State::Terminated);
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

std::shared_ptr<Participant> ServerGroupChatRoomPrivate::getOtherParticipant(const std::shared_ptr<Participant> someParticipant) const{
	L_Q();
	std::shared_ptr<Participant> otherParticipant;
	bool looksSane = false;
	if (capabilities & ServerGroupChatRoom::Capabilities::OneToOne ){
		for (auto & p : q->getParticipants()){
			if (p == someParticipant) looksSane = true;
			else otherParticipant = p;
		}
		/* Yes I'm paranoid.*/
		if (!looksSane){
			lError() << "getOtherParticipant() reference participant not found !";
		}else if (!otherParticipant){
			lError() << "getOtherParticipant() other participant not found !";
		}
	}else lError() << "getOtherParticipant() used for not a 1-1 chatroom.";
	return otherParticipant;
}

void ServerGroupChatRoomPrivate::onBye(const shared_ptr<ParticipantDevice> &participantLeaving){
	L_Q();
	bool shouldRemoveParticipant = true;
	
	if (capabilities & ServerGroupChatRoom::Capabilities::OneToOne ){
		if (protocolVersion < Utils::Version(1, 1)){
			/* 
			* In protocol 1.0, unlike normal group chatrooms, a participant can never leave a one to one chatroom.
			* The receiving of BYE is instead interpreted as a termination of the SIP session specific for the device.
			*/
			shouldRemoveParticipant = false;
		}else{
			/* 
			* In subsequent protocol versions, both participants of a one to one chatroom are removed,
			* which terminates the chatroom forever.
			*/
			lInfo() << "1-1 chatroom was left by one participant, removing other participant to terminate the chatroom";
			auto otherParticipant = getOtherParticipant(participantLeaving->getParticipant()->getSharedFromThis());
			if (otherParticipant) {
				q->removeParticipant(otherParticipant);
				// Do not wait to delete the chat room from the list stored in the core as the process of terminating it has already started and it cannot be restored
				lInfo() << q << ": Delete chatroom from MainDB as not participant is left";
				unique_ptr<MainDb> &mainDb = q->getCore()->getPrivate()->mainDb;
				mainDb->deleteChatRoom(q->getConferenceId());
			}
			q->setState(ConferenceInterface::State::TerminationPending);
		}
	}
	if (shouldRemoveParticipant){
		// Participant leaves the chat room on its own by sending a BYE.
		// Set it the Leaving state first, so that q->removeParticipant() does nothing with it (does not attempt to send a BYE...).
		setParticipantDeviceState(participantLeaving, ParticipantDevice::State::Leaving);
		// Remove participant will change other devices to Leaving state.
		q->removeParticipant(participantLeaving->getParticipant()->getSharedFromThis());
		// But since we received its BYE, it is actually already left.
	}
	setParticipantDeviceState(participantLeaving, ParticipantDevice::State::Left);
}

// -----------------------------------------------------------------------------

void ServerGroupChatRoomPrivate::onCallSessionStateChanged (const shared_ptr<CallSession> &session, CallSession::State newState, const string &message) {
	L_Q();
	auto device = q->findCachedParticipantDevice(session);
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
				lInfo() << q << ": "<< device->getParticipant()->getAddress().asString() << " is leaving the chatroom.";
				onBye(device);
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
			shared_ptr<Participant> participant = q->findParticipant(session);
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
	shared_ptr<ParticipantDevice> device = q->findCachedParticipantDevice(session);
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
	: ChatRoom(*new ServerGroupChatRoomPrivate, core, ChatRoomParams::getDefaults(core), make_shared<LocalConference>(core, IdentityAddress(op->getTo())
	, nullptr, ConferenceParams::create(core->getCCore()),this)) {
	L_D();

	getConference()->setSubject(op->getSubject());

	getConference()->setConferenceId(ConferenceId());

	const char *oneToOneChatRoomStr = sal_custom_header_find(op->getRecvCustomHeaders(), "One-To-One-Chat-Room");
	if (oneToOneChatRoomStr && (strcmp(oneToOneChatRoomStr, "true") == 0))
		d->capabilities |= ServerGroupChatRoom::Capabilities::OneToOne;
	string endToEndEncrypted = L_C_TO_STRING(sal_custom_header_find(op->getRecvCustomHeaders(), "End-To-End-Encrypted"));
	if (endToEndEncrypted == "true")
		d->capabilities |= ServerGroupChatRoom::Capabilities::Encrypted;

	d->params = ChatRoomParams::fromCapabilities(d->capabilities);

	shared_ptr<CallSession> session = getMe()->createSession(*getConference().get(), nullptr, false, d);
	session->configure(LinphoneCallIncoming, nullptr, op, Address(op->getFrom()), Address(op->getTo()));
	d->protocolVersion = CorePrivate::groupChatProtocolVersion;
	
	/*
	 * HACK: see comment in ServerGroupChatRoomPrivate::requestDeletion() for details.
	 * When this constructor is used, a reference to the C object is given to the core.
	 * Thus, remember that it will have to be released.
	 */
	d->needsUnref = true;
}

ServerGroupChatRoom::ServerGroupChatRoom (
	const shared_ptr<Core> &core,
	const ConferenceAddress &peerAddress,
	AbstractChatRoom::CapabilitiesMask capabilities,
	const shared_ptr<ChatRoomParams> &params,
	const string &subject,
	list<shared_ptr<Participant>> &&participants,
	unsigned int lastNotifyId
) : ChatRoom(*new ServerGroupChatRoomPrivate(capabilities), core, params, make_shared<LocalConference>(core, peerAddress, nullptr, ConferenceParams::create(core->getCCore()),this)) {
	L_D();
	cachedParticipants = move(participants);
	getConference()->setLastNotify(lastNotifyId);
	getConference()->setConferenceId(ConferenceId(peerAddress, peerAddress));
	getConference()->confParams->setConferenceAddress(peerAddress);
	getConference()->confParams->setSubject(subject);
	getConference()->confParams->enableChat(true);
	getCore()->getPrivate()->localListEventHandler->addHandler(static_pointer_cast<LocalConference>(getConference())->eventHandler.get());
	d->protocolVersion = CorePrivate::groupChatProtocolVersion;
}

ServerGroupChatRoom::~ServerGroupChatRoom () {
	lInfo() << this << " destroyed.";
	if (getConference()->getConferenceId().isValid()){
		try {
			if (getCore()->getPrivate()->localListEventHandler)
				getCore()->getPrivate()->localListEventHandler->removeHandler(static_pointer_cast<LocalConference>(getConference())->eventHandler.get());
		} catch (const bad_weak_ptr &) {
			// Unable to unregister listener here. Core is destroyed and the listener doesn't exist.
		}
	}
};

shared_ptr<Core> ServerGroupChatRoom::getCore () const {
	return ChatRoom::getCore();
}

ServerGroupChatRoom::CapabilitiesMask ServerGroupChatRoom::getCapabilities () const {
	L_D();
	return d->capabilities;
}

shared_ptr<Participant> ServerGroupChatRoom::findParticipant (const shared_ptr<const CallSession> &session) const {
	for (const auto &participant : getParticipants()) {
		shared_ptr<ParticipantDevice> device = participant->findDevice(session);
		if (device || (participant->getSession() == session))
			return participant;
	}
	lInfo() << "Unable to find participant in server group chat room " << this << " with call session " << session;
	return nullptr;
}

shared_ptr<Participant> ServerGroupChatRoom::findParticipant (const IdentityAddress &participantAddress) const {
	IdentityAddress searchedAddr(participantAddress);
	searchedAddr.setGruu("");
	for (const auto &participant : getParticipants()) {
		if (participant->getAddress() == searchedAddr)
			return participant;
	}
	lInfo() << "Unable to find participant in server group chat room " << this << " with address " << participantAddress.asString();
	return nullptr;
}

shared_ptr<Participant> ServerGroupChatRoom::findCachedParticipant (const shared_ptr<const CallSession> &session) const {
	for (const auto &participant : cachedParticipants) {
		shared_ptr<ParticipantDevice> device = participant->findDevice(session);
		if (device || (participant->getSession() == session))
			return participant;
	}
	return nullptr;
}

shared_ptr<Participant> ServerGroupChatRoom::findCachedParticipant (const IdentityAddress &participantAddress) const {
	IdentityAddress searchedAddr(participantAddress);
	searchedAddr.setGruu("");
	for (const auto &participant : cachedParticipants) {
		if (participant->getAddress() == searchedAddr)
			return participant;
	}
	return nullptr;
}

shared_ptr<ParticipantDevice> ServerGroupChatRoom::findCachedParticipantDevice (const shared_ptr<const CallSession> &session) const {

	for (const auto &participant : cachedParticipants) {
		for (const auto &device : participant->getDevices()) {
			if (device->getSession() == session)
				return device;
		}
	}

	return nullptr;
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

bool ServerGroupChatRoom::addParticipant (const IdentityAddress &participantAddress) {
	L_D();

	if (participantAddress.hasGruu()){
		lInfo() << this << ": Not adding participant '" << participantAddress.asString() << "' because it is a gruu address.";
		return false;
	}

	if (findParticipant(participantAddress)) {
		lInfo() << this << ": Not adding participant '" << participantAddress.asString() << "' because it is already a participant";
		return false;
	}

	shared_ptr<Participant> participant = findCachedParticipant(participantAddress);

	if (participant == nullptr && (d->capabilities & ServerGroupChatRoom::Capabilities::OneToOne) && getParticipantCount() == 2) {
		lInfo() << this << ": Not adding participant '" << participantAddress.asString() << "' because this OneToOne chat room already has 2 participants";
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
		lInfo() << this << ": Requested to add participant '" << participantAddress.asString() << "', checking capabilities first.";
		list<IdentityAddress> participantsList;
		participantsList.push_back(participantAddress);
		d->subscribeRegistrationForParticipants(participantsList, true);
	}
	return true;
}

const ConferenceAddress ServerGroupChatRoom::getConferenceAddress () const {
	return getConference()->getConferenceAddress();
}

shared_ptr<Participant> ServerGroupChatRoom::getMe () const {
	return getConference()->getMe();
}

int ServerGroupChatRoom::getParticipantCount () const {
	return (int)cachedParticipants.size();
}

const list<shared_ptr<Participant>> &ServerGroupChatRoom::getParticipants () const {
	return getConference()->getParticipants();
}

const string &ServerGroupChatRoom::getSubject () const {
	return getConference()->getSubject();
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

void ServerGroupChatRoom::setParticipantAdminStatus (const shared_ptr<Participant> &participant, bool isAdmin) {
	L_D();
	if (isAdmin != participant->isAdmin()) {
		participant->setAdmin(isAdmin);
		if (!(d->capabilities & ServerGroupChatRoom::Capabilities::OneToOne)) {
			shared_ptr<ConferenceParticipantEvent> event = getConference()->notifyParticipantSetAdmin(time(nullptr), false, participant, participant->isAdmin());
			getCore()->getPrivate()->mainDb->addEvent(event);
		}
	}
}

void ServerGroupChatRoom::setSubject (const string &subject) {
	if (subject != getSubject()) {
		getConference()->setSubject(subject);
		shared_ptr<ConferenceSubjectEvent> event = getConference()->notifySubjectChanged(time(nullptr), false, getSubject());
		getCore()->getPrivate()->mainDb->addEvent(event);
	}
}

void ServerGroupChatRoom::setState (ConferenceInterface::State state) {
	L_D();
	ChatRoom::setState(state);
	if (state == ConferenceInterface::State::Created) {
		// Handle transitional states (joining and leaving of participants)
		// This is needed when the chat room is loaded from its state in database
		list<IdentityAddress> participantAddresses;
		for (const auto &participant : cachedParticipants) {
			participantAddresses.emplace_back(participant->getAddress());

			if (d->capabilities & ServerGroupChatRoom::Capabilities::OneToOne){
				/**
				 * With protocol < 1.1, even if devices can BYE and get rid of their session, actually no one can leave a one to one chatroom.
				 * With protocol >= 1.1, two states are possible: both have Present/Joining/ScheduledForJoining devices
				 * OR both have Left/Leaving/ScheduledForLeaving devices.
				 * Since we don't have the protocol version at this stage (it will be known after receiving register information),
				 * it is not a problem to push the two participants in the authorized list even if they are in the process of leaving.
				 */
				getConference()->participants.push_back(participant);
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
					getConference()->participants.push_back(participant);
				}
			}
		}
		d->updateParticipantsSessions();
		// Subscribe to the registration events from the proxy
		d->subscribeRegistrationForParticipants(participantAddresses, false);
	}
}

void ServerGroupChatRoom::subscribeReceived (LinphoneEvent *event) {
	static_pointer_cast<LocalConference>(getConference())->subscribeReceived(event);
}

// -----------------------------------------------------------------------------

ostream &operator<< (ostream &stream, const ServerGroupChatRoom *chatRoom) {
	// TODO: Is conference ID needed to be stored in both remote conference and chat room base classes?
	return stream << "ServerGroupChatRoom [" << chatRoom->getConferenceId().getPeerAddress().asString() << "]";
}

LINPHONE_END_NAMESPACE
