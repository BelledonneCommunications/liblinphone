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

#include "address/address.h"
#include "basic-to-client-group-chat-room.h"
#include "c-wrapper/c-wrapper.h"
#include "conference/handlers/remote-conference-event-handler.h"
#include "client-group-chat-room-p.h"
#include "conference/handlers/remote-conference-list-event-handler.h"
#include "conference/participant.h"
#include "conference/participant-device.h"
#include "conference/remote-conference.h"
#include "conference/session/call-session-p.h"
#include "content/content-disposition.h"
#include "content/content-type.h"
#include "core/core-p.h"
#include "logger/logger.h"
#include "sal/refer-op.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------
//Removes own address and existing participants from the list.
//Also removes gru from kept addresses
list<IdentityAddress> ClientGroupChatRoomPrivate::cleanAddressesList (const list<IdentityAddress> &addresses) const {
	L_Q();
	list<IdentityAddress> cleanedList(addresses);

	cleanedList.sort();
	cleanedList.unique();
	for (auto it = cleanedList.begin(); it != cleanedList.end();) {
		if (q->findParticipant(*it) || (q->getMe()->getAddress() == *it)) {
			it = cleanedList.erase(it);
		} else {
			it++;
		}
	}
	return cleanedList;
}

shared_ptr<CallSession> ClientGroupChatRoomPrivate::createSession () {
	L_Q();
	L_Q_T(RemoteConference, qConference);

	CallSessionParams csp;
	csp.addCustomHeader("Require", "recipient-list-invite");
	csp.addCustomContactParameter("text");
	if (capabilities & ClientGroupChatRoom::Capabilities::OneToOne)
		csp.addCustomHeader("One-To-One-Chat-Room", "true");
	if (capabilities & ClientGroupChatRoom::Capabilities::Encrypted)
		csp.addCustomHeader("End-To-End-Encrypted", "true");

	shared_ptr<Participant> focus = qConference->focus;
	shared_ptr<CallSession> session = focus->createSession(*q, &csp, false, callSessionListener);
	Address myCleanedAddress(q->getMe()->getAddress());
	myCleanedAddress.removeUriParam("gr"); // Remove gr parameter for INVITE.
	session->configure(LinphoneCallOutgoing, nullptr, nullptr, myCleanedAddress, focus->getDevices().front()->getAddress());
	session->initiateOutgoing();
	session->getPrivate()->createOp();
	return session;
}

void ClientGroupChatRoomPrivate::notifyReceived (const string &body) {
	L_Q_T(RemoteConference, qConference);
	qConference->eventHandler->notifyReceived(body);
}

void ClientGroupChatRoomPrivate::multipartNotifyReceived (const string &body) {
	L_Q_T(RemoteConference, qConference);
	qConference->eventHandler->multipartNotifyReceived(body);
}

// -----------------------------------------------------------------------------

void ClientGroupChatRoomPrivate::setCallSessionListener (CallSessionListener *listener) {
	L_Q();
	L_Q_T(RemoteConference, qConference);

	callSessionListener = listener;
	shared_ptr<CallSession> session = qConference->focus->getSession();
	if (session)
		session->getPrivate()->setCallSessionListener(listener);
	for (const auto &participant : q->getParticipants()) {
		session = participant->getSession();
		if (session)
			session->getPrivate()->setCallSessionListener(listener);
	}
}

void ClientGroupChatRoomPrivate::addOneToOneCapability () {
	capabilities |= ClientGroupChatRoom::Capabilities::OneToOne;
}

unsigned int ClientGroupChatRoomPrivate::getLastNotifyId () const {
	L_Q_T(RemoteConference, qConference);
	return qConference->getLastNotify();
}

// -----------------------------------------------------------------------------

void ClientGroupChatRoomPrivate::confirmJoining (SalCallOp *op) {
	L_Q();
	L_Q_T(RemoteConference, qConference);

	auto focus = qConference->focus;
	bool previousSession = (focus->getSession() != nullptr);
	auto session = focus->createSession(*q, nullptr, false, this);
	session->configure(LinphoneCallIncoming, nullptr, op, Address(op->getFrom()), Address(op->getTo()));
	session->startIncomingNotification(false);

	if (!previousSession) {
		setState(ClientGroupChatRoom::State::CreationPending);
		// Handle participants addition
		list<IdentityAddress> identAddresses = ClientGroupChatRoom::parseResourceLists(op->getRemoteBody());
		for (const auto &addr : identAddresses) {
			auto participant = q->findParticipant(addr);
			if (!participant) {
				participant = Participant::create(q,addr);
				qConference->participants.push_back(participant);
			}
		}
	}
	acceptSession(session);
}

// -----------------------------------------------------------------------------

