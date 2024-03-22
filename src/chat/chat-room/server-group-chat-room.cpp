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
#include "chat/modifier/cpim-chat-message-modifier.h"
#include "conference/handlers/local-conference-event-handler.h"
#include "conference/handlers/local-conference-list-event-handler.h"
#include "conference/local-conference.h"
#include "conference/participant-info.h"
#include "conference/participant.h"
#include "conference/session/call-session-p.h"
#include "content/content-disposition.h"
#include "content/content-type.h"
#include "core/core-p.h"
#include "event-log/events.h"
#include "factory/factory.h"
#include "logger/logger.h"
#include "sal/refer-op.h"
#include "server-group-chat-room-p.h"
#include "sip-tools/sip-headers.h"

#include "linphone/wrapper_utils.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

ParticipantDeviceIdentity::ParticipantDeviceIdentity(const std::shared_ptr<Address> &address, const string &name)
    : mDeviceAddress(address), mDeviceName(name) {
}

void ParticipantDeviceIdentity::setCapabilityDescriptor(const string &capabilities) {
	setCapabilityDescriptor(Utils::toList(bctoolbox::Utils::split(capabilities, ",")));
}

void ParticipantDeviceIdentity::setCapabilityDescriptor(const std::list<std::string> &capabilities) {
	for (const auto &spec : capabilities) {
		const auto nameVersion = Core::getSpecNameVersion(spec);
		const auto &name = nameVersion.first;
		const auto &version = nameVersion.second;
		mCapabilityDescriptor[name] = version;
	}
}

const std::string &ParticipantDeviceIdentity::getCapabilityDescriptor() const {
	const std::list<std::string> capabilityDescriptor = getCapabilityDescriptorList();
	mCapabilityDescriptorString = Utils::join(Utils::toVector(capabilityDescriptor), ",");
	return mCapabilityDescriptorString;
}

const std::list<std::string> ParticipantDeviceIdentity::getCapabilityDescriptorList() const {
	std::list<std::string> specsList;
	for (const auto &nameVersion : mCapabilityDescriptor) {
		const auto &name = nameVersion.first;
		const auto &version = nameVersion.second;
		std::string specNameVersion;
		specNameVersion += name;
		if (!version.empty()) {
			specNameVersion += "/";
			specNameVersion += version;
		}
		specsList.push_back(specNameVersion);
	}

	return specsList;
}

ParticipantDeviceIdentity::~ParticipantDeviceIdentity() {
}

// -----------------------------------------------------------------------------

#define CALL_CHAT_ROOM_CBS(cr, cbName, functionName, ...)                                                              \
	do {                                                                                                               \
		bctbx_list_t *callbacksCopy = bctbx_list_copy_with_data(linphone_chat_room_get_callbacks_list(cr),             \
		                                                        (bctbx_list_copy_func)belle_sip_object_ref);           \
		linphone_chat_room_ref(cr);                                                                                    \
		for (bctbx_list_t *it = callbacksCopy; it; it = bctbx_list_next(it)) {                                         \
			LinphoneChatRoomCbs *cbs = static_cast<LinphoneChatRoomCbs *>(bctbx_list_get_data(it));                    \
			linphone_chat_room_set_current_callbacks(cr, cbs);                                                         \
			LinphoneChatRoomCbs##cbName##Cb cb = linphone_chat_room_cbs_get_##functionName(cbs);                       \
			if (cb) cb(__VA_ARGS__);                                                                                   \
		}                                                                                                              \
		linphone_chat_room_set_current_callbacks(cr, nullptr);                                                         \
		linphone_chat_room_unref(cr);                                                                                  \
		bctbx_list_free_with_data(callbacksCopy, (bctbx_list_free_func)belle_sip_object_unref);                        \
	} while (0)

shared_ptr<Participant> ServerGroupChatRoomPrivate::addParticipant(const std::shared_ptr<Address> &addr) {
	L_Q();

	shared_ptr<Participant> participant = q->findCachedParticipant(addr);
	if (!participant) {
		participant = Participant::create(q->getConference().get(), addr);
		q->cachedParticipants.push_back(participant);
	}
	/* Case of participant that is still referenced in the chatroom, but no longer authorized because it has been
	 * removed previously OR a totally new participant. */
	if (q->findParticipant(addr) == nullptr) {
		q->getConference()->participants.push_back(participant);
		shared_ptr<ConferenceParticipantEvent> event =
		    q->getConference()->notifyParticipantAdded(time(nullptr), false, participant);
		q->getCore()->getPrivate()->mainDb->addEvent(event);
	}
	return participant;
}

/* This function is used to re-join devices of a participant that has left previously. Its device are still referenced
 * until they 're all left. */
