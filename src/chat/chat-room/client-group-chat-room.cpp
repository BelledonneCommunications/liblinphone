/*
 * client-group-chat-room.cpp
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

#include "linphone/utils/utils.h"

#include "address/address-p.h"
#include "basic-to-client-group-chat-room.h"
#include "c-wrapper/c-wrapper.h"
#include "client-group-chat-room-p.h"
#include "conference/handlers/remote-conference-event-handler-p.h"
#include "conference/handlers/remote-conference-list-event-handler.h"
#include "conference/participant-p.h"
#include "conference/participant-device.h"
#include "conference/remote-conference-p.h"
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

list<IdentityAddress> ClientGroupChatRoomPrivate::cleanAddressesList (const list<IdentityAddress> &addresses) const {
	L_Q();
	list<IdentityAddress> cleanedList(addresses);
	cleanedList.sort();
	cleanedList.unique();
	for (auto it = cleanedList.begin(); it != cleanedList.end();) {
		if (q->findParticipant(*it) || (q->getMe()->getAddress() == *it))
			it = cleanedList.erase(it);
		else
			it++;
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

	ParticipantPrivate *dFocus = qConference->getPrivate()->focus->getPrivate();
	shared_ptr<CallSession> session = dFocus->createSession(*q, &csp, false, callSessionListener);
	Address myCleanedAddress(q->getMe()->getAddress());
	myCleanedAddress.removeUriParam("gr"); // Remove gr parameter for INVITE.
	session->configure(LinphoneCallOutgoing, nullptr, nullptr, myCleanedAddress, dFocus->getDevices().front()->getAddress());
	session->initiateOutgoing();
	session->getPrivate()->createOp();
	return session;
}

void ClientGroupChatRoomPrivate::notifyReceived (const string &body) {
	L_Q_T(RemoteConference, qConference);
	qConference->getPrivate()->eventHandler->notifyReceived(body);
}

void ClientGroupChatRoomPrivate::multipartNotifyReceived (const string &body) {
	L_Q_T(RemoteConference, qConference);
	qConference->getPrivate()->eventHandler->multipartNotifyReceived(body);
}

// -----------------------------------------------------------------------------

void ClientGroupChatRoomPrivate::setCallSessionListener (CallSessionListener *listener) {
	L_Q();
	L_Q_T(RemoteConference, qConference);

	callSessionListener = listener;
	shared_ptr<CallSession> session = qConference->getPrivate()->focus->getPrivate()->getSession();
	if (session)
		session->getPrivate()->setCallSessionListener(listener);
	for (const auto &participant : q->getParticipants()) {
		session = participant->getPrivate()->getSession();
		if (session)
			session->getPrivate()->setCallSessionListener(listener);
	}
}

void ClientGroupChatRoomPrivate::addOneToOneCapability () {
	capabilities |= ClientGroupChatRoom::Capabilities::OneToOne;
}

unsigned int ClientGroupChatRoomPrivate::getLastNotifyId () const {
	L_Q_T(RemoteConference, qConference);
	return qConference->getPrivate()->eventHandler->getLastNotify();
}

// -----------------------------------------------------------------------------

void ClientGroupChatRoomPrivate::confirmJoining (SalCallOp *op) {
	L_Q();
	L_Q_T(RemoteConference, qConference);

	auto focus = qConference->getPrivate()->focus;
	bool previousSession = (focus->getPrivate()->getSession() != nullptr);
	auto session = focus->getPrivate()->createSession(*q, nullptr, false, this);
	session->configure(LinphoneCallIncoming, nullptr, op, Address(op->getFrom()), Address(op->getTo()));
	session->startIncomingNotification(false);

	if (!previousSession) {
		setState(ClientGroupChatRoom::State::CreationPending);
		// Handle participants addition
		list<IdentityAddress> identAddresses = ClientGroupChatRoom::parseResourceLists(op->getRemoteBody());
		for (const auto &addr : identAddresses) {
			auto participant = q->findParticipant(addr);
			if (!participant) {
				participant = make_shared<Participant>(q, addr);
				qConference->getPrivate()->participants.push_back(participant);
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

	unsigned int notifyId = qConference->getPrivate()->eventHandler->getLastNotify();;
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

	ParticipantPrivate *participantPrivate = qConference->getPrivate()->focus->getPrivate();
	if (session == participantPrivate->getSession())
		participantPrivate->removeSession();
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
		} else if (q->getState() == ChatRoom::State::TerminationPending)
			qConference->getPrivate()->focus->getPrivate()->getSession()->terminate();
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
				// Somehow the chat room is no longer know on the server, so terminate it
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

	IdentityAddress addr(remoteContact);
	q->onConferenceCreated(addr);
	if (remoteContact.hasParam("isfocus")) {
		if (q->getCore()->getPrivate()->remoteListEventHandler->findHandler(q->getConferenceId())) {
			q->getCore()->getPrivate()->remoteListEventHandler->subscribe();
		} else {
			bgTask.start(q->getCore(), 32); // It will be stopped when receiving the first notify
			qConference->getPrivate()->eventHandler->subscribe(q->getConferenceId());
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
	bool encrypted
) :
ChatRoom(*new ClientGroupChatRoomPrivate, core, conferenceId),
RemoteConference(core, conferenceId.getLocalAddress(), nullptr) {
	L_D();
	L_D_T(RemoteConference, dConference);
	RemoteConference::setSubject(subject);
	for (const auto &addr : Conference::parseResourceLists(content))
		dConference->participants.push_back(make_shared<Participant>(this, addr));

	if (encrypted)
		d->capabilities |= ClientGroupChatRoom::Capabilities::Encrypted;

	dConference->focus = make_shared<Participant>(this, focus);
	dConference->focus->getPrivate()->addDevice(focus);
}

ClientGroupChatRoom::ClientGroupChatRoom (
	const shared_ptr<Core> &core,
	const string &factoryUri,
	const IdentityAddress &me,
	const string &subject,
	bool encrypted
) : ClientGroupChatRoom(
	core,
	IdentityAddress(factoryUri),
	ConferenceId(IdentityAddress(), me),
	subject,
	Content(),
	encrypted
) {}

ClientGroupChatRoom::ClientGroupChatRoom (
	const shared_ptr<Core> &core,
	const ConferenceId &conferenceId,
	shared_ptr<Participant> &me,
	AbstractChatRoom::CapabilitiesMask capabilities,
	const string &subject,
	list<shared_ptr<Participant>> &&participants,
	unsigned int lastNotifyId,
	bool hasBeenLeft
) : ChatRoom(*new ClientGroupChatRoomPrivate, core, conferenceId),
RemoteConference(core, me->getAddress(), nullptr) {
	L_D();
	L_D_T(RemoteConference, dConference);

	d->capabilities |=
		(capabilities & ClientGroupChatRoom::Capabilities::OneToOne) |
		(capabilities & ClientGroupChatRoom::Capabilities::Encrypted);
	const IdentityAddress &peerAddress = conferenceId.getPeerAddress();
	dConference->focus = make_shared<Participant>(this, peerAddress);
	dConference->focus->getPrivate()->addDevice(peerAddress);
	dConference->conferenceAddress = peerAddress;
	dConference->subject = subject;
	dConference->participants = move(participants);

	getMe()->getPrivate()->setAdmin(me->isAdmin());

	dConference->eventHandler->setConferenceId(conferenceId);
	
	bool_t forceFullState = linphone_config_get_bool(linphone_core_get_config(getCore()->getCCore()), "misc", "conference_event_package_force_full_state",FALSE );
	dConference->eventHandler->setLastNotify(forceFullState?0:lastNotifyId);
	lInfo() << "Last notify set to [" << dConference->eventHandler->getLastNotify() << "] for conference [" << dConference << "]";

	if (!hasBeenLeft)
		getCore()->getPrivate()->remoteListEventHandler->addHandler(dConference->eventHandler.get());
}

ClientGroupChatRoom::~ClientGroupChatRoom () {
	L_D();
	L_D_T(RemoteConference, dConference);

	try {
		if (getCore()->getPrivate()->remoteListEventHandler)
			getCore()->getPrivate()->remoteListEventHandler->removeHandler(dConference->eventHandler.get());
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

	bool isSafe = true;
	for (const auto &participant : getParticipants()) {
		auto level = participant->getSecurityLevel();
		switch (level) {
			case AbstractChatRoom::SecurityLevel::Unsafe:
				lInfo() << "Chatroom SecurityLevel = Unsafe";
				return level; // if one device is Unsafe the whole participant is Unsafe
			case AbstractChatRoom::SecurityLevel::ClearText:
				lInfo() << "Chatroom securityLevel = ClearText";
				return level; // if one device is ClearText the whole participant is ClearText
			case AbstractChatRoom::SecurityLevel::Encrypted:
				isSafe = false; // if one device is Encrypted the whole participant is Encrypted
				break;
			case AbstractChatRoom::SecurityLevel::Safe:
				break; // if all devices are Safe the whole participant is Safe
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

const IdentityAddress &ClientGroupChatRoom::getConferenceAddress () const {
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
		getConferenceId(),
		nLast,
		(d->capabilities & Capabilities::OneToOne) ?
			MainDb::Filter::ConferenceChatMessageSecurityFilter :
			MainDb::FilterMask({MainDb::Filter::ConferenceChatMessageFilter, MainDb::Filter::ConferenceInfoNoDeviceFilter})
	);
}

list<shared_ptr<EventLog>> ClientGroupChatRoom::getHistoryRange (int begin, int end) const {
	L_D();
	return getCore()->getPrivate()->mainDb->getHistoryRange(
		getConferenceId(),
		begin,
		end,
		(d->capabilities & Capabilities::OneToOne) ?
			MainDb::Filter::ConferenceChatMessageSecurityFilter :
			MainDb::FilterMask({MainDb::Filter::ConferenceChatMessageFilter, MainDb::Filter::ConferenceInfoNoDeviceFilter})
	);
}

bool ClientGroupChatRoom::addParticipant (const IdentityAddress &addr, const CallSessionParams *params, bool hasMedia) {
	L_D();

	if ((getState() != ChatRoom::State::Instantiated) && (getState() != ChatRoom::State::Created)) {
		lError() << "Cannot add participants to the ClientGroupChatRoom in a state other than Instantiated or Created";
		return false;
	}

	if ((getState() == ChatRoom::State::Created) && (d->capabilities & ClientGroupChatRoom::Capabilities::OneToOne)) {
		lError() << "Cannot add more participants to a OneToOne ClientGroupChatRoom";
		return false;
	}

	LinphoneCore *cCore = getCore()->getCCore();
	if (getState() == ChatRoom::State::Instantiated) {
		list<IdentityAddress> addressesList;
		addressesList.push_back(addr);
		Content content;
		content.setBody(getResourceLists(addressesList));
		content.setContentType(ContentType::ResourceLists);
		content.setContentDisposition(ContentDisposition::RecipientList);

		auto session = d->createSession();
		session->startInvite(nullptr, getSubject(), &content);
		d->setState(ChatRoom::State::CreationPending);
	} else {
		SalReferOp *referOp = new SalReferOp(cCore->sal);
		LinphoneAddress *lAddr = linphone_address_new(getConferenceAddress().asString().c_str());
		linphone_configure_op(cCore, referOp, lAddr, nullptr, true);
		linphone_address_unref(lAddr);
		Address referToAddr = addr;
		referToAddr.setParam("text");
		referOp->sendRefer(referToAddr.getPrivate()->getInternalAddress());
		referOp->unref();
	}

	return true;
}

bool ClientGroupChatRoom::addParticipants (
	const list<IdentityAddress> &addresses,
	const CallSessionParams *params,
	bool hasMedia
) {
	L_D();

	list<IdentityAddress> addressesList = d->cleanAddressesList(addresses);
	if (addressesList.empty()) {
		lError() << "No participants given.";
		return false;
	}

	if ((getState() == ChatRoom::State::Instantiated)
		&& (addressesList.size() == 1)
		&& (linphone_config_get_bool(linphone_core_get_config(L_GET_C_BACK_PTR(getCore())),
			"misc", "one_to_one_chat_room_enabled", TRUE))
	) {
		d->capabilities |= ClientGroupChatRoom::Capabilities::OneToOne;
		const IdentityAddress &participant = addresses.front();
		bool encrypted = getCapabilities() & ClientGroupChatRoom::Capabilities::Encrypted;
		auto existingChatRoom = getCore()->findOneToOneChatRoom(getLocalAddress(), participant, encrypted);
		if (existingChatRoom) {
			const IdentityAddress &me = getMe()->getAddress();
			lError() << "Trying to create already existing " << (encrypted ? "" : "non-") << "encrypted one-to-one chatroom with participants: " <<
				me << ", " << participant;
			return false;
		}
	}

	if (getState() == ChatRoom::State::Instantiated) {
		Content content;
		content.setBody(getResourceLists(addressesList));
		content.setContentType(ContentType::ResourceLists);
		content.setContentDisposition(ContentDisposition::RecipientList);
		if (linphone_core_content_encoding_supported(getCore()->getCCore(), "deflate"))
			content.setContentEncoding("deflate");

		auto session = d->createSession();
		session->startInvite(nullptr, getSubject(), &content);
		d->setState(ChatRoom::State::CreationPending);
	} else {
		for (const auto &addr : addresses)
			addParticipant(addr, params, hasMedia);
	}

	return true;
}

bool ClientGroupChatRoom::removeParticipant (const shared_ptr<Participant> &participant) {
	LinphoneCore *cCore = getCore()->getCCore();

	SalReferOp *referOp = new SalReferOp(cCore->sal);
	LinphoneAddress *lAddr = linphone_address_new(getConferenceAddress().asString().c_str());
	linphone_configure_op(cCore, referOp, lAddr, nullptr, false);
	linphone_address_unref(lAddr);
	Address referToAddr = participant->getAddress();
	referToAddr.setParam("text");
	referToAddr.setUriParam("method", "BYE");
	referOp->sendRefer(referToAddr.getPrivate()->getInternalAddress());
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
	referOp->sendRefer(referToAddr.getPrivate()->getInternalAddress());
	referOp->unref();
}

const string &ClientGroupChatRoom::getSubject () const {
	return RemoteConference::getSubject();
}

void ClientGroupChatRoom::setSubject (const string &subject) {
	L_D();
	L_D_T(RemoteConference, dConference);

	if (getState() != ChatRoom::State::Created) {
		lError() << "Cannot change the ClientGroupChatRoom subject in a state other than Created";
		return;
	}

	if (!getMe()->isAdmin()) {
		lError() << "Cannot change the ClientGroupChatRoom subject because I am not admin";
		return;
	}

	shared_ptr<CallSession> session = dConference->focus->getPrivate()->getSession();
	if (session)
		session->update(nullptr, subject);
	else {
		session = d->createSession();
		session->startInvite(nullptr, subject, nullptr);
	}
}

void ClientGroupChatRoom::join () {
	L_D();
	L_D_T(RemoteConference, dConference);

	shared_ptr<CallSession> session = dConference->focus->getPrivate()->getSession();
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
	L_D_T(RemoteConference, dConference);

	dConference->eventHandler->unsubscribe();
	shared_ptr<CallSession> session = dConference->focus->getPrivate()->getSession();
	if (session)
		session->terminate();
	else {
		session = d->createSession();
		session->startInvite(nullptr, "", nullptr);
	}

	d->setState(ChatRoom::State::TerminationPending);
}

// -----------------------------------------------------------------------------

void ClientGroupChatRoom::onConferenceCreated (const IdentityAddress &addr) {
	L_D();
	L_D_T(RemoteConference, dConference);
	dConference->conferenceAddress = addr;
	dConference->focus->getPrivate()->setAddress(addr);
	dConference->focus->getPrivate()->clearDevices();
	dConference->focus->getPrivate()->addDevice(addr);
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
	L_D_T(RemoteConference, dConference);

	dConference->eventHandler->unsubscribe();
	dConference->eventHandler->resetLastNotify();
	//remove event handler from list event handler if present
	if (getCore()->getPrivate()->remoteListEventHandler)
		getCore()->getPrivate()->remoteListEventHandler->removeHandler(dConference->eventHandler.get());
	
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
			<< Utils::toString(getState()) << "], ignoring it!";
		return;
	}

	bool performMigration = false;
	shared_ptr<AbstractChatRoom> chatRoom;
	if (getParticipantCount() == 1 && d->capabilities & ClientGroupChatRoom::Capabilities::OneToOne) {
		ConferenceId id(getParticipants().front()->getAddress(), getMe()->getAddress());
		chatRoom = getCore()->findChatRoom(id);
		if (chatRoom) {
			auto capabilities = chatRoom->getCapabilities();
			bool migrationEnabled = !!linphone_config_get_int(
				linphone_core_get_config(getCore()->getCCore()),
				"misc",
				"enable_basic_to_client_group_chat_room_migration",
				FALSE
			);
			if (migrationEnabled && (capabilities & ChatRoom::Capabilities::Basic) && (capabilities & ChatRoom::Capabilities::Migratable)) {
				performMigration = true;
			}
		}
	}

	if (performMigration)
		BasicToClientGroupChatRoom::migrate(getSharedFromThis(), chatRoom);
	else
		d->chatRoomListener->onChatRoomInsertInDatabaseRequested(getSharedFromThis());

	auto event = make_shared<ConferenceEvent>(
		EventLog::Type::ConferenceCreated,
		time(nullptr),
		d->conferenceId
	);
	//to be moved into invite transaction
	//d->addEvent(event);

	LinphoneChatRoom *cr = d->getCChatRoom();
	_linphone_chat_room_notify_conference_joined(cr, L_GET_C_BACK_PTR(event));

	d->bgTask.stop();
}

void ClientGroupChatRoom::onParticipantAdded (const shared_ptr<ConferenceParticipantEvent> &event, bool isFullState) {
	L_D();
	L_D_T(RemoteConference, dConference);

	const IdentityAddress &addr = event->getParticipantAddress();
	if (isMe(addr))
		return;

	shared_ptr<Participant> participant = findParticipant(addr);
	if (participant) {
		lWarning() << "Participant " << participant << " added but already in the list of participants!";
		return;
	}

	participant = make_shared<Participant>(this, addr);
	dConference->participants.push_back(participant);

	if (isFullState)
		return;

	d->addEvent(event);

	LinphoneChatRoom *cr = d->getCChatRoom();
	_linphone_chat_room_notify_participant_added(cr, L_GET_C_BACK_PTR(event));
}

void ClientGroupChatRoom::onParticipantRemoved (const shared_ptr<ConferenceParticipantEvent> &event, bool isFullState) {
	(void)isFullState;

	L_D();
	L_D_T(RemoteConference, dConference);

	const IdentityAddress &addr = event->getParticipantAddress();
	shared_ptr<Participant> participant = findParticipant(addr);
	if (!participant) {
		lWarning() << "Participant " << addr.asString() << " removed but not in the list of participants!";
		return;
	}

	dConference->participants.remove(participant);
	d->addEvent(event);

	LinphoneChatRoom *cr = d->getCChatRoom();
	_linphone_chat_room_notify_participant_removed(cr, L_GET_C_BACK_PTR(event));
}

void ClientGroupChatRoom::onParticipantSetAdmin (const shared_ptr<ConferenceParticipantEvent> &event, bool isFullState) {
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
	participant->getPrivate()->setAdmin(isAdmin);

	if (isFullState)
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

void ClientGroupChatRoom::onSubjectChanged (const shared_ptr<ConferenceSubjectEvent> &event, bool isFullState) {
	L_D();

	if (getSubject() == event->getSubject())
		return; // No change in the local subject, do not notify
	RemoteConference::setSubject(event->getSubject());

	if (isFullState)
		return;

	d->addEvent(event);

	LinphoneChatRoom *cr = d->getCChatRoom();
	_linphone_chat_room_notify_subject_changed(cr, L_GET_C_BACK_PTR(event));
}

void ClientGroupChatRoom::onParticipantDeviceAdded (const shared_ptr<ConferenceParticipantDeviceEvent> &event, bool isFullState) {
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
	shared_ptr<ParticipantDevice> device = participant->getPrivate()->addDevice(event->getDeviceAddress());
	const string &deviceName = event->getDeviceName();
	if (!deviceName.empty())
		device->setName(deviceName);

	// Check if new device degrades the chatroom security level and return corresponding security event
	shared_ptr<ConferenceSecurityEvent> securityEvent = nullptr;

	auto encryptionEngine = getCore()->getEncryptionEngine();
	if (encryptionEngine)
		securityEvent = encryptionEngine->onDeviceAdded(event->getDeviceAddress(), participant, getSharedFromThis(), currentSecurityLevel);

	if (isFullState)
		return;

	d->addEvent(event);

	if (securityEvent) onSecurityEvent(securityEvent);

	LinphoneChatRoom *cr = d->getCChatRoom();
	_linphone_chat_room_notify_participant_device_added(cr, L_GET_C_BACK_PTR(event));
}

void ClientGroupChatRoom::onParticipantDeviceRemoved (const shared_ptr<ConferenceParticipantDeviceEvent> &event, bool isFullState) {
	L_D();

	(void)isFullState;

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
	participant->getPrivate()->removeDevice(event->getDeviceAddress());
	d->addEvent(event);

	LinphoneChatRoom *cr = d->getCChatRoom();
	_linphone_chat_room_notify_participant_device_removed(cr, L_GET_C_BACK_PTR(event));
}

void ClientGroupChatRoom::onParticipantsCleared () {
	L_D_T(RemoteConference, dConference);
	//clear from db as well
	for (const auto &participant : dConference->participants) {
		getCore()->getPrivate()->mainDb->deleteChatRoomParticipant(getSharedFromThis(), participant->getAddress());
		for (const auto &device : participant->getPrivate()->getDevices())
			getCore()->getPrivate()->mainDb->deleteChatRoomParticipantDevice(getSharedFromThis(), device);
	}
	dConference->participants.clear();
	
}

LINPHONE_END_NAMESPACE