void ClientGroupChatRoomPrivate::onChatRoomInsertRequested (const shared_ptr<AbstractChatRoom> &chatRoom) {
	L_Q();
	q->getCore()->getPrivate()->insertChatRoom(chatRoom);
}

void ClientGroupChatRoomPrivate::onChatRoomInsertInDatabaseRequested (const shared_ptr<AbstractChatRoom> &chatRoom) {
	L_Q();
	L_Q_T(RemoteConference, qConference);

	unsigned int notifyId = qConference->getLastNotify();
	q->getCore()->getPrivate()->insertChatRoomWithDb(chatRoom, notifyId);
}

void ClientGroupChatRoomPrivate::onChatRoomDeleteRequested (const shared_ptr<AbstractChatRoom> &chatRoom) {
	L_Q();
	q->getCore()->deleteChatRoom(chatRoom);
	setState(ClientGroupChatRoom::State::Deleted);
}

// -----------------------------------------------------------------------------

void ClientGroupChatRoomPrivate::onCallSessionSetReleased (const shared_ptr<CallSession> &session) {
	L_Q_T(RemoteConference, qConference);

	shared_ptr<Participant> participant = qConference->focus;
	if (session == participant->getSession())
		participant->removeSession();
}

void ClientGroupChatRoomPrivate::onCallSessionStateChanged (
	const shared_ptr<CallSession> &session,
	CallSession::State newState,
	const string &message
) {
	L_Q();
	L_Q_T(RemoteConference, qConference);

	if (newState == CallSession::State::Connected) {
		if (q->getState() == ChatRoom::State::CreationPending) {
			onChatRoomCreated(*session->getRemoteContactAddress());
		} else if (q->getState() == ChatRoom::State::TerminationPending){
			/* This is the case where we have re-created the session in order to quit the chatroom.
			 * In this case, defer the sending of the bye so that it is sent after the ACK.
			 * Indeed, the ACK is sent immediately after being notified of the Connected state.*/
			q->getCore()->doLater([qConference](){
				qConference->focus->getSession()->terminate();
			});
		}
	} else if (newState == CallSession::State::End) {
		setState(ChatRoom::State::TerminationPending);
	} else if (newState == CallSession::State::Released) {
		if (q->getState() == ChatRoom::State::TerminationPending) {
			if (session->getReason() == LinphoneReasonNone
                || session->getReason() ==  LinphoneReasonDeclined) {
				// Everything is fine, the chat room has been left on the server side.
				// Or received 603 Declined, the chat room has been left on the server side but
				// remains local.
				q->onConferenceTerminated(q->getConferenceAddress());
			} else {
				// Go to state TerminationFailed and then back to Created since it has not been terminated
				setState(ChatRoom::State::TerminationFailed);
				setState(ChatRoom::State::Created);
			}
		}
	} else if (newState == CallSession::State::Error) {
		if (q->getState() == ChatRoom::State::CreationPending)
			setState(ChatRoom::State::CreationFailed);
		else if (q->getState() == ChatRoom::State::TerminationPending) {
			if (session->getReason() == LinphoneReasonNotFound) {
				// Somehow the chat room is no longer known on the server, so terminate it
				q->onConferenceTerminated(q->getConferenceAddress());
			} else {
				// Go to state TerminationFailed and then back to Created since it has not been terminated
				setState(ChatRoom::State::TerminationFailed);
				setState(ChatRoom::State::Created);
			}
		}
	}
}

void ClientGroupChatRoomPrivate::onChatRoomCreated (const Address &remoteContact) {
	L_Q();
	L_Q_T(RemoteConference, qConference);

	ConferenceAddress addr(remoteContact);
	q->onConferenceCreated(addr);
	if (remoteContact.hasParam("isfocus")) {
		if (q->getCore()->getPrivate()->remoteListEventHandler->findHandler(q->ChatRoom::getConferenceId())) {
			q->getCore()->getPrivate()->remoteListEventHandler->subscribe();
		} else {
			bgTask.start(q->getCore(), 32); // It will be stopped when receiving the first notify
			qConference->eventHandler->subscribe(q->getConferenceId());
		}
	}
}

// -----------------------------------------------------------------------------

void ClientGroupChatRoomPrivate::acceptSession (const shared_ptr<CallSession> &session) {
	if (session->getState() == CallSession::State::UpdatedByRemote)
		session->acceptUpdate();
	else
		session->accept();
}

// =============================================================================

