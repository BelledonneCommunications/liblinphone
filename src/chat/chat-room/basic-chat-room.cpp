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

#include "basic-chat-room.h"

#include "call/call.h"
#include "conference/conference-params.h"
#include "conference/participant.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "logger/logger.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

BasicChatRoom::BasicChatRoom(const std::shared_ptr<Core> &core,
                             const ConferenceId &conferenceId,
                             const std::shared_ptr<ConferenceParams> &params)
    : ChatRoom(core) {
	mMe = Participant::create(nullptr, conferenceId.getLocalAddress());
	mParticipants.push_back(Participant::create(nullptr, conferenceId.getPeerAddress()));
	mParams = params->clone()->toSharedPtr();

	mConferenceId = conferenceId;
}

void BasicChatRoom::allowCpim(bool value) {
	mCpimAllowed = value;
}

void BasicChatRoom::allowMultipart(bool value) {
	mMultipartAllowed = value;
}

bool BasicChatRoom::canHandleCpim() const {
	bool cpimAllowedInBasicChatRooms = false;

	LinphoneCore *lc = getCore()->getCCore();
	auto addr = mConferenceId.getLocalAddress();
	LinphoneAccount *account = linphone_core_lookup_account_by_identity(lc, addr->toC());
	if (account) {
		const LinphoneAccountParams *params = linphone_account_get_params(account);
		cpimAllowedInBasicChatRooms = linphone_account_params_cpim_in_basic_chat_room_enabled(params);
	}

	return mCpimAllowed || cpimAllowedInBasicChatRooms;
}

bool BasicChatRoom::canHandleMultipart() const {
	return mMultipartAllowed;
}

BasicChatRoom::CapabilitiesMask BasicChatRoom::getCapabilities() const {
	shared_ptr<Call> call = getCall();
	if (call && call->getCurrentParams()->realtimeTextEnabled()) {
		return {Capabilities::Basic, Capabilities::OneToOne, Capabilities::RealTimeText};
	}
	return {Capabilities::Basic, Capabilities::OneToOne};
}

bool BasicChatRoom::hasBeenLeft() const {
	return false;
}

bool BasicChatRoom::isReadOnly() const {
	return false;
}

std::shared_ptr<ConferenceParams> BasicChatRoom::getCurrentParams() const {
	shared_ptr<Call> call = getCall();
	if (call && call->getCurrentParams()) {
		mParams->getChatParams()->setRealTimeText(call->getCurrentParams()->realtimeTextEnabled());
	}
	return mParams;
}

void BasicChatRoom::invalidateAccount() {
	mParams->setAccount(nullptr);
}

std::shared_ptr<Account> BasicChatRoom::getAccount() {
	auto account = mParams->getAccount();
	if (!account) {
		mParams->setAccount(getCore()->findAccountByIdentityAddress(mConferenceId.getLocalAddress()));
	}
	return account;
}

const ConferenceId &BasicChatRoom::getConferenceId() const {
	return mConferenceId;
}

void BasicChatRoom::setConferenceId(const ConferenceId &conferenceId) {
	mConferenceId = conferenceId;
}

std::optional<std::reference_wrapper<const std::string>> BasicChatRoom::getIdentifier() const {
	if (mState == ConferenceInterface::State::Instantiated) {
		return std::nullopt;
	}
	return mConferenceId.getIdentifier();
}

ConferenceInterface::State BasicChatRoom::getState() const {
	return mState;
}

void BasicChatRoom::setState(ConferenceInterface::State newState) {
	if (getState() != newState) {
		mState = newState;
		notifyStateChanged();
	}
}

void BasicChatRoom::setUtf8Subject(const string &subject) {
	mParams->setUtf8Subject(subject);
}

const std::string &BasicChatRoom::getSubjectUtf8() const {
	return mParams->getUtf8Subject();
}

bool BasicChatRoom::isMe(const std::shared_ptr<Address> &address) const {
	return address->weakEqual(*mMe->getAddress());
}

std::shared_ptr<Participant> BasicChatRoom::getMe() const {
	return mMe;
}

std::list<std::shared_ptr<Participant>> BasicChatRoom::getParticipants() const {
	return mParticipants;
}

std::list<std::shared_ptr<Address>> BasicChatRoom::getParticipantAddresses() const {
	list<std::shared_ptr<Address>> addresses;
	for (auto &participant : mParticipants) {
		addresses.push_back(participant->getAddress());
	}
	return addresses;
}

LINPHONE_END_NAMESPACE
