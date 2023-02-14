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

#include <bctoolbox/defs.h>

#include "linphone/utils/utils.h"

#include "basic-chat-room-p.h"
#include "call/call.h"
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

	d->me = Participant::create(nullptr, conferenceId.getLocalAddress());
	d->participants.push_back(Participant::create(nullptr, conferenceId.getPeerAddress()));

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
	bool cpimAllowedInBasicChatRooms = false;

	LinphoneCore *lc = getCore()->getCCore();
	Address addr(conferenceId.getLocalAddress().asAddress());
	LinphoneAccount *account = linphone_core_lookup_account_by_identity(lc, L_GET_C_BACK_PTR(&addr));
	if (account) {
		const LinphoneAccountParams *params = linphone_account_get_params(account);
		cpimAllowedInBasicChatRooms = linphone_account_params_cpim_in_basic_chat_room_enabled(params);
	}

	return d->cpimAllowed || cpimAllowedInBasicChatRooms;
}

bool BasicChatRoom::canHandleMultipart () const {
	L_D();
	return d->multipartAllowed;
}

BasicChatRoom::CapabilitiesMask BasicChatRoom::getCapabilities () const {
	shared_ptr<Call> call = getCall();
	if (call && call->getCurrentParams()->realtimeTextEnabled()) {
		return { Capabilities::Basic, Capabilities::OneToOne, Capabilities::RealTimeText };
	}
	return { Capabilities::Basic, Capabilities::OneToOne };
}

bool BasicChatRoom::hasBeenLeft () const {
	return false;
}

bool BasicChatRoom::isReadOnly () const {
	return false;
}

const ConferenceAddress &BasicChatRoom::getConferenceAddress () const {
	lError() << "a BasicChatRoom does not have a conference address";
	return Utils::getEmptyConstRefObject<ConferenceAddress>();
}

bool BasicChatRoom::addParticipant (
	UNUSED(std::shared_ptr<Call> call)
) {
	lError() << "addParticipant() is not allowed on a BasicChatRoom";
	return false;
}

bool BasicChatRoom::addParticipant (
	UNUSED(const IdentityAddress &participantAddress)
) {
	lError() << "addParticipant() is not allowed on a BasicChatRoom";
	return false;
}

bool BasicChatRoom::addParticipants (
	UNUSED(const list<IdentityAddress> &addresses)
) {
	lError() << "addParticipants() is not allowed on a BasicChatRoom";
	return false;
}

bool BasicChatRoom::removeParticipant (UNUSED(const shared_ptr<Participant> &participantAddress)) {
	lError() << "removeParticipant() is not allowed on a BasicChatRoom";
	return false;
}

bool BasicChatRoom::removeParticipants (UNUSED(const list<shared_ptr<Participant>> &participantAddress)) {
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

const list<shared_ptr<ParticipantDevice>> BasicChatRoom::getParticipantDevices () const {
	list<shared_ptr<ParticipantDevice>> devices;
	for (const auto & p : getParticipants()) {
		const auto & d = p->getDevices();
		if (!d.empty()) {
			devices.insert(devices.begin(), d.begin(), d.end());
		}
	}
	return devices;
}

const list<shared_ptr<Participant>> &BasicChatRoom::getParticipants () const {
	L_D();
	return d->participants;
}

void BasicChatRoom::setParticipantAdminStatus (const shared_ptr<Participant> &, bool) {
	lError() << "setParticipantAdminStatus() is not allowed on a BasicChatRoom";
}

const string & BasicChatRoom::getSubject () const {
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

void BasicChatRoom::join (UNUSED(const IdentityAddress &participantAddress)) {
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

void BasicChatRoom::setState (ConferenceInterface::State newState) {
	L_D();

	if (getState() != newState) {
		state = newState;
		d->notifyStateChanged();
	}
}

ConferenceInterface::State BasicChatRoom::getState () const {
	return state;
}
LINPHONE_END_NAMESPACE