ClientGroupChatRoom::ClientGroupChatRoom (
	const shared_ptr<Core> &core,
	const IdentityAddress &focus,
	const ConferenceId &conferenceId,
	const string &subject,
	const Content &content,
	CapabilitiesMask capabilities,
	const std::shared_ptr<ChatRoomParams> &params
) :
ChatRoom(*new ClientGroupChatRoomPrivate(capabilities | ChatRoom::Capabilities::Conference), core, conferenceId, params),
RemoteConference(core, conferenceId.getLocalAddress(), nullptr) {
	L_D();
	RemoteConference::setSubject(subject);
	for (const auto &addr : Conference::parseResourceLists(content))
		participants.push_back(Participant::create(this,addr));

	//if preserve_backward_compatibility, force creation of secure room in all cases
	if (params->isEncrypted() || linphone_config_get_bool(linphone_core_get_config(getCore()->getCCore()), "lime", "preserve_backward_compatibility",FALSE))
		d->capabilities |= ClientGroupChatRoom::Capabilities::Encrypted;
	this->focus = Participant::create(this,focus);
	this->focus->addDevice(focus);
}

ClientGroupChatRoom::ClientGroupChatRoom (
	const shared_ptr<Core> &core,
	const string &factoryUri,
	const IdentityAddress &me,
	const string &subject,
	CapabilitiesMask capabilities,
	const std::shared_ptr<ChatRoomParams> &params
) : ClientGroupChatRoom(
	core,
	IdentityAddress(factoryUri),
	ConferenceId(IdentityAddress(), me),
	subject,
	Content(),
	capabilities,
	params
) {}

ClientGroupChatRoom::ClientGroupChatRoom (
	const shared_ptr<Core> &core,
	const ConferenceId &conferenceId,
	shared_ptr<Participant> &me,
	AbstractChatRoom::CapabilitiesMask capabilities,
	const std::shared_ptr<ChatRoomParams> &params,
	const string &subject,
	list<shared_ptr<Participant>> &&newParticipants,
	unsigned int lastNotifyId,
	bool hasBeenLeft
) : ChatRoom(*new ClientGroupChatRoomPrivate(capabilities | ClientGroupChatRoom::Capabilities::Conference), core, conferenceId, params),
RemoteConference(core, me->getAddress(), nullptr) {
	L_D();

	const IdentityAddress &peerAddress = conferenceId.getPeerAddress();
	this->focus = Participant::create(this,peerAddress);
	this->focus->addDevice(peerAddress);
	this->conferenceAddress = peerAddress;
	this->subject = subject;
	this->participants = move(newParticipants);
	this->conferenceId = conferenceId;

	getMe()->setAdmin(me->isAdmin());
	for (const auto &device : me->getDevices())
		getMe()->addDevice(device->getAddress(), device->getName());

	bool_t forceFullState = linphone_config_get_bool(linphone_core_get_config(getCore()->getCCore()), "misc", "conference_event_package_force_full_state", FALSE);
	lastNotify = (forceFullState ? 0 : lastNotifyId);
	lInfo() << "Last notify set to [" << lastNotify << "] for conference [" << this << "]";

	if (!hasBeenLeft){
		getCore()->getPrivate()->remoteListEventHandler->addHandler(eventHandler.get());
		d->listHandlerUsed = true;
	}
}

ClientGroupChatRoom::~ClientGroupChatRoom () {
	L_D();

	try {
		if (getCore()->getPrivate()->remoteListEventHandler && d->listHandlerUsed){
			getCore()->getPrivate()->remoteListEventHandler->removeHandler(eventHandler.get());
		}
	} catch (const bad_weak_ptr &) {
		// Unable to unregister listener here. Core is destroyed and the listener doesn't exist.
	}
	d->setCallSessionListener(nullptr);
}

shared_ptr<Core> ClientGroupChatRoom::getCore () const {
	return ChatRoom::getCore();
}

void ClientGroupChatRoom::allowCpim (bool value) {

}

void ClientGroupChatRoom::allowMultipart (bool value) {

}

bool ClientGroupChatRoom::canHandleCpim () const {
	return true;
}

bool ClientGroupChatRoom::canHandleMultipart () const {
	return true;
}

ClientGroupChatRoom::CapabilitiesMask ClientGroupChatRoom::getCapabilities () const {
	L_D();
	return d->capabilities;
}