void ServerGroupChatRoomPrivate::resumeParticipant(const std::shared_ptr<Participant> &participant) {
	addParticipant(participant->getAddress());
	for (auto device : participant->getDevices()) {
		switch (device->getState()) {
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

void ServerGroupChatRoomPrivate::setParticipantDeviceState(const shared_ptr<ParticipantDevice> &device,
                                                           ParticipantDevice::State state,
                                                           bool notify) {
	L_Q();

	// Do not change state of participants if the core is shutting down.
	// If a participant is about to leave and its call session state is End, it will be released during shutdown event
	// though the participant may not be notified yet as it is offline
	if (linphone_core_get_global_state(q->getCore()->getCCore()) == LinphoneGlobalOn) {
		string address(device->getAddress()->toString());
		lInfo() << q << ": Set participant device '" << address << "' state to " << state;
		device->setState(state, notify);
		q->getCore()->getPrivate()->mainDb->updateChatRoomParticipantDevice(q->getSharedFromThis(), device);
		switch (state) {
			case ParticipantDevice::State::ScheduledForLeaving:
			case ParticipantDevice::State::Leaving:
				queuedMessages.erase(address);
				break;
			case ParticipantDevice::State::Left:
				queuedMessages.erase(address);
				onParticipantDeviceLeft(device);
				break;
			case ParticipantDevice::State::Present:
				dispatchQueuedMessages();
				break;
			default:
				break;
		}
	}
}

void ServerGroupChatRoomPrivate::acceptSession(const shared_ptr<CallSession> &session) {
	if (session->getState() == CallSession::State::UpdatedByRemote) session->acceptUpdate();
	else session->accept();
}

void ServerGroupChatRoomPrivate::confirmCreation() {
	L_Q();

	shared_ptr<Participant> me = q->getMe();
	shared_ptr<CallSession> session = me->getSession();
	session->startIncomingNotification(false);

	const auto &remoteContactAddress = session->getRemoteContactAddress();
	if (remoteContactAddress->hasParam("+org.linphone.specs")) {
		const auto linphoneSpecs = remoteContactAddress->getParamValue("+org.linphone.specs");
		// The creator of the chatroom must have the capability "groupchat"
		auto protocols = Utils::parseCapabilityDescriptor(linphoneSpecs.substr(1, linphoneSpecs.size() - 2));
		auto groupchat = protocols.find("groupchat");
		if (groupchat == protocols.end()) {
			lError() << "Creator " << remoteContactAddress->asStringUriOnly()
			         << " has no groupchat capability set: " << linphoneSpecs;
			q->setState(ConferenceInterface::State::CreationFailed);
			auto errorInfo = linphone_error_info_new();
			linphone_error_info_set(errorInfo, nullptr, LinphoneReasonNotAcceptable, 488,
			                        "\"groupchat\" capability has not been found in remote contact address", nullptr);
			session->decline(errorInfo);
			linphone_error_info_unref(errorInfo);
			return;
		}
	}

	/* Assign a random conference address to this new chatroom, with domain
	 * set according to the proxy config used to receive the INVITE.
	 */

	auto destAccount = session->getPrivate()->getDestAccount();
	if (!destAccount) {
		destAccount = Account::toCpp(linphone_core_get_default_account(q->getCore()->getCCore()))->getSharedFromThis();
	}
	const auto accountParams = destAccount->getAccountParams();
	auto addr = accountParams->getIdentityAddress()->clone()->toSharedPtr();

	char token[17];
	ostringstream os;

	belle_sip_random_token(token, sizeof(token));
	os << "chatroom-" << token;
	addr->setUsername(os.str());
	q->getConference()->confParams->setConferenceAddress(addr);

	/* Application (conference server) callback to register the name.
	 * In response, the conference server will call setConferenceAddress().
	 * It has the possibility to change the conference address.
	 */
	LinphoneChatRoom *cr = L_GET_C_BACK_PTR(q);
	CALL_CHAT_ROOM_CBS(cr, ConferenceAddressGeneration, conference_address_generation, cr);
}

void ServerGroupChatRoomPrivate::requestDeletion() {
	L_Q();

	/*
	 * Our ServerGroupChatRoom is registered as listener to the numerous CallSession it manages,
	 * and is subscribing for registration events.
	 * It is time to remove this listener and unsubscribe, because we are going to disapear and we don't want
	 * to be notified anymore of anything.
	 */
	for (auto participant : q->getParticipants()) {
		unSubscribeRegistrationForParticipant(participant->getAddress());
		for (auto devices : participant->getDevices()) {
			auto session = devices->getSession();
			if (session) session->setListener(nullptr);
		}
	}
	if (!registrationSubscriptions.empty()) {
		lError() << q << " still " << registrationSubscriptions.size()
		         << " registration subscriptions pending while deletion is requested.";
	}

	shared_ptr<ChatRoom> chatRoom(
	    q->getSharedFromThis()); // Take a shared_ptr here, because onChatRoomDeleteRequested() may destroy our chatroom
	                             // otherwise.
	chatRoomListener->onChatRoomDeleteRequested(chatRoom);
	/*
	 * The LinphoneChatRoom (C object) is also built when the ServerGroupChatRoom is created upon receiving an INVITE.
	 * The "wrap mode" is set to external in this case. Not really a good choice. However, when the chatroom is created
	 * while loading data from mysql database, the C object is not created and then the "wrap mode" will be set to
	 * internal.
	 * The best solution would have been to change the creation of server group chat room when receiving an INVITE.
	 * To minimize risks, I prefer to create a dirty hack here. The C wrapping will be moved to HybridObject soon, with
	 * which no "external" or "internal" concepts exists (because we don't care about it). So that explicit unref()
	 * shall be removed after moving to HybridObject<>. The destruction is defered to next main loop iteration, in order
	 * to make the self-destruction outside of the call stack that leaded to it.
	 * TODO: remove this after switching chatrooms to HybridObject.
	 */

	LinphoneChatRoom *cChatRoom = L_GET_C_BACK_PTR(chatRoom);
	/* If the chatroom was created as "external" mode, explicitely unref the C object to destroy it.*/
	if (cChatRoom && Wrapper::isOwnedByC<ServerGroupChatRoom>(cChatRoom)) {
		q->getCore()->doLater([cChatRoom]() { linphone_chat_room_unref(cChatRoom); });
	}
}

/*
 * We go in this method in two cases:
 * - when the client who created the conference INVITEs the conference address that was specified
 *   in the redirect response consecutive to the first INVITE that was made to the factory uri.
 *   In this case, joiningPendingAfterCreation is set to true.
 * - when an already joined participant device reconnects for whatever reason.
 */
void ServerGroupChatRoomPrivate::confirmJoining(SalCallOp *op) {
	L_Q();
	shared_ptr<Participant> participant;

	std::shared_ptr<Address> contactAddr = Address::create(op->getRemoteContact());
	if (contactAddr->getUriParamValue("gr").empty()) {
		lError() << q << ": Declining INVITE because the contact does not have a 'gr' uri parameter ["
		         << contactAddr->toString() << "]";
		op->decline(SalReasonDeclined, "");
		joiningPendingAfterCreation = false;
		return;
	}

	std::shared_ptr<Address> gruu(contactAddr);
	shared_ptr<ParticipantDevice> device;
	shared_ptr<CallSession> deviceSession;
	if (joiningPendingAfterCreation) {
		// Check if the participant is already there, this INVITE may come from an unknown device of an already present
		// participant
		participant = addParticipant(Address::create(op->getFrom()));
		participant->setAdmin(true);
		device = participant->addDevice(gruu);
		deviceSession = device->getSession();
		mInitiatorDevice = device;

		/*Since the initiator of the chatroom has not yet subscribed at this stage, this won't generate NOTIFY, the
		 * events will be queued. */
		shared_ptr<ConferenceParticipantDeviceEvent> deviceEvent =
		    q->getConference()->notifyParticipantDeviceAdded(time(nullptr), false, participant, device);
		q->getCore()->getPrivate()->mainDb->addEvent(deviceEvent);
		if (!(capabilities & ServerGroupChatRoom::Capabilities::OneToOne)) {
			shared_ptr<ConferenceParticipantEvent> adminEvent =
			    q->getConference()->notifyParticipantSetAdmin(time(nullptr), false, participant, true);
			q->getCore()->getPrivate()->mainDb->addEvent(adminEvent);
		}
	} else {
		// INVITE coming from an invited participant
		participant = q->findCachedParticipant(Address::create(op->getFrom()));
		if (!participant) {
			lError() << q << ": Declining INVITE coming from someone that is not a participant";
			op->decline(SalReasonDeclined, "");
			return;
		}
		// In protocol < 1.1, one to one chatroom can be resurected by a participant, but the participant actually never
		// leaves from server's standpoint.
		if (!(capabilities & ServerGroupChatRoom::Capabilities::OneToOne) &&
		    op->isContentInRemote(ContentType::ResourceLists)) {
			lError() << q << "Receiving ressource list body while not in creation step.";
			op->decline(SalReasonNotAcceptable);
			return;
		}
		device = participant->addDevice(gruu);
		if (capabilities & ServerGroupChatRoom::Capabilities::OneToOne) {
			if (device->getState() == ParticipantDevice::State::Left) {
				lInfo() << q << " " << gruu << " is reconnected to the one to one chatroom.";
				setParticipantDeviceState(device, ParticipantDevice::State::Joining);
			}
			participant->setAdmin(true);
		}
		deviceSession = device->getSession();
	}

	shared_ptr<CallSession> newDeviceSession = deviceSession;
	auto rejectSession = false;
	if (!deviceSession || (deviceSession->getPrivate()->getOp() != op)) {
		CallSessionParams params;
		// params.addCustomContactParameter("isfocus");
		newDeviceSession = participant->createSession(*q->getConference().get(), &params, false, this);
		newDeviceSession->configure(LinphoneCallIncoming, nullptr, op, participant->getAddress(),
		                            Address::create(op->getTo()));
		newDeviceSession->startIncomingNotification(false);
		std::shared_ptr<Address> addr = q->getConferenceAddress()->clone()->toSharedPtr();
		addr->setParam("isfocus");
		// to force is focus to be added
		newDeviceSession->getPrivate()->getOp()->setContactAddress(addr->getImpl());
		// Reject a session if there is already an active outgoing session and the participant device is trying to leave
		// the conference
		rejectSession = deviceSession && (deviceSession->getDirection() == LinphoneCallOutgoing) &&
		                ParticipantDevice::isLeavingState(device->getState());

		if (!rejectSession) {
			device->setSession(newDeviceSession);
		}
	}

	// Changes are only allowed from admin participants
	if (participant->isAdmin()) {
		if (joiningPendingAfterCreation) {
			if (!initializeParticipants(participant, op)) {
				op->decline(SalReasonNotAcceptable, "");
				requestDeletion();
			}
			/* we don't accept the session yet: initializeParticipants() has launched queries for device information
			 * that will later populate the chatroom*/
		} else if (rejectSession) {
			lInfo() << "Reject session because admin device " << *device->getAddress()
			        << " has already an established session";
			op->decline(SalReasonDeclined, "");
		} else {
			/* after creation, only changes to the subject and ephemeral settings are allowed*/
			handleSubjectChange(op);
			handleEphemeralSettingsChange(newDeviceSession);
			acceptSession(newDeviceSession);
		}
	} else {
		if (rejectSession) {
			lInfo() << "Reject session because device " << *device->getAddress()
			        << " has already an established session";
			op->decline(SalReasonDeclined, "");
		} else {
			/*it is a non-admin participant that reconnected to the chatroom*/
			acceptSession(newDeviceSession);
		}
	}
}

/* This is called only when a participant attempts to create a one to one chatroom that already exists.
 * We just redirect it to the existing chatroom uri. */
void ServerGroupChatRoomPrivate::confirmRecreation(SalCallOp *op) {
	L_Q();

	const auto from = Address::create(op->getFrom());
	const auto to = Address::create(op->getTo());
	auto participant = q->findCachedParticipant(from);
	if (!participant) {
		lError() << q << " bug - " << op->getFrom() << " is not a participant.";
		op->decline(SalReasonInternalError, "");
		return;
	}

	lInfo() << q << " is re-joined by " << participant->getAddress();
	shared_ptr<Participant> me = q->getMe();
	shared_ptr<CallSession> session = me->createSession(*q->getConference().get(), nullptr, false, this);
	session->configure(LinphoneCallIncoming, nullptr, op, from, to);
	session->startIncomingNotification(false);
	auto confAddr = *q->getConferenceAddress();
	confAddr.setParam("isfocus");
	session->redirect(confAddr);
}

void ServerGroupChatRoomPrivate::declineSession(const shared_ptr<CallSession> &session, LinphoneReason reason) {
	session->decline(reason);
}

void ServerGroupChatRoomPrivate::dispatchQueuedMessages() {
	L_Q();
	for (const auto &participant : q->getParticipants()) {
		/*
		 * Dispatch messages for each device in Present state. In a one to one chatroom, if a device
		 * is found is Left state, it must be invited first.
		 */

		for (const auto &device : participant->getDevices()) {
			string uri(device->getAddress()->toString());
			auto &msgQueue = queuedMessages[uri];

			if (!msgQueue.empty()) {
				if ((capabilities & ServerGroupChatRoom::Capabilities::OneToOne) &&
				    device->getState() == ParticipantDevice::State::Left) {
					// Happens only with protocol < 1.1
					lInfo() << "There is a message to transmit to a participant in left state in a one to one "
					           "chatroom, so inviting first.";
					inviteDevice(device);
					continue;
				}
				if (device->getState() != ParticipantDevice::State::Present) continue;
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

void ServerGroupChatRoomPrivate::removeParticipant(const shared_ptr<Participant> &participant) {
	L_Q();

	const auto &devices = participant->getDevices();
	const auto participantHasNoDevices = (devices.size() == 0);
	for (const auto &device : devices) {
		if ((device->getState() == ParticipantDevice::State::Leaving) ||
		    (device->getState() == ParticipantDevice::State::Left))
			continue;
		setParticipantDeviceState(device, ParticipantDevice::State::ScheduledForLeaving);
		updateParticipantDeviceSession(device);
	}

	for (const auto &p : q->getParticipants()) {
		if (*participant->getAddress() == *p->getAddress()) {
			lInfo() << q << " 'participant ' " << *p->getAddress() << " no more authorized'";
			q->getConference()->removeParticipant(p);
			break;
		}
	}

	queuedMessages.erase(participant->getAddress()->toString());

	unique_ptr<MainDb> &mainDb = q->getCore()->getPrivate()->mainDb;
	shared_ptr<ConferenceParticipantEvent> event =
	    q->getConference()->notifyParticipantRemoved(time(nullptr), false, participant);
	mainDb->addConferenceParticipantEventToDb(event);

	// Remove participant from the database immediately because it has no devices associated.
	// In case of registration in the future, the devices will attempt to subscribe and the conference server will reply
	// 603 Decline
	if (participantHasNoDevices) {
		lInfo() << q << ": Participant '" << *participant->getAddress()
		        << "' is immediately removed because there has been an explicit request to do it and it has no devices "
		           "associated to it, unsubscribing";
		unSubscribeRegistrationForParticipant(participant->getAddress());
		mainDb->deleteChatRoomParticipant(q->getSharedFromThis(), participant->getAddress());
	}

	if (!isAdminLeft()) designateAdmin();
}

void ServerGroupChatRoomPrivate::subscriptionStateChanged(const shared_ptr<EventSubscribe> &event,
                                                          LinphoneSubscriptionState state) {
	L_Q();
	static_pointer_cast<LocalConference>(q->getConference())->eventHandler->subscriptionStateChanged(event, state);
}

void ServerGroupChatRoomPrivate::handleSubjectChange(SalCallOp *op) {
	L_Q();
	if (sal_custom_header_find(op->getRecvCustomHeaders(), "Subject")) {
		// Handle subject change
		lInfo() << q << ": New subject \"" << op->getSubject() << "\"";
		q->setUtf8Subject(op->getSubject());
	}
}

void ServerGroupChatRoomPrivate::handleEphemeralSettingsChange(const shared_ptr<CallSession> &session) {
	if (capabilities & ServerGroupChatRoom::Capabilities::Ephemeral) {
		const auto op = session->getPrivate()->getOp();
		string ephemeralLifeTime =
		    L_C_TO_STRING(sal_custom_header_find(op->getRecvCustomHeaders(), "Ephemeral-Life-Time"));
		if (ephemeralLifeTime.empty()) {
			setEphemeralMode(AbstractChatRoom::EphemeralMode::DeviceManaged, session);
		} else {
			setEphemeralLifetime(std::stol(ephemeralLifeTime, nullptr), session);
		}
	}
}

void ServerGroupChatRoomPrivate::setEphemeralMode(AbstractChatRoom::EphemeralMode mode,
                                                  const shared_ptr<CallSession> &session) {
	L_Q();
	lInfo() << q << ": New mode is: " << mode;
	params->setEphemeralMode(mode);

	const auto device = q->getConference()->findParticipantDevice(session);
	if (device) {
		time_t creationTime = time(nullptr);
		const auto eventType = (mode == AbstractChatRoom::EphemeralMode::AdminManaged)
		                           ? EventLog::Type::ConferenceEphemeralMessageManagedByAdmin
		                           : EventLog::Type::ConferenceEphemeralMessageManagedByParticipants;
		q->getConference()->notifyEphemeralModeChanged(creationTime, false, eventType);
	} else {
		lWarning() << "Unable to find device among those of the participants that changed ephemeral message mode to "
		           << mode;
	}
}

void ServerGroupChatRoomPrivate::setEphemeralLifetime(long lifetime, const shared_ptr<CallSession> &session) {
	L_Q();
	lInfo() << q << ": New ephemeral time: " << lifetime;
	params->setEphemeralLifetime(lifetime);

	const auto device = q->getConference()->findParticipantDevice(session);
	if (device) {
		time_t creationTime = time(nullptr);
		q->getConference()->notifyEphemeralLifetimeChanged(creationTime, false, lifetime);
	} else {
		lWarning()
		    << "Unable to find device among those of the participants that changed ephemeral message lifetime to "
		    << lifetime;
	}
}

/*
 * This function setups registration subscriptions if not already there.
 * If no registration subscription is started (because they were all running already), it returns false.
 * newInvited specifies whether these participants are in the process of being invited, in which case they will be
 * automatically added to the invitedParticipants list, so that when registration info arrives, they
 * will be added.
 */
bool ServerGroupChatRoomPrivate::subscribeRegistrationForParticipants(
    const std::list<std::shared_ptr<Address>> &identAddresses, bool newInvited) {
	L_Q();

	LinphoneGlobalState gs = linphone_core_get_global_state(q->getCore()->getCCore());
	if (gs != LinphoneGlobalOn) {
		lError() << "The core is currently in state " << std::string(linphone_global_state_to_string(gs))
		         << " but registration subscriptions can only be set up when the core is in state GlobalOn";
		return false;
	}

	std::list<Address> requestedAddresses;
	bool subscriptionsPending = false;

	// Subscribe to the registration events from the proxy
	for (const auto &addr : identAddresses) {
		const auto cleanedAddr = addr->getUri();
		if (registrationSubscriptions.find(cleanedAddr.toString()) == registrationSubscriptions.end()) {
			requestedAddresses.emplace_back(cleanedAddr);
			if (newInvited) invitedParticipants.emplace_back(cleanedAddr);
			unnotifiedRegistrationSubscriptions++;
			subscriptionsPending = true;
		}
	}

	for (const auto &addr : requestedAddresses) {
		LinphoneChatRoom *cr = L_GET_C_BACK_PTR(q);
		const auto laddr = addr.toC();
		registrationSubscriptions[addr.toString()].context =
		    nullptr; // we 'll put here later a context pointer returned by the callback.
		CALL_CHAT_ROOM_CBS(cr, ParticipantRegistrationSubscriptionRequested,
		                   participant_registration_subscription_requested, cr, laddr);
	}
	return subscriptionsPending;
}

void ServerGroupChatRoomPrivate::unSubscribeRegistrationForParticipant(const std::shared_ptr<Address> &identAddress) {
	L_Q();
	auto p = registrationSubscriptions.find(identAddress->toString());
	if (p == registrationSubscriptions.end()) {
		lError() << q << " no active subscription for " << identAddress;
		return;
	}
	registrationSubscriptions.erase(p);
	removeCachedParticipant(identAddress);

	LinphoneChatRoom *cr = L_GET_C_BACK_PTR(q);
	LinphoneAddress *laddr = identAddress->toC();
	CALL_CHAT_ROOM_CBS(cr, ParticipantRegistrationUnsubscriptionRequested,
	                   participant_registration_unsubscription_requested, cr, laddr);
}

bool ServerGroupChatRoomPrivate::initializeParticipants(const shared_ptr<Participant> &initiator, SalCallOp *op) {
	handleSubjectChange(op);
	// Handle participants addition
	const auto participantList = Utils::parseResourceLists(op->getContentInRemote(ContentType::ResourceLists));
	std::list<std::shared_ptr<Address>> identAddresses;
	// DO not try to add participants with invalid address
	for (auto it = participantList.begin(); it != participantList.end(); ++it) {
		const auto &address = (*it)->getAddress();
		if (!(address->isValid())) {
			lError() << "ServerGroupChatRoomPrivate::initializeParticipants(): removing invalid address " << *address
			         << " at position " << std::distance(it, participantList.begin());
		} else {
			identAddresses.push_back(address);
		}
	}
	if (identAddresses.empty()) {
		lError() << "ServerGroupChatRoomPrivate::initializeParticipants(): empty list !";
		return false;
	}

	identAddresses.unique(); // Protection for the case where the client has sent a list with duplicates

	if (capabilities & ServerGroupChatRoom::Capabilities::OneToOne) {
		if (identAddresses.size() > 1) {
			lError() << "ServerGroupChatRoomPrivate::initializeParticipants(): chatroom is one to one but the list "
			            "contains multiple participants !";
			return false;
		}
	}
	identAddresses.push_back(initiator->getAddress());
	if (!subscribeRegistrationForParticipants(identAddresses, true)) {
		/* If we are not waiting for any registration information, then we can conclude immediately. */
		conclude();
	}
	return true;
}

void ServerGroupChatRoomPrivate::removeCachedParticipant(const std::shared_ptr<Address> &address) {
	L_Q();
	auto c = std::find_if(q->cachedParticipants.begin(), q->cachedParticipants.end(),
	                      [&address](const auto &p) { return (address->weakEqual(*p->getAddress())); });
	if (c == q->cachedParticipants.end()) {
		lDebug() << "Unable to find participant " << *address << " in the list of cached participants";
	} else {
		q->cachedParticipants.erase(c);
	}
}

// -----------------------------------------------------------------------------

LinphoneReason ServerGroupChatRoomPrivate::onSipMessageReceived(SalOp *op, const SalMessage *message) {
	L_Q();
	// Check that the message is coming from a participant of the chat room
	std::shared_ptr<Address> fromAddr = Address::create(op->getFrom());
	if (!q->findParticipant(fromAddr)) {
		return LinphoneReasonForbidden;
	}

	ContentType contentType(message->content_type);
	std::string contentBody(message->text ? message->text : "");
	// Do not check that we received a CPIM message because ciphered messages are not
	shared_ptr<Message> msg = make_shared<Message>(op->getFrom(), contentType, contentBody, op->getRecvCustomHeaders());

	queueMessage(msg);
	dispatchQueuedMessages();
	return LinphoneReasonNone;
}

// TODO: Move to conference.cpp?
// If conference address is not valid, then the conference fails to create
void ServerGroupChatRoomPrivate::setConferenceAddress(const std::shared_ptr<Address> &conferenceAddress) {
	L_Q();

	if (!conferenceAddress || !conferenceAddress->isValid()) {
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
	lInfo() << "The ServerGroupChatRoom has been given the address " << conferenceAddress->toString()
	        << ", now finalizing its creation";
	finalizeCreation();
}

void ServerGroupChatRoomPrivate::updateParticipantDevices(const std::shared_ptr<Address> &participantAddress,
                                                          const list<shared_ptr<ParticipantDeviceIdentity>> &devices) {
	L_Q();
	bool newParticipantReginfo = false;

	auto it = registrationSubscriptions.find(participantAddress->getUri().toString());

	/*
	 * For security, since registration information might come from outside, make sure that the device list we are asked
	 * to add are from a participant for which we have requested device information, ie a participant that is in the
	 * process of being added to the chatroom
	 */
	if (it == registrationSubscriptions.end()) {
		lError() << "updateParticipantDevices(): " << *participantAddress << " registration info was not requested.";
		return;
	} else {
		// Check if this registration information is for a participant in the process of being added.
		auto it = std::find(invitedParticipants.begin(), invitedParticipants.end(), *participantAddress);
		if (it != invitedParticipants.end()) {
			invitedParticipants.erase(it);
			unnotifiedRegistrationSubscriptions--;
			newParticipantReginfo = true;
		}
	}
	shared_ptr<Participant> participant;

	if (newParticipantReginfo) {
		if (!devices.empty()) {
			participant = addParticipant(participantAddress);
		} else {
			lInfo() << q << " " << participantAddress->toString() << " has no compatible devices.";
			unSubscribeRegistrationForParticipant(participantAddress);
			removeCachedParticipant(participantAddress);
			return;
		}
	} else {
		participant = q->findCachedParticipant(participantAddress);
	}

	if (!participant) {
		lError() << q << " participant devices updated for unknown participant, ignored.";
		return;
	}
	lInfo() << q << ": Setting " << devices.size() << " participant device(s) for " << *participantAddress;

	// Remove devices that are in the chatroom but no longer in the given list
	list<shared_ptr<ParticipantDevice>> devicesToRemove;
	for (const auto &device : participant->getDevices()) {
		auto predicate = [device](const shared_ptr<ParticipantDeviceIdentity> &deviceIdentity) {
			return *device->getAddress() == *deviceIdentity->getAddress();
		};
		auto it = find_if(devices.cbegin(), devices.cend(), predicate);
		if (it == devices.cend()) {
			lInfo() << q << " Device " << *device->getAddress()
			        << " is no longer registered, it will be removed from the chatroom.";
			devicesToRemove.push_back(device);
		}
	}
	// Add all the devices in the given list, if already present they will be ignored
	for (const auto &device : devices)
		addParticipantDevice(participant, device);

	// Remove all devices that are no longer existing.
	for (auto &device : devicesToRemove)
		removeParticipantDevice(participant, device->getAddress());

	if (protocolVersion < CorePrivate::groupChatProtocolVersion) {
		/* we need to recheck in case some devices have upgraded. */
		determineProtocolVersion();
		if (protocolVersion == CorePrivate::groupChatProtocolVersion) {
			lInfo() << "It's marvellous, all devices are now up to date !";
		}
	}
}

void ServerGroupChatRoomPrivate::conclude() {
	L_Q();
	lInfo() << q << " All devices are known, the chatroom creation can be concluded.";
	shared_ptr<CallSession> session = mInitiatorDevice->getSession();

	if (!session) {
		lError() << q << "ServerGroupChatRoomPrivate::conclude(): initiator's session died.";
		requestDeletion();
		return;
	}

	const auto device = q->getConference()->findParticipantDevice(session);
	if (q->getParticipants().size() < 2) {
		lError() << q << ": there are less than 2 participants in this chatroom, refusing creation.";
		declineSession(session, LinphoneReasonNotAcceptable);
		requestDeletion();
	} else if (!device || (device->getState() != ParticipantDevice::State::Joining)) {
		// We may end up here if a client successfully created a chat room but the conference server thinks it should be
		// allowed to be part of a conference. A scenario where this branch is hit is the following. The client creating
		// the chatroom registered into the server without the groupchat capability. Nonetheless, the capability is
		// added in the INVITE creating the chatroom. Upon reception of the 302 Moved Temporarely, the client will dial
		// the chatroom URI directly and the server will look for devices allowed to join the chatroom in method
		// ServerGroupChatRoomPrivate::subscribeRegistrationForParticipants(). Since "groupchat" capability was not
		// there, then the server doesn't allow to conclude the creation of the chatroom
		// -
		lError() << q
		         << ": Declining session because it looks like the device creating the chatroom is not allowed to be "
		            "part of this chatroom";
		declineSession(session, LinphoneReasonForbidden);
		requestDeletion();
	} else {
		/* Ok we are going to accept the session with 200Ok. However we want to wait for the ACK to be sure
		 * that the initiator is aware that he's now part of the conference, before we invite the others.
		 */
		acceptSession(session);
		if ((capabilities & ServerGroupChatRoom::Capabilities::OneToOne) && (q->getParticipantCount() == 2)) {
			// Insert the one-to-one chat room in Db if participants count is 2.
			// This is necessary for protocol version < 1.1, and for backward compatibility in case these prior versions
			// are subsequently used by device that gets joined to the chatroom.
			q->getCore()->getPrivate()->mainDb->insertOneToOneConferenceChatRoom(
			    q->getSharedFromThis(), !!(capabilities & ServerGroupChatRoom::Capabilities::Encrypted));
		}
	}
}

void ServerGroupChatRoomPrivate::setParticipantDevicesAtCreation(
    const std::shared_ptr<Address> &participantAddress, const list<shared_ptr<ParticipantDeviceIdentity>> &devices) {
	updateParticipantDevices(participantAddress, devices);
	if (unnotifiedRegistrationSubscriptions == 0) {
		conclude();
	}
}

void ServerGroupChatRoomPrivate::setParticipantDevices(const std::shared_ptr<Address> &participantAddress,
                                                       const list<shared_ptr<ParticipantDeviceIdentity>> &devices) {

	if (joiningPendingAfterCreation) {
		setParticipantDevicesAtCreation(participantAddress, devices);
	} else {
		updateParticipantDevices(participantAddress, devices);
		updateParticipantsSessions();
	}
}

// -----------------------------------------------------------------------------

void ServerGroupChatRoomPrivate::copyMessageHeaders(const shared_ptr<Message> &fromMessage,
                                                    const shared_ptr<ChatMessage> &toMessage) {
	static const string headersToCopy[] = {"Content-Encoding", "Expires", "Priority", XFsEventIdHeader::HeaderName};
	for (const auto &headerName : headersToCopy) {
		const char *headerValue = sal_custom_header_find(fromMessage->customHeaders, headerName.c_str());
		if (headerValue) toMessage->getPrivate()->addSalCustomHeader(headerName, headerValue);
	}
}

/*
 * This method is in charge of applying the state of a participant device to the SIP session
 */
void ServerGroupChatRoomPrivate::updateParticipantDeviceSession(const shared_ptr<ParticipantDevice> &device,
                                                                bool freshlyRegistered) {
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
		case ParticipantDevice::State::Alerting:
		case ParticipantDevice::State::Present:
		case ParticipantDevice::State::OnHold:
		case ParticipantDevice::State::MutedByFocus:
			// nothing to do
			break;
	}
}

/*
 * This method is in charge of applying to the SIP session
 * the state of all participant devices belonging to a participant.
 */
void ServerGroupChatRoomPrivate::updateParticipantsSessions() {
	L_Q();

	for (const auto &p : q->getParticipants()) {
		for (const auto &device : p->getDevices()) {
			updateParticipantDeviceSession(device);
		}
	}
}

void ServerGroupChatRoomPrivate::updateProtocolVersionFromDevice(const shared_ptr<ParticipantDevice> &device) {
	auto protocols = Utils::parseCapabilityDescriptor(device->getCapabilityDescriptor());
	auto groupchat = protocols.find("groupchat");
	if (groupchat == protocols.end()) {
		lError() << "Device " << device->getAddress()->toString()
		         << " has no groupchat capability set: " << device->getCapabilityDescriptor();
		return;
	}
	if (protocolVersion > groupchat->second) {
		protocolVersion = groupchat->second;
		lWarning() << "Device " << device->getAddress()->toString() << " downgrades chatroom's protocol version to "
		           << protocolVersion;
	}
}

void ServerGroupChatRoomPrivate::determineProtocolVersion() {
	L_Q();
	protocolVersion = CorePrivate::groupChatProtocolVersion;
	for (const auto &participant : q->getParticipants()) {
		for (const auto &device : participant->getDevices()) {
			updateProtocolVersionFromDevice(device);
		}
	}
}

void ServerGroupChatRoomPrivate::addParticipantDevice(const shared_ptr<Participant> &participant,
                                                      const shared_ptr<ParticipantDeviceIdentity> &deviceInfo) {
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
		shared_ptr<ConferenceParticipantDeviceEvent> event =
		    q->getConference()->notifyParticipantDeviceAdded(time(nullptr), false, participant, device);
		q->getCore()->getPrivate()->mainDb->addEvent(event);

		if (protocolVersion < Utils::Version(1, 1) && (capabilities & ServerGroupChatRoom::Capabilities::OneToOne) &&
		    allDevLeft) {
			/* If all other devices have left, let this new device to left state too, it will be invited to join if a
			 * message is sent to it. */
			setParticipantDeviceState(device, ParticipantDevice::State::Left);
		} else {
			setParticipantDeviceState(device, ParticipantDevice::State::ScheduledForJoining);
		}
	} else {
		lWarning() << q << ": Participant device " << participant << " cannot be added because not authorized";
	}
}

void ServerGroupChatRoomPrivate::designateAdmin() {
	L_Q();
	// Do not designate new admin for one-to-one chat room
	if (!(capabilities & ServerGroupChatRoom::Capabilities::OneToOne) && !q->getParticipants().empty()) {
		q->setParticipantAdminStatus(q->getConference()->participants.front(), true);
		lInfo() << q << ": New admin designated";
	}
}

void ServerGroupChatRoomPrivate::sendMessage(const shared_ptr<Message> &message,
                                             const std::shared_ptr<Address> &deviceAddr) {
	L_Q();

	shared_ptr<ChatMessage> msg = q->createChatMessage();
	copyMessageHeaders(message, msg);
	// Special custom header to identify MESSAGE that belong to server group chatroom
	msg->getPrivate()->addSalCustomHeader("Session-mode", "true");
	msg->setInternalContent(message->content);
	msg->getPrivate()->forceFromAddress(q->getConferenceAddress());
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

void ServerGroupChatRoomPrivate::finalizeCreation() {
	L_Q();
	std::shared_ptr<Address> confAddr = q->getConferenceAddress()->clone()->toSharedPtr();
	const ConferenceId conferenceId = ConferenceId(confAddr, confAddr);
	q->getConference()->setConferenceId(conferenceId);
	q->getCore()->getPrivate()->localListEventHandler->addHandler(
	    static_pointer_cast<LocalConference>(q->getConference())->eventHandler.get());
	lInfo() << q << " created";
	// Let the SIP stack set the domain and the port
	shared_ptr<Participant> me = q->getMe();
	me->setAddress(confAddr);
	auto addr = *confAddr;
	addr.setParam("isfocus");
	shared_ptr<CallSession> session = me->getSession();
	if (session->getState() == CallSession::State::Idle) {
		lInfo() << " Scheduling redirection to [" << addr << "] for Call session [" << session << "]";
		q->getCore()->doLater([session, addr] { session->redirect(addr); });
	} else {
		session->redirect(addr);
	}
	joiningPendingAfterCreation = true;
	chatRoomListener->onChatRoomInsertRequested(q->getSharedFromThis());
	q->setState(ConferenceInterface::State::Created);
	chatRoomListener->onChatRoomInsertInDatabaseRequested(q->getSharedFromThis());
}

// returns true if a new session has been created, false if there is already an existing and valid one.
shared_ptr<CallSession> ServerGroupChatRoomPrivate::makeSession(const std::shared_ptr<ParticipantDevice> &device) {
	L_Q();
	shared_ptr<CallSession> session = device->getSession();

	if (session) {
		switch (session->getState()) {
			case CallSession::State::End:
			case CallSession::State::Error:
			case CallSession::State::Released:
				session = nullptr; // our session is dead, we'll make a new one.
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
		if (capabilities & ServerGroupChatRoom::Capabilities::Ephemeral) {
			csp.addCustomHeader("Ephemerable", "true");
			csp.addCustomHeader("Ephemeral-Life-Time", to_string(params->getEphemeralLifetime()));
		}
		shared_ptr<Participant> participant =
		    const_pointer_cast<Participant>(device->getParticipant()->getSharedFromThis());
		session = participant->createSession(*q->getConference().get(), &csp, false, this);
		session->configure(LinphoneCallOutgoing, nullptr, nullptr, q->getConferenceAddress(), device->getAddress());
		device->setSession(session);
		session->initiateOutgoing();
		session->getPrivate()->createOp();
		// FIXME jehan check conference server  potential impact
		Address contactAddr(*q->getConferenceAddress());
		contactAddr.setParam("isfocus");
		contactAddr.setParam("text");
		session->getPrivate()->getOp()->setContactAddress(contactAddr.getImpl());
	}
	return session;
}

void ServerGroupChatRoomPrivate::inviteDevice(const shared_ptr<ParticipantDevice> &device) {
	L_Q();

	lInfo() << q << ": Inviting device '" << *device->getAddress() << "'";
	shared_ptr<Participant> participant =
	    const_pointer_cast<Participant>(device->getParticipant()->getSharedFromThis());
	shared_ptr<CallSession> session = makeSession(device);
	if (device->getState() == ParticipantDevice::State::Joining &&
	    (session->getState() == CallSession::State::OutgoingProgress ||
	     session->getState() == CallSession::State::Connected)) {
		lInfo() << q << ": outgoing INVITE already in progress.";
		return;
	}
	setParticipantDeviceState(device, ParticipantDevice::State::Joining);
	if (session && session->getState() == CallSession::State::IncomingReceived) {
		lInfo() << q << ": incoming INVITE in progress.";
		return;
	}

	list<std::shared_ptr<Address>> addressesList;
	for (const auto &invitedParticipant : q->getParticipants()) {
		if (invitedParticipant != participant) addressesList.push_back(invitedParticipant->getAddress());
	}
	if (addressesList.empty()) {
		// Having an empty participant list shall never happen, but should this happen don't spread the bug to clients.
		lError() << q << ": empty participant list, this should never happen, INVITE not sent.";
		return;
	}

	auto content = Content::create();
	content->setBodyFromUtf8(Utils::getResourceLists(addressesList));
	content->setContentType(ContentType::ResourceLists);
	content->setContentDisposition(ContentDisposition::RecipientListHistory);
	if (linphone_core_content_encoding_supported(q->getCore()->getCCore(), "deflate"))
		content->setContentEncoding("deflate");
	session->startInvite(nullptr, q->getUtf8Subject(), content);
}

void ServerGroupChatRoomPrivate::byeDevice(const std::shared_ptr<ParticipantDevice> &device) {
	L_Q();

	lInfo() << q << ": Asking device '" << *device->getAddress() << "' to leave";
	setParticipantDeviceState(device, ParticipantDevice::State::Leaving);
	shared_ptr<CallSession> session = makeSession(device);
	switch (session->getState()) {
		case CallSession::State::OutgoingInit:
			session->startInvite(nullptr, q->getUtf8Subject(), nullptr);
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
void ServerGroupChatRoomPrivate::notifyParticipantDeviceRegistration(
    const std::shared_ptr<Address> &participantDevice) {
	L_Q();

	shared_ptr<Participant> participant = q->findCachedParticipant(participantDevice);
	if (!participant) {
		lError() << q << ": " << participantDevice << " is not part of the chatroom.";
		return;
	}
	shared_ptr<ParticipantDevice> pd = participant->findDevice(participantDevice);
	if (!pd) {
		/* A device that does not have the required capabilities may be notified. */
		lInfo() << q << ": device " << participantDevice << " is not part of any participant of the chatroom.";
		return;
	}
	updateParticipantDeviceSession(pd, true);
}

bool ServerGroupChatRoomPrivate::isAdminLeft() const {
	L_Q();
	for (const auto &participant : q->getParticipants()) {
		if (participant->isAdmin()) return true;
	}
	return false;
}

void ServerGroupChatRoomPrivate::queueMessage(const shared_ptr<Message> &msg) {
	L_Q();
	for (const auto &participant : q->getParticipants()) {
		for (const auto &device : participant->getDevices()) {
			// Queue the message for all devices except the one that sent it
			if (*msg->fromAddr != *device->getAddress()) {
				queueMessage(msg, device->getAddress());
			}
		}
	}
}

void ServerGroupChatRoomPrivate::queueMessage(const shared_ptr<Message> &msg,
                                              const std::shared_ptr<Address> &deviceAddress) {
	chrono::system_clock::time_point timestamp = chrono::system_clock::now();
	string uri(deviceAddress->toString());
	// Remove queued messages older than one week
	while (!queuedMessages[uri].empty()) {
		shared_ptr<Message> m = queuedMessages[uri].front();
		chrono::hours age = chrono::duration_cast<chrono::hours>(timestamp - m->timestamp);
		chrono::hours oneWeek(168);
		if (age < oneWeek) break;
		queuedMessages[uri].pop();
	}
	queuedMessages[uri].push(msg);
}

/* The removal of participant device is done only when such device disapears from registration database, ie when a
 * device unregisters explicitely or removed by an administrator.
 */
void ServerGroupChatRoomPrivate::removeParticipantDevice(const shared_ptr<Participant> &participant,
                                                         const std::shared_ptr<Address> &deviceAddress) {
	L_Q();
	shared_ptr<Participant> participantCopy = participant; // make a copy of the shared_ptr because the participant may
	                                                       // be removed by setParticipantDeviceState().
	lInfo() << q << " device " << *deviceAddress << " is removed because it is has unregistered.";
	auto participantDevice = participant->findDevice(deviceAddress);
	if (!participantDevice) {
		lError() << q << " device " << *deviceAddress << " is removed, but we can't find it in this chatroom.";
		return;
	}
	// Notify to everyone the retirement of this device.
	auto deviceEvent =
	    q->getConference()->notifyParticipantDeviceRemoved(time(nullptr), false, participant, participantDevice);
	q->getCore()->getPrivate()->mainDb->addEvent(deviceEvent);

	// First set it as left, so that it may eventually trigger the destruction of the chatroom if no device are present
	// for any participant.
	setParticipantDeviceState(participantDevice, ParticipantDevice::State::Left, false);
	participantCopy->removeDevice(deviceAddress);
}

// -----------------------------------------------------------------------------

bool ServerGroupChatRoomPrivate::allDevicesLeft(const std::shared_ptr<Participant> &participant) {
	bool allDevicesLeft = true;

	for (const auto &device : participant->getDevices()) {
		if (device->getState() != ParticipantDevice::State::Left) {
			allDevicesLeft = false;
			break;
		}
	}
	return allDevicesLeft;
}

void ServerGroupChatRoomPrivate::onParticipantDeviceLeft(const std::shared_ptr<ParticipantDevice> &device) {
	L_Q();

	unique_ptr<MainDb> &mainDb = q->getCore()->getPrivate()->mainDb;
	lInfo() << q << ": Participant device '" << device->getAddress()->toString() << "' left";

	auto session = device->getSession();
	if (session) session->setListener(nullptr);

	if (!(capabilities & ServerGroupChatRoom::Capabilities::OneToOne) || protocolVersion >= Utils::Version(1, 1)) {
		shared_ptr<Participant> participant =
		    const_pointer_cast<Participant>(device->getParticipant()->getSharedFromThis());
		if (allDevicesLeft(participant) && q->findParticipant(participant->getAddress()) == nullptr) {
			lInfo() << q << ": Participant '" << participant->getAddress()->toString()
			        << "'removed and last device left, unsubscribing";
			unSubscribeRegistrationForParticipant(participant->getAddress());
			mainDb->deleteChatRoomParticipant(q->getSharedFromThis(), participant->getAddress());
		}
	}

	// device left, we no longuer need to receive subscription info from it
	if (device->isSubscribedToConferenceEventPackage()) {
		lError() << q << " still subscription pending for [" << device << "], terminating in emergency";
		// try to terminate subscription if any, but do not wait for anser.
		auto ev = device->getConferenceSubscribeEvent();
		ev->clearCallbacksList();
		ev->terminate();
		device->setConferenceSubscribeEvent(nullptr);
	}

	/* if all devices of participants are left we'll delete the chatroom*/
	if (q->getCore()->emptyChatroomsDeletionEnabled()) {
		bool allLeft = true;
		for (const auto &participant : q->cachedParticipants) {
			if (!allDevicesLeft(participant)) {
				allLeft = false;
				break;
			}
		}
		if (allLeft) {
			// Delete the chat room from the main DB as its termination process started and it cannot be retrieved in
			// the future
			lInfo() << q << ": Delete chatroom from MainDB as last participant has left";
			mainDb->deleteChatRoom(q->getConferenceId());
			if (q->getState() != ConferenceInterface::State::TerminationPending) {
				q->setState(ConferenceInterface::State::TerminationPending);
			}
			q->setState(ConferenceInterface::State::Terminated);
			lInfo() << q << ": No participant left, deleting the chat room";
			requestDeletion();
		}
	}
}

// -----------------------------------------------------------------------------

void ServerGroupChatRoomPrivate::onChatRoomInsertRequested(const shared_ptr<AbstractChatRoom> &chatRoom) {
	L_Q();
	q->getCore()->getPrivate()->insertChatRoom(chatRoom);
}

void ServerGroupChatRoomPrivate::onChatRoomInsertInDatabaseRequested(const shared_ptr<AbstractChatRoom> &chatRoom) {
	L_Q();
	q->getCore()->getPrivate()->insertChatRoomWithDb(chatRoom);
}

void ServerGroupChatRoomPrivate::onChatRoomDeleteRequested(BCTBX_UNUSED(const shared_ptr<AbstractChatRoom> &chatRoom)) {
	L_Q();
	q->deleteFromDb();
}

std::shared_ptr<Participant>
ServerGroupChatRoomPrivate::getOtherParticipant(const std::shared_ptr<Participant> someParticipant) const {
	L_Q();
	std::shared_ptr<Participant> otherParticipant;
	bool looksSane = false;
	if (capabilities & ServerGroupChatRoom::Capabilities::OneToOne) {
		for (auto &p : q->getParticipants()) {
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

void ServerGroupChatRoomPrivate::onBye(const shared_ptr<ParticipantDevice> &participantLeaving) {
	L_Q();
	bool shouldRemoveParticipant = true;

	if (capabilities & ServerGroupChatRoom::Capabilities::OneToOne) {
		if (protocolVersion < Utils::Version(1, 1)) {
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
				q->removeParticipant(otherParticipant);
				// Do not wait to delete the chat room from the list stored in the core as the process of terminating it
				// has already started and it cannot be restored
				lInfo() << q << ": Delete chatroom from MainDB as last participant has left";
				unique_ptr<MainDb> &mainDb = q->getCore()->getPrivate()->mainDb;
				mainDb->deleteChatRoom(q->getConferenceId());
			}
			q->setState(ConferenceInterface::State::TerminationPending);
		}
	}
	if (shouldRemoveParticipant) {
		// Participant leaves the chat room on its own by sending a BYE.
		// Set it the Leaving state first, so that q->removeParticipant() does nothing with it (does not attempt to send
		// a BYE...).
		setParticipantDeviceState(participantLeaving, ParticipantDevice::State::Leaving);
		// Remove participant will change other devices to Leaving state.
		q->removeParticipant(participantLeaving->getParticipant()->getSharedFromThis());
		// But since we received its BYE, it is actually already left.
	}
	// Notify the lient only if the participant should be removed. Doing so, the client may not be able to send any
	// further message to this device in particular if using an old version of a secured chat room
	setParticipantDeviceState(participantLeaving, ParticipantDevice::State::Left, shouldRemoveParticipant);
}

// -----------------------------------------------------------------------------

bool ServerGroupChatRoomPrivate::dispatchMessagesAfterFullState(const shared_ptr<CallSession> &session) const {
	L_Q();
	auto device = q->findCachedParticipantDevice(session);
	if (!device) {
		lWarning() << q << " dispatchMessagesAfterFullState on unknown device.";
		return false; // Assume it is a recent device.
	}
	return dispatchMessagesAfterFullState(device);
}

bool ServerGroupChatRoomPrivate::dispatchMessagesAfterFullState(const shared_ptr<ParticipantDevice> &device) const {
	auto protocols = Utils::parseCapabilityDescriptor(device->getCapabilityDescriptor());
	auto groupchat = protocols.find("groupchat");
	return ((groupchat == protocols.end()) || (groupchat->second < Utils::Version(1, 2)));
}

void ServerGroupChatRoomPrivate::onCallSessionStateChanged(const shared_ptr<CallSession> &session,
                                                           CallSession::State newState,
                                                           const string &message) {
	L_Q();
	auto device = q->findCachedParticipantDevice(session);
	if (!device) {
		lInfo() << q << " onCallSessionStateChanged on unknown device (maybe not yet).";
		if ((newState == CallSession::State::Released) && (session->getReason() == LinphoneReasonNotAcceptable) &&
		    (q->getState() == ConferenceInterface::State::CreationFailed)) {
			// Delete the chat room from the main DB as its termination process started and it cannot be retrieved in
			// the future
			lInfo() << q << ": Delete chatroom from MainDB as its creation failed";
			unique_ptr<MainDb> &mainDb = q->getCore()->getPrivate()->mainDb;
			mainDb->deleteChatRoom(q->getConferenceId());
			q->setState(ConferenceInterface::State::TerminationPending);
			q->setState(ConferenceInterface::State::Terminated);
			requestDeletion();
		}
		return;
	}
	switch (newState) {
		case CallSession::State::Connected:
			if (device->getState() == ParticipantDevice::State::Leaving) {
				byeDevice(device);
			} else {
				if ((session->getDirection() == LinphoneCallOutgoing) && !dispatchMessagesAfterFullState(session)) {
					// According to RFC3261, a request is successful once the 200Ok is received
					moveDeviceToPresent(session);
				}

				if (joiningPendingAfterCreation && mInitiatorDevice && mInitiatorDevice == device) {
					lInfo() << "Session of the initiation of the chatroom is in state " << Utils::toString(newState)
					        << " things can start now.";
					joiningPendingAfterCreation = false;
					updateParticipantsSessions();
				}
			}
			break;
		case CallSession::State::End: {
			const auto errorInfo = session->getErrorInfo();

			if (errorInfo != nullptr && linphone_error_info_get_protocol_code(errorInfo) > 299) {
				if (device->getState() == ParticipantDevice::State::Joining ||
				    device->getState() == ParticipantDevice::State::Present) {
					lWarning() << q << ": Received a BYE from " << device->getAddress() << " with reason "
					           << linphone_error_info_get_protocol_code(errorInfo)
					           << ", setting it back to ScheduledForJoining.";
					setParticipantDeviceState(device, ParticipantDevice::State::ScheduledForJoining);
					if (linphone_error_info_get_protocol_code(errorInfo) == 408 && mInitiatorDevice &&
					    mInitiatorDevice == device) {
						// Recovering if the initiator of the chatroom did not receive the 200Ok or the ACK has been
						// lost
						inviteDevice(device);
					}
				}
			} else {
				if (device->getState() == ParticipantDevice::State::Present) {
					lInfo() << q << ": " << device->getParticipant()->getAddress()->toString()
					        << " is leaving the chatroom.";
					onBye(device);
				}
			}
		} break;
		case CallSession::State::Released:
			/* Handle the case of participant we've send a BYE. */
			if (device->getState() == ParticipantDevice::State::Leaving &&
			    session->getPreviousState() == CallSession::State::End) {
				if (session->getReason() == LinphoneReasonNone) {
					/* We've received a 200 Ok for our BYE, so it is assumed to be left. */
					setParticipantDeviceState(device, ParticipantDevice::State::Left);
				} else if (session->getReason() == LinphoneReasonNoMatch) {
					/* Our current session was lost, but the device is currently reachable, so retry to send the BYE
					 * now. */
					byeDevice(device);
				}
			}
			break;
		case CallSession::State::UpdatedByRemote: {
			shared_ptr<Participant> participant = q->findParticipant(session);
			if (participant && participant->isAdmin()) {
				/* The only thing that a participant can change with re-INVITE is the subject. */
				handleSubjectChange(session->getPrivate()->getOp());
				handleEphemeralSettingsChange(session);
			}
		} break;
		default:
			break;
	}
	linphone_chat_room_notify_session_state_changed(getCChatRoom(), static_cast<LinphoneCallState>(newState),
	                                                message.c_str());
}

void ServerGroupChatRoomPrivate::onCallSessionSetReleased(const shared_ptr<CallSession> &session) {
	L_Q();
	shared_ptr<ParticipantDevice> device = q->findCachedParticipantDevice(session);
	if (device) device->setSession(nullptr);
}

void ServerGroupChatRoomPrivate::moveDeviceToPresent(const std::shared_ptr<CallSession> &session) {
	L_Q();
	shared_ptr<ParticipantDevice> device = q->findCachedParticipantDevice(session);
	moveDeviceToPresent(device);
}

void ServerGroupChatRoomPrivate::moveDeviceToPresent(const std::shared_ptr<ParticipantDevice> &device) {
	if (device && !ParticipantDevice::isLeavingState(device->getState())) {
		setParticipantDeviceState(device, ParticipantDevice::State::Present);
	}
}

void ServerGroupChatRoomPrivate::onAckReceived(const std::shared_ptr<CallSession> &session,
                                               BCTBX_UNUSED(LinphoneHeaders *headers)) {
	if (!dispatchMessagesAfterFullState(session)) {
		moveDeviceToPresent(session);
	}
}

// =============================================================================

ServerGroupChatRoom::ServerGroupChatRoom(const shared_ptr<Core> &core, SalCallOp *op)
    : ChatRoom(*new ServerGroupChatRoomPrivate,
               core,
               ChatRoomParams::getDefaults(core),
               make_shared<LocalConference>(
                   core, Address::create(op->getTo()), nullptr, ConferenceParams::create(core->getCCore()), this)) {
	L_D();

	getCore()->getPrivate()->registerListener(this);
	getConference()->setUtf8Subject(op->getSubject());

	getConference()->setConferenceId(ConferenceId());

	const char *oneToOneChatRoomStr = sal_custom_header_find(op->getRecvCustomHeaders(), "One-To-One-Chat-Room");
	if (oneToOneChatRoomStr && (strcmp(oneToOneChatRoomStr, "true") == 0))
		d->capabilities |= ServerGroupChatRoom::Capabilities::OneToOne;
	string endToEndEncrypted =
	    L_C_TO_STRING(sal_custom_header_find(op->getRecvCustomHeaders(), "End-To-End-Encrypted"));
	if (endToEndEncrypted == "true") d->capabilities |= ServerGroupChatRoom::Capabilities::Encrypted;
	string ephemerable = L_C_TO_STRING(sal_custom_header_find(op->getRecvCustomHeaders(), "Ephemerable"));
	if (ephemerable == "true") {
		d->capabilities |= ServerGroupChatRoom::Capabilities::Ephemeral;
	}

	d->params = ChatRoomParams::fromCapabilities(d->capabilities);

	if (ephemerable == "true") {
		string ephemeralLifeTime =
		    L_C_TO_STRING(sal_custom_header_find(op->getRecvCustomHeaders(), "Ephemeral-Life-Time"));
		long time = std::stol(ephemeralLifeTime, nullptr);
		d->params->setEphemeralLifetime(time);
	}

	shared_ptr<CallSession> session = getMe()->createSession(*getConference().get(), nullptr, false, d);
	const auto from = Address::create(op->getFrom());
	const auto to = Address::create(op->getTo());
	session->configure(LinphoneCallIncoming, nullptr, op, from, to);
	d->protocolVersion = CorePrivate::groupChatProtocolVersion;
}

ServerGroupChatRoom::ServerGroupChatRoom(const shared_ptr<Core> &core,
                                         const std::shared_ptr<Address> &peerAddress,
                                         AbstractChatRoom::CapabilitiesMask capabilities,
                                         const shared_ptr<ChatRoomParams> &params,
                                         const string &subject,
                                         list<shared_ptr<Participant>> &&participants,
                                         unsigned int lastNotifyId)
    : ChatRoom(
          *new ServerGroupChatRoomPrivate(capabilities),
          core,
          params,
          make_shared<LocalConference>(core, peerAddress, nullptr, ConferenceParams::create(core->getCCore()), this)) {
	L_D();
	cachedParticipants = std::move(participants);
	getCore()->getPrivate()->registerListener(this);
	getConference()->setLastNotify(lastNotifyId);
	getConference()->setConferenceId(ConferenceId(peerAddress, peerAddress));
	getConference()->confParams->setConferenceAddress(peerAddress);
	getConference()->confParams->setUtf8Subject(subject);
	getConference()->confParams->enableChat(true);
	getCore()->getPrivate()->localListEventHandler->addHandler(
	    static_pointer_cast<LocalConference>(getConference())->eventHandler.get());
	d->protocolVersion = CorePrivate::groupChatProtocolVersion;
}

ServerGroupChatRoom::~ServerGroupChatRoom() {
	lInfo() << this << " destroyed.";
	try {
		if (getCore()->getPrivate()->localListEventHandler) {
			getCore()->getPrivate()->localListEventHandler->removeHandler(
			    static_pointer_cast<LocalConference>(getConference())->eventHandler.get());
		}
		getCore()->getPrivate()->unregisterListener(this);
	} catch (const bad_weak_ptr &) {
		// Unable to unregister listener here. Core is destroyed and the listener doesn't exist.
	}
};

shared_ptr<Core> ServerGroupChatRoom::getCore() const {
	return ChatRoom::getCore();
}

ServerGroupChatRoom::CapabilitiesMask ServerGroupChatRoom::getCapabilities() const {
	L_D();
	return d->capabilities;
}

shared_ptr<Participant> ServerGroupChatRoom::findParticipant(const shared_ptr<const CallSession> &session) const {
	for (const auto &participant : getParticipants()) {
		shared_ptr<ParticipantDevice> device = participant->findDevice(session);
		if (device || (participant->getSession() == session)) return participant;
	}
	lInfo() << "Unable to find participant with call session " << session << " in server group chat room " << this;
	return nullptr;
}

shared_ptr<Participant> ServerGroupChatRoom::findParticipant(const std::shared_ptr<Address> &participantAddress) const {
	for (const auto &participant : getParticipants()) {
		if (participant->getAddress()->weakEqual(*participantAddress)) return participant;
	}
	lInfo() << "Unable to find participant with address " << participantAddress->toString()
	        << " in server group chat room " << this;
	return nullptr;
}

shared_ptr<Participant> ServerGroupChatRoom::findCachedParticipant(const shared_ptr<const CallSession> &session) const {
	for (const auto &participant : cachedParticipants) {
		shared_ptr<ParticipantDevice> device = participant->findDevice(session);
		if (device || (participant->getSession() == session)) return participant;
	}
	return nullptr;
}

shared_ptr<Participant>
ServerGroupChatRoom::findCachedParticipant(const std::shared_ptr<Address> &participantAddress) const {
	for (const auto &participant : cachedParticipants) {
		if (participant->getAddress()->weakEqual(*participantAddress)) return participant;
	}
	return nullptr;
}

shared_ptr<ParticipantDevice>
ServerGroupChatRoom::findCachedParticipantDevice(const shared_ptr<const CallSession> &session) const {

	for (const auto &participant : cachedParticipants) {
		for (const auto &device : participant->getDevices()) {
			if (device->getSession() == session) return device;
		}
	}

	return nullptr;
}

void ServerGroupChatRoom::allowCpim(BCTBX_UNUSED(bool value)) {
}

void ServerGroupChatRoom::allowMultipart(BCTBX_UNUSED(bool value)) {
}

bool ServerGroupChatRoom::canHandleCpim() const {
	return true;
}

bool ServerGroupChatRoom::canHandleMultipart() const {
	return true;
}

bool ServerGroupChatRoom::hasBeenLeft() const {
	return false;
}

bool ServerGroupChatRoom::isReadOnly() const {
	return false;
}

bool ServerGroupChatRoom::addParticipant(const std::shared_ptr<Address> &participantAddress) {
	L_D();

	if (participantAddress->hasUriParam("gr")) {
		lInfo() << this << ": Not adding participant '" << participantAddress->toString()
		        << "' because it is a gruu address.";
		return false;
	}

	if (findParticipant(participantAddress)) {
		lInfo() << this << ": Not adding participant '" << participantAddress->toString()
		        << "' because it is already a participant";
		return false;
	}

	shared_ptr<Participant> participant = findCachedParticipant(participantAddress);

	if (participant == nullptr && (d->capabilities & ServerGroupChatRoom::Capabilities::OneToOne) &&
	    getParticipantCount() == 2) {
		lInfo() << this << ": Not adding participant '" << participantAddress->toString()
		        << "' because this OneToOne chat room already has 2 participants";
		return false;
	}

	/* Handle the case where a participant is removed then re-added to chat room. In such case, until all devices have
	 * left the chatroom, the participant is still referenced in the chatroom, with devices either left or leaving.
	 * Furthermore, registration subscription is still active, so we don't need to wait for a notify, and we can instead
	 * proceed immediately with the INVITE of its devices.
	 */
	if (participant) {
		d->resumeParticipant(participant);
	} else {
		lInfo() << this << ": Requested to add participant '" << participantAddress->toString()
		        << "', checking capabilities first.";
		list<std::shared_ptr<Address>> participantsList;
		participantsList.push_back(participantAddress);
		d->subscribeRegistrationForParticipants(participantsList, true);
	}
	return true;
}

const std::shared_ptr<Address> &ServerGroupChatRoom::getConferenceAddress() const {
	return getConference()->getConferenceAddress();
}

shared_ptr<Participant> ServerGroupChatRoom::getMe() const {
	return getConference()->getMe();
}

int ServerGroupChatRoom::getParticipantCount() const {
	return (int)cachedParticipants.size();
}

const list<shared_ptr<ParticipantDevice>> ServerGroupChatRoom::getParticipantDevices() const {
	return getConference()->getParticipantDevices();
}

const list<shared_ptr<Participant>> &ServerGroupChatRoom::getParticipants() const {
	return getConference()->getParticipants();
}

void ServerGroupChatRoom::join() {
}

void ServerGroupChatRoom::leave() {
}

void ServerGroupChatRoom::onFirstNotifyReceived(const std::shared_ptr<Address> &addr) {
	L_D();
	for (const auto &participant : getParticipants()) {
		for (const auto &device : participant->getDevices()) {
			if ((*device->getAddress() == *addr) && (d->dispatchMessagesAfterFullState(device))) {
				d->moveDeviceToPresent(device);
				return;
			}
		}
	}
}

bool ServerGroupChatRoom::removeParticipant(const shared_ptr<Participant> &participant) {
	L_D();
	if (participant->isAdmin()) setParticipantAdminStatus(participant, false);
	d->removeParticipant(participant);
	return true;
}

void ServerGroupChatRoom::setParticipantAdminStatus(const shared_ptr<Participant> &participant, bool isAdmin) {
	L_D();
	if (isAdmin != participant->isAdmin()) {
		participant->setAdmin(isAdmin);
		if (!(d->capabilities & ServerGroupChatRoom::Capabilities::OneToOne)) {
			shared_ptr<ConferenceParticipantEvent> event =
			    getConference()->notifyParticipantSetAdmin(time(nullptr), false, participant, participant->isAdmin());
			getCore()->getPrivate()->mainDb->addEvent(event);
		}
	}
}

void ServerGroupChatRoom::setSubject(const string &subject) {
	if (subject != getSubject()) {
		getConference()->setSubject(subject);
		shared_ptr<ConferenceSubjectEvent> event =
		    getConference()->notifySubjectChanged(time(nullptr), false, getUtf8Subject());
		getCore()->getPrivate()->mainDb->addEvent(event);
	}
}

void ServerGroupChatRoom::setState(ConferenceInterface::State state) {
	L_D();
	ChatRoom::setState(state);
	if (state == ConferenceInterface::State::Created) {
		// Handle transitional states (joining and leaving of participants)
		// This is needed when the chat room is loaded from its state in database
		list<std::shared_ptr<Address>> participantAddresses;
		for (const auto &participant : cachedParticipants) {
			participantAddresses.emplace_back(participant->getAddress());

			if (d->capabilities & ServerGroupChatRoom::Capabilities::OneToOne) {
				/**
				 * With protocol < 1.1, even if devices can BYE and get rid of their session, actually no one can leave
				 * a one to one chatroom. With protocol >= 1.1, two states are possible: both have
				 * Present/Joining/ScheduledForJoining devices OR both have Left/Leaving/ScheduledForLeaving devices.
				 * Since we don't have the protocol version at this stage (it will be known after receiving register
				 * information), it is not a problem to push the two participants in the authorized list even if they
				 * are in the process of leaving.
				 */
				getConference()->participants.push_back(participant);
			} else {
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
						case ParticipantDevice::State::Alerting:
							atLeastOneDeviceJoining = true;
							break;
						case ParticipantDevice::State::OnHold:
						case ParticipantDevice::State::Present:
						case ParticipantDevice::State::MutedByFocus:
							atLeastOneDevicePresent = true;
							break;
						case ParticipantDevice::State::Left:
							break;
					}
				}
				// Basically the only case where this participant is not authorized is in case it was removed from the
				// room but not all its devices were "BYEed" yet. This is what the line below is testing. Might be
				// better to add a new state in the participant Class,
				//  but it's not the case yet.
				if (atLeastOneDevicePresent || atLeastOneDeviceJoining || atLeastOneDeviceLeaving == false) {
					getConference()->participants.push_back(participant);
				}
			}
		}
		d->updateParticipantsSessions();
		// Subscribe to the registration events from the proxy
		d->subscribeRegistrationForParticipants(participantAddresses, false);
	}
}

void ServerGroupChatRoom::subscribeReceived(const shared_ptr<EventSubscribe> &event) {
	L_D();

	const auto &participantAddress = event->getFrom();
	shared_ptr<Participant> participant = findCachedParticipant(participantAddress);
	if (participant) {
		std::shared_ptr<Address> contactAddr = event->getRemoteContact();
		shared_ptr<ParticipantDevice> device = participant->findDevice(contactAddr);
		const auto deviceState = device ? device->getState() : ParticipantDevice::State::ScheduledForJoining;
		if (device && (deviceState == ParticipantDevice::State::ScheduledForJoining)) {
			lInfo() << "Inviting device " << *device->getAddress() << " because it was scheduled to join the chat room";
			// Invite device as last time round it was attempted, the INVITE session errored out
			d->inviteDevice(device);
		}
	}

	static_pointer_cast<LocalConference>(getConference())->subscribeReceived(event);
	// Store last notify ID in the database
	getCore()->getPrivate()->mainDb->insertChatRoom(getSharedFromThis(), getConference()->getLastNotify());
}

void ServerGroupChatRoom::onGlobalStateChanged(LinphoneGlobalState state) {
	L_D();
	if (state == LinphoneGlobalOn) {
		// Try to subscribe again to cached participants when the core goes to GlobalOn state because the first time
		// around (when the chat room goes into state Created) the core might have still being initializing. We must do
		// be sure that the core is initialized because if one or more participants are in another domain that the
		// chatroom. In such a scenarion, in fact, the server might be sending out SIP SUBSCRIBE and therefore the
		// channel should not be destroyed by network changes that occur during startup
		lInfo() << "The core has reached the GlobalOn state, therefore try to subscribe participants of chatroom "
		        << *getConference()->getConferenceAddress();
		list<shared_ptr<Address>> participantAddresses;
		for (const auto &participant : cachedParticipants) {
			participantAddresses.emplace_back(participant->getAddress());
		}
		// Subscribe to the registration events from the proxy
		d->subscribeRegistrationForParticipants(participantAddresses, false);
	}
}
// -----------------------------------------------------------------------------

ostream &operator<<(ostream &stream, const ServerGroupChatRoom *chatRoom) {
	// TODO: Is conference ID needed to be stored in both remote conference and chat room base classes?
	const auto &peerAddress = chatRoom->getConferenceId().getPeerAddress();
	return stream << "ServerGroupChatRoom [" << (peerAddress ? peerAddress->toString() : std::string("unknown address"))
	              << "]";
}

LINPHONE_END_NAMESPACE
