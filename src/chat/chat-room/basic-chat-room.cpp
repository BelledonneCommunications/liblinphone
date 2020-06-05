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

#include "linphone/utils/utils.h"

#include "basic-chat-room-p.h"
#include "conference/participant.h"
#include "logger/logger.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

BasicChatRoom::BasicChatRoom (const shared_ptr<Core> &core, const ConferenceId &conferenceId, const std::shared_ptr<ChatRoomParams> &params) :
	BasicChatRoom(*new BasicChatRoomPrivate, core, conferenceId, params) {
}

BasicChatRoom::BasicChatRoom (
	BasicChatRoomPrivate &p,
	const std::shared_ptr<Core> &core,
	const ConferenceId &conferenceId,
	const std::shared_ptr<ChatRoomParams> &params
) : ChatRoom(p, core, params) {
	L_D();

	d->me = Participant::create(nullptr, getLocalAddress());
	d->participants.push_back(Participant::create(nullptr, getPeerAddress()));

	this->conferenceId = conferenceId;
}

void BasicChatRoom::allowCpim (bool value) {
	L_D();
	d->cpimAllowed = value;
}

void BasicChatRoom::allowMultipart (bool value) {
	L_D();
	d->multipartAllowed = value;
}

bool BasicChatRoom::canHandleCpim () const {
	L_D();
	return d->cpimAllowed;
}

bool BasicChatRoom::canHandleMultipart () const {
	L_D();
	return d->multipartAllowed;
}

BasicChatRoom::CapabilitiesMask BasicChatRoom::getCapabilities () const {
	return { Capabilities::Basic, Capabilities::OneToOne };
}

bool BasicChatRoom::hasBeenLeft () const {
	return false;
}

bool BasicChatRoom::canHandleParticipants () const {
	return false;
}

const ConferenceAddress &BasicChatRoom::getConferenceAddress () const {
	lError() << "a BasicChatRoom does not have a conference address";
	return Utils::getEmptyConstRefObject<ConferenceAddress>();
}

bool BasicChatRoom::addParticipant (const IdentityAddress &, const CallSessionParams *, bool) {
	lError() << "addParticipant() is not allowed on a BasicChatRoom";
	return false;
}

bool BasicChatRoom::addParticipant (
	std::shared_ptr<Call> call
) {
	lError() << "addParticipant() is not allowed on a BasicChatRoom";
	return false;
}

bool BasicChatRoom::addParticipant (
	const IdentityAddress &participantAddress
) {
	lError() << "addParticipant() is not allowed on a BasicChatRoom";
	return false;
}

bool BasicChatRoom::addParticipants (const list<IdentityAddress> &, const CallSessionParams *, bool) {
	lError() << "addParticipants() is not allowed on a BasicChatRoom";
	return false;
}

bool BasicChatRoom::addParticipants (
	const list<IdentityAddress> &addresses
) {
	lError() << "addParticipants() is not allowed on a BasicChatRoom";
	return false;
}

bool BasicChatRoom::removeParticipant (const shared_ptr<Participant> &) {
	lError() << "removeParticipant() is not allowed on a BasicChatRoom";
	return false;
}

bool BasicChatRoom::removeParticipants (const list<shared_ptr<Participant>> &) {
	lError() << "removeParticipants() is not allowed on a BasicChatRoom";
	return false;
}

shared_ptr<Participant> BasicChatRoom::findParticipant (const IdentityAddress &) const {
	lError() << "findParticipant() is not allowed on a BasicChatRoom";
	return nullptr;
}

shared_ptr<Participant> BasicChatRoom::getMe () const {
	L_D();
	return d->me;
}

int BasicChatRoom::getParticipantCount () const {
	return 1;
}

const list<shared_ptr<Participant>> &BasicChatRoom::getParticipants () const {
	L_D();
	return d->participants;
}

void BasicChatRoom::setParticipantAdminStatus (const shared_ptr<Participant> &, bool) {
	lError() << "setParticipantAdminStatus() is not allowed on a BasicChatRoom";
}

const string &BasicChatRoom::getSubject () const {
	L_D();
	return d->subject;
}

void BasicChatRoom::setSubject (const string &subject) {
	L_D();
	d->subject = subject;
}

void BasicChatRoom::join () {
	lError() << "join() is not allowed on a BasicChatRoom";
}

void BasicChatRoom::join (const IdentityAddress &participantAddress) {
	lError() << "join() is not allowed on a BasicChatRoom";
}

void BasicChatRoom::leave () {
	lError() << "leave() is not allowed on a BasicChatRoom";
}

const ConferenceId &BasicChatRoom::getConferenceId () const {
	return conferenceId;
}

bool BasicChatRoom::update(const ConferenceParamsInterface &newParameters) {
	return ChatRoom::update(newParameters);
}

LINPHONE_END_NAMESPACE