ChatRoom::SecurityLevel ClientGroupChatRoom::getSecurityLevel () const {
	L_D();
	if (!(d->capabilities & ClientGroupChatRoom::Capabilities::Encrypted)) {
		return AbstractChatRoom::SecurityLevel::ClearText;
	}
	
	// Until participant list & self devices list is populated, don't assume chat room is safe but encrypted
	if (getParticipants().size() == 0 && getMe()->getPrivate()->getDevices().size() == 0) {
		lInfo() << "Chatroom SecurityLevel = Encrypted";
		return AbstractChatRoom::SecurityLevel::Encrypted;
	}

	bool isSafe = true;
	// check other participants
	for (const auto &participant : getParticipants()) {
		auto level = participant->getSecurityLevel();
		// Note: the algorithm implemented is not actually doing what it says and we may exit on the first Unsafe participant
		// while we also have a ClearText one
		// It actually never occurs because in a ciphered chatroom, no one can be set as ClearText except the local
		// device when it turns off lime after joining the chatroom and this status is thus intercepted before landing here.
		switch (level) {
			case AbstractChatRoom::SecurityLevel::Unsafe:
				lInfo() << "Chatroom SecurityLevel = Unsafe";
				return level; // if one participant is Unsafe the whole chatroom is Unsafe
			case AbstractChatRoom::SecurityLevel::ClearText:
				lInfo() << "Chatroom securityLevel = ClearText";
				return level; // if one participant is ClearText the whole chatroom is ClearText
			case AbstractChatRoom::SecurityLevel::Encrypted:
				isSafe = false; // if one participant is Encrypted the whole chatroom is Encrypted
				break;
			case AbstractChatRoom::SecurityLevel::Safe:
				break; // if all participants are Safe the whole chatroom is Safe
		}
	}

	// check self other devices
	for (const auto &selfDevice : getMe()->getDevices()) {
		if (selfDevice->getAddress() != getLocalAddress()) { // ignore local device
			auto level = selfDevice->getSecurityLevel();
			switch (level) {
				case AbstractChatRoom::SecurityLevel::Unsafe:
					return level; // if one device is Unsafe the whole participant is Unsafe
				case AbstractChatRoom::SecurityLevel::ClearText:
					return level; // if one device is ClearText the whole participant is ClearText
				case AbstractChatRoom::SecurityLevel::Encrypted:
					isSafe = false; // if one device is Encrypted the whole participant is Encrypted
					break;
				case AbstractChatRoom::SecurityLevel::Safe:
					break; // if all devices are Safe the whole participant is Safe
			}
		}
	}

	if (isSafe) {
		lInfo() << "Chatroom SecurityLevel = Safe";
		return AbstractChatRoom::SecurityLevel::Safe;
	} else {
		lInfo() << "Chatroom SecurityLevel = Encrypted";
		return AbstractChatRoom::SecurityLevel::Encrypted;
	}
}

bool ClientGroupChatRoom::hasBeenLeft () const {
	return (getState() != State::Created);
}

bool ClientGroupChatRoom::canHandleParticipants () const {
	return RemoteConference::canHandleParticipants();
}

const ConferenceAddress &ClientGroupChatRoom::getConferenceAddress () const {
	return RemoteConference::getConferenceAddress();
}

void ClientGroupChatRoom::deleteFromDb () {
	L_D();
	if (!hasBeenLeft()) {
		d->deletionOnTerminationEnabled = true;
		leave();
		return;
	}
	d->chatRoomListener->onChatRoomDeleteRequested(d->proxyChatRoom ? d->proxyChatRoom->getSharedFromThis() : getSharedFromThis());
}

list<shared_ptr<EventLog>> ClientGroupChatRoom::getHistory (int nLast) const {
	L_D();
	return getCore()->getPrivate()->mainDb->getHistory(
		ChatRoom::getConferenceId(),
		nLast,
		(d->capabilities & Capabilities::OneToOne) ?
			MainDb::Filter::ConferenceChatMessageSecurityFilter :
			MainDb::FilterMask({MainDb::Filter::ConferenceChatMessageFilter, MainDb::Filter::ConferenceInfoNoDeviceFilter})
	);
}

list<shared_ptr<EventLog>> ClientGroupChatRoom::getHistoryRange (int begin, int end) const {
	L_D();
	return getCore()->getPrivate()->mainDb->getHistoryRange(
		ChatRoom::getConferenceId(),
		begin,
		end,
		(d->capabilities & Capabilities::OneToOne) ?
			MainDb::Filter::ConferenceChatMessageSecurityFilter :
			MainDb::FilterMask({MainDb::Filter::ConferenceChatMessageFilter, MainDb::Filter::ConferenceInfoNoDeviceFilter})
	);
}

int ClientGroupChatRoom::getHistorySize () const {
	L_D();
	return getCore()->getPrivate()->mainDb->getHistorySize(
		ChatRoom::getConferenceId(),
		(d->capabilities & Capabilities::OneToOne) ?
			MainDb::Filter::ConferenceChatMessageSecurityFilter :
			MainDb::FilterMask({MainDb::Filter::ConferenceChatMessageFilter, MainDb::Filter::ConferenceInfoNoDeviceFilter})
	);
}

bool ClientGroupChatRoom::addParticipant (const IdentityAddress &addr, const CallSessionParams *params, bool hasMedia) {
	list<IdentityAddress> addressesList({addr});

	return addParticipants(addressesList, params, hasMedia);
}

bool ClientGroupChatRoom::addParticipants (
	const list<IdentityAddress> &addresses,
	const CallSessionParams *params,
	bool hasMedia
) {
	L_D();

	if ((getState() != ChatRoom::State::Instantiated) && (getState() != ChatRoom::State::Created)) {
		lError() << "Cannot add participants to the ClientGroupChatRoom in a state other than Instantiated or Created";
		return false;
	}

	list<IdentityAddress> addressesList = d->cleanAddressesList(addresses);
	if (addressesList.empty()) {
		lError() << "No participants given.";
		return false;
	}
	if (getCapabilities() & ClientGroupChatRoom::Capabilities::OneToOne &&
	    (addressesList.size() > 1 || getParticipantCount() != 0)) {
		lError() << "Cannot add more than one participant in a one-to-one chatroom";
		return false;
	}

	if (getState() == ChatRoom::State::Instantiated) {
		Content content;
		content.setBody(getResourceLists(addressesList));
		content.setContentType(ContentType::ResourceLists);
		content.setContentDisposition(ContentDisposition::RecipientList);
		if (linphone_core_content_encoding_supported(getCore()->getCCore(), "deflate")) {
			content.setContentEncoding("deflate");
		}

		auto session = d->createSession();
		session->startInvite(nullptr, getSubject(), &content);
		d->setState(ChatRoom::State::CreationPending);
	} else {
		SalReferOp *referOp = new SalReferOp(getCore()->getCCore()->sal);
		LinphoneAddress *lAddr = linphone_address_new(getConferenceAddress().asString().c_str());
		linphone_configure_op(getCore()->getCCore(), referOp, lAddr, nullptr, true);
		linphone_address_unref(lAddr);
		for (const auto &addr : addresses) {
			Address referToAddr = addr;
			referToAddr.setParam("text");
			referOp->sendRefer(referToAddr.getInternalAddress());
		}
		referOp->unref();
	}
	return true;
}

bool ClientGroupChatRoom::removeParticipant (const shared_ptr<Participant> &participant) {
	LinphoneCore *cCore = getCore()->getCCore();

	//TODO handle one-to-one case ?
	SalReferOp *referOp = new SalReferOp(cCore->sal);
	LinphoneAddress *lAddr = linphone_address_new(getConferenceAddress().asString().c_str());
	linphone_configure_op(cCore, referOp, lAddr, nullptr, false);
	linphone_address_unref(lAddr);
	Address referToAddr = participant->getAddress();
	referToAddr.setParam("text");
	referToAddr.setUriParam("method", "BYE");
	referOp->sendRefer(referToAddr.getInternalAddress());
	referOp->unref();

	return true;
}

bool ClientGroupChatRoom::removeParticipants (const list<shared_ptr<Participant>> &participants) {
	return RemoteConference::removeParticipants(participants);
}

shared_ptr<Participant> ClientGroupChatRoom::findParticipant (const IdentityAddress &addr) const {
	return RemoteConference::findParticipant(addr);
}

shared_ptr<Participant> ClientGroupChatRoom::getMe () const {
	return RemoteConference::getMe();
}

int ClientGroupChatRoom::getParticipantCount () const {
	return RemoteConference::getParticipantCount();
}

const list<shared_ptr<Participant>> &ClientGroupChatRoom::getParticipants () const {
	return RemoteConference::getParticipants();
}

void ClientGroupChatRoom::setParticipantAdminStatus (const shared_ptr<Participant> &participant, bool isAdmin) {
	if (isAdmin == participant->isAdmin())
		return;

	if (!getMe()->isAdmin()) {
		lError() << "Cannot change the participant admin status because I am not admin";
		return;
	}

	LinphoneCore *cCore = getCore()->getCCore();

	SalReferOp *referOp = new SalReferOp(cCore->sal);
	LinphoneAddress *lAddr = linphone_address_new(getConferenceAddress().asString().c_str());
	linphone_configure_op(cCore, referOp, lAddr, nullptr, false);
	linphone_address_unref(lAddr);
	Address referToAddr = participant->getAddress();
	referToAddr.setParam("text");
	referToAddr.setParam("admin", Utils::toString(isAdmin));
	referOp->sendRefer(referToAddr.getInternalAddress());
	referOp->unref();
}

const string &ClientGroupChatRoom::getSubject () const {
	return RemoteConference::getSubject();
}

void ClientGroupChatRoom::setSubject (const string &subject) {
	L_D();

	if (getState() != ChatRoom::State::Created) {
		lError() << "Cannot change the ClientGroupChatRoom subject in a state other than Created";
		return;
	}

	if (!getMe()->isAdmin()) {
		lError() << "Cannot change the ClientGroupChatRoom subject because I am not admin";
		return;
	}

	shared_ptr<CallSession> session = focus->getSession();
	if (session)
		session->update(nullptr, subject);
	else {
		session = d->createSession();
		session->startInvite(nullptr, subject, nullptr);
	}
}

void ClientGroupChatRoom::join () {
	L_D();

	shared_ptr<CallSession> session = focus->getSession();
	if (!session && ((getState() == ChatRoom::State::Instantiated) || (getState() == ChatRoom::State::Terminated))) {
		session = d->createSession();
	}
	if (session) {
		if (getState() != ChatRoom::State::TerminationPending)
			session->startInvite(nullptr, "", nullptr);
		if (getState() != ChatRoom::State::Created)
			d->setState(ChatRoom::State::CreationPending);
	}
}

void ClientGroupChatRoom::leave () {
	L_D();

	eventHandler->unsubscribe();
	shared_ptr<CallSession> session = focus->getSession();
	if (session)
		session->terminate();
	else {
		session = d->createSession();
		session->startInvite(nullptr, "", nullptr);
	}

	d->setState(ChatRoom::State::TerminationPending);
}

// -----------------------------------------------------------------------------

void ClientGroupChatRoom::onConferenceCreated (const ConferenceAddress &addr) {
	L_D();
	conferenceAddress = addr;
	focus->setAddress(addr);
	focus->clearDevices();
	focus->addDevice(addr);
	d->conferenceId = ConferenceId(addr, d->conferenceId.getLocalAddress());
	d->chatRoomListener->onChatRoomInsertRequested(getSharedFromThis());
	d->setState(ChatRoom::State::Created);
}

void ClientGroupChatRoom::onConferenceKeywordsChanged (const vector<string> &keywords) {
	L_D();
	if (find(keywords.cbegin(), keywords.cend(), "one-to-one") != keywords.cend())
		d->capabilities |= ClientGroupChatRoom::Capabilities::OneToOne;
}

void ClientGroupChatRoom::onConferenceTerminated (const IdentityAddress &addr) {
	L_D();

	eventHandler->unsubscribe();
	resetLastNotify();
	//remove event handler from list event handler if used
	if (d->listHandlerUsed && getCore()->getPrivate()->remoteListEventHandler)
		getCore()->getPrivate()->remoteListEventHandler->removeHandler(eventHandler.get());

	d->setState(ChatRoom::State::Terminated);

	auto event = make_shared<ConferenceEvent>(
		EventLog::Type::ConferenceTerminated,
		time(nullptr),
		d->conferenceId
	);
	d->addEvent(event);

	LinphoneChatRoom *cr = d->getCChatRoom();
	_linphone_chat_room_notify_conference_left(cr, L_GET_C_BACK_PTR(event));

	if (d->deletionOnTerminationEnabled) {
		d->deletionOnTerminationEnabled = false;
		d->chatRoomListener->onChatRoomDeleteRequested(d->proxyChatRoom ? d->proxyChatRoom->getSharedFromThis() : getSharedFromThis());
	}
}

void ClientGroupChatRoom::onFirstNotifyReceived (const IdentityAddress &addr) {
	L_D();

	if (getState() != ChatRoom::State::Created) {
		lWarning() << "First notify received in ClientGroupChatRoom that is not in the Created state ["
			<< getState() << "], ignoring it!";
		return;
	}

	bool performMigration = false;
	shared_ptr<AbstractChatRoom> chatRoom;
	if (getParticipantCount() == 1 && d->capabilities & ClientGroupChatRoom::Capabilities::OneToOne) {
		//ConferenceId id(getParticipants().front()->getAddress(), getMe()->getAddress());
		chatRoom = getCore()->findOneToOneChatRoom(getMe()->getAddress(), getParticipants().front()->getAddress(), true, d->capabilities & ClientGroupChatRoom::Capabilities::Encrypted);

		if (chatRoom) {
			auto capabilities = chatRoom->getCapabilities();

			if (getCore()->getPrivate()->basicToFlexisipChatroomMigrationEnabled() && (capabilities & ChatRoom::Capabilities::Basic) && (capabilities & ChatRoom::Capabilities::Migratable)) {
				performMigration = true;
			}
		}
	}

	if (performMigration) {
		BasicToClientGroupChatRoom::migrate(getSharedFromThis(), chatRoom);
	}
	else {
		d->chatRoomListener->onChatRoomInsertInDatabaseRequested(getSharedFromThis());
	}

	auto event = make_shared<ConferenceEvent>(
		EventLog::Type::ConferenceCreated,
		time(nullptr),
		d->conferenceId
	);

	bool_t forceFullState = linphone_config_get_bool(linphone_core_get_config(getCore()->getCCore()), "misc", "conference_event_package_force_full_state",FALSE );
	if (!forceFullState) //to avoid this event to be repeated for each full state
		d->addEvent(event);

	LinphoneChatRoom *cr = d->getCChatRoom();
	_linphone_chat_room_notify_conference_joined(cr, L_GET_C_BACK_PTR(event));

	d->bgTask.stop();
}

void ClientGroupChatRoom::onParticipantAdded (const shared_ptr<ConferenceParticipantEvent> &event) {
	L_D();

	const IdentityAddress &addr = event->getParticipantAddress();
	if (isMe(addr))
		return;

	shared_ptr<Participant> participant = findParticipant(addr);
	if (participant) {
		lWarning() << "Participant " << participant << " added but already in the list of participants!";
		return;
	}

	participant = Participant::create(this,addr);
	participants.push_back(participant);

	if (event->getFullState())
		return;

	d->addEvent(event);

	LinphoneChatRoom *cr = d->getCChatRoom();
	_linphone_chat_room_notify_participant_added(cr, L_GET_C_BACK_PTR(event));
}

void ClientGroupChatRoom::onParticipantRemoved (const shared_ptr<ConferenceParticipantEvent> &event) {
	L_D();

	const IdentityAddress &addr = event->getParticipantAddress();
	shared_ptr<Participant> participant = findParticipant(addr);
	if (!participant) {
		lWarning() << "Participant " << addr.asString() << " removed but not in the list of participants!";
		return;
	}

	participants.remove(participant);
	d->addEvent(event);

	LinphoneChatRoom *cr = d->getCChatRoom();
	_linphone_chat_room_notify_participant_removed(cr, L_GET_C_BACK_PTR(event));
}

void ClientGroupChatRoom::onParticipantSetAdmin (const shared_ptr<ConferenceParticipantEvent> &event) {
	L_D();

	const IdentityAddress &addr = event->getParticipantAddress();
	shared_ptr<Participant> participant;
	if (isMe(addr))
		participant = getMe();
	else
		participant = findParticipant(addr);
	if (!participant) {
		lWarning() << "Participant " << addr.asString() << " admin status has been changed but is not in the list of participants!";
		return;
	}

	bool isAdmin = event->getType() == EventLog::Type::ConferenceParticipantSetAdmin;
	if (participant->isAdmin() == isAdmin)
		return; // No change in the local admin status, do not notify
	participant->setAdmin(isAdmin);

	if (event->getFullState())
		return;

	d->addEvent(event);

	LinphoneChatRoom *cr = d->getCChatRoom();
	_linphone_chat_room_notify_participant_admin_status_changed(cr, L_GET_C_BACK_PTR(event));
}

void ClientGroupChatRoom::onSecurityEvent (const shared_ptr<ConferenceSecurityEvent> &event) {
	L_D();
	shared_ptr<ConferenceSecurityEvent> finalEvent = nullptr;
	shared_ptr<ConferenceSecurityEvent> cleanEvent = nullptr;

	// Remove faulty device if its address is invalid
	IdentityAddress faultyDevice = event->getFaultyDeviceAddress();
	if (!faultyDevice.isValid()) {
		cleanEvent = make_shared<ConferenceSecurityEvent>(
			event->getCreationTime(),
			event->getConferenceId(),
			event->getSecurityEventType()
		);
	}
	finalEvent = cleanEvent ? cleanEvent : event;

	d->addEvent(event);

	LinphoneChatRoom *cr = d->getCChatRoom();
	_linphone_chat_room_notify_security_event(cr, L_GET_C_BACK_PTR(event));
}

void ClientGroupChatRoom::onSubjectChanged (const shared_ptr<ConferenceSubjectEvent> &event) {
	L_D();

	if (getSubject() == event->getSubject())
		return; // No change in the local subject, do not notify
	RemoteConference::setSubject(event->getSubject());

	if (event->getFullState())
		return;

	d->addEvent(event);

	LinphoneChatRoom *cr = d->getCChatRoom();
	_linphone_chat_room_notify_subject_changed(cr, L_GET_C_BACK_PTR(event));
	linphone_core_notify_chat_room_subject_changed(linphone_chat_room_get_core(cr), cr);
}

void ClientGroupChatRoom::onParticipantDeviceAdded (const shared_ptr<ConferenceParticipantDeviceEvent> &event) {
	L_D();

	const IdentityAddress &addr = event->getParticipantAddress();
	shared_ptr<Participant> participant;
	if (isMe(addr))
		participant = getMe();
	else
		participant = findParticipant(addr);
	if (!participant) {
		lWarning() << "Participant " << addr.asString() << " added a device but is not in the list of participants!";
		return;
	}

	ChatRoom::SecurityLevel currentSecurityLevel = getSecurityLevel();
	shared_ptr<ParticipantDevice> device = participant->addDevice(event->getDeviceAddress());
	const string &deviceName = event->getDeviceName();
	if (!deviceName.empty())
		device->setName(deviceName);

	// Check if new device degrades the chatroom security level and return corresponding security event
	shared_ptr<ConferenceSecurityEvent> securityEvent = nullptr;

	auto encryptionEngine = getCore()->getEncryptionEngine();
	if (encryptionEngine)
		securityEvent = encryptionEngine->onDeviceAdded(event->getDeviceAddress(), participant, getSharedFromThis(), currentSecurityLevel);

	if (event->getFullState())
		return;

	d->addEvent(event);

	if (securityEvent) onSecurityEvent(securityEvent);

	LinphoneChatRoom *cr = d->getCChatRoom();
	_linphone_chat_room_notify_participant_device_added(cr, L_GET_C_BACK_PTR(event));
}

void ClientGroupChatRoom::onParticipantDeviceRemoved (const shared_ptr<ConferenceParticipantDeviceEvent> &event) {
	L_D();

	const IdentityAddress &addr = event->getParticipantAddress();
	shared_ptr<Participant> participant;
	if (isMe(addr))
		participant = getMe();
	else
		participant = findParticipant(addr);
	if (!participant) {
		lWarning() << "Participant " << addr.asString() << " removed a device but is not in the list of participants!";
		return;
	}
	participant->removeDevice(event->getDeviceAddress());
	d->addEvent(event);

	LinphoneChatRoom *cr = d->getCChatRoom();
	_linphone_chat_room_notify_participant_device_removed(cr, L_GET_C_BACK_PTR(event));
}

void ClientGroupChatRoom::onParticipantsCleared () {
	//clear from db as well
	for (const auto &participant : participants) {
		getCore()->getPrivate()->mainDb->deleteChatRoomParticipant(getSharedFromThis(), participant->getAddress());
		for (const auto &device : participant->getDevices())
			getCore()->getPrivate()->mainDb->deleteChatRoomParticipantDevice(getSharedFromThis(), device);
	}
	participants.clear();

}

void ClientGroupChatRoom::enableEphemeral (bool ephem, bool updateDb) {
	L_D();
	d->isEphemeral = ephem;
	const string active = ephem ? "enabled" : "disabled";
	lDebug() << "Ephemeral message is " << active << " in chat room [" << d->conferenceId << "]";
	if (updateDb) {
		getCore()->getPrivate()->mainDb->updateChatRoomEphemeralEnabled(d->conferenceId, ephem);
		shared_ptr<ConferenceEphemeralMessageEvent> event;
		if (ephem)
			event = make_shared<ConferenceEphemeralMessageEvent>(EventLog::Type::ConferenceEphemeralMessageEnabled, time(nullptr), d->conferenceId, d->ephemeralLifetime);
		else
			event = make_shared<ConferenceEphemeralMessageEvent>(EventLog::Type::ConferenceEphemeralMessageDisabled, time(nullptr), d->conferenceId, d->ephemeralLifetime);
		d->addEvent(event);

		LinphoneChatRoom *cr = d->getCChatRoom();
		_linphone_chat_room_notify_ephemeral_event(cr, L_GET_C_BACK_PTR(event));
	}
}

bool ClientGroupChatRoom::ephemeralEnabled() const {
	L_D();
	return d->isEphemeral;
}

void ClientGroupChatRoom::setEphemeralLifetime (long lifetime, bool updateDb) {
	L_D();
	if (lifetime == d->ephemeralLifetime) {
		if (updateDb)
			lWarning() << "Ephemeral lifetime will not be changed! Trying to set the same ephemaral lifetime as before : " << lifetime;
		return;
	}
	d->ephemeralLifetime = lifetime;

	if (updateDb) {
		lInfo() << "Set new ephemeral lifetime " << lifetime << ", used to be " << d->ephemeralLifetime << ".";
		getCore()->getPrivate()->mainDb->updateChatRoomEphemeralLifetime(d->conferenceId, lifetime);

		if (d->isEphemeral) { // Do not create event if ephemeral feature is disabled
			shared_ptr<ConferenceEphemeralMessageEvent> event = make_shared<ConferenceEphemeralMessageEvent>(EventLog::Type::ConferenceEphemeralMessageLifetimeChanged, time(nullptr), d->conferenceId, lifetime);
			d->addEvent(event);

			LinphoneChatRoom *cr = d->getCChatRoom();
			_linphone_chat_room_notify_ephemeral_event(cr, L_GET_C_BACK_PTR(event));
		}
	}
}

long ClientGroupChatRoom::getEphemeralLifetime () const {
	L_D();
	return d->ephemeralLifetime;
}

bool ClientGroupChatRoom::ephemeralSupportedByAllParticipants () const {
	// TODO
	return false;
}

LINPHONE_END_NAMESPACE
