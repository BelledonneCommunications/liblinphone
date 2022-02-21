/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{
}
 without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "conference-scheduler.h"
#include "participant.h"
#include "chat/chat-message/chat-message-p.h"
#include "core/core-p.h"
#include "content/file-content.h"
#include "account/account.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ConferenceScheduler::ConferenceScheduler (
	const shared_ptr<Core> &core
	) : CoreAccessor(core) {
	mState = State::Idle;
	auto default_account = linphone_core_get_default_account(core->getCCore());
	if (default_account) {
		mAccount =  Account::toCpp(default_account)->getSharedFromThis();
	}
}

ConferenceScheduler::~ConferenceScheduler () {
	if (mSession != nullptr) {
		mSession->setListener(nullptr);
	}
	if (mAccount) {
		mAccount = nullptr;
	}
}

const std::shared_ptr<Account> & ConferenceScheduler::getAccount() const {
	return mAccount;
}

void ConferenceScheduler::setAccount(std::shared_ptr<Account> account) {
	if ((mState == State::Idle) || (mState == State::AllocationPending) || (mState == State::Error)) {
		mAccount = account;
	} else {
		lWarning() << "[Conference Scheduler] Unable to change account because scheduler is in state " << mState;
	}
}

ConferenceScheduler::State ConferenceScheduler::getState () const {
	return mState;
}

void ConferenceScheduler::setState (State newState) {
	mState = newState;
	linphone_conference_scheduler_notify_state_changed(toC(), (LinphoneConferenceSchedulerState)newState);
}

const std::shared_ptr<ConferenceInfo> ConferenceScheduler::getInfo () const {
	return mConferenceInfo;
}

void ConferenceScheduler::setInfo (std::shared_ptr<ConferenceInfo> info) {

	if (!mAccount) {
		auto default_account = linphone_core_get_default_account(getCore()->getCCore());
		if (default_account) {
			mAccount =  Account::toCpp(default_account)->getSharedFromThis();
		}
	}

	const auto creator =  mAccount ? IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(mAccount->getAccountParams()->getIdentityAddress())) : IdentityAddress(linphone_core_get_identity(getCore()->getCCore()));
	if (!creator.isValid()) {
		lWarning() << "[Conference Scheduler] Core address attempting to set conference information!";
		return;
	}

	const auto & participants = info->getParticipants();
	if (participants.empty()) {
		lWarning() << "[Conference Scheduler] Can't create a scheduled conference if no participants are added!";
		setState(State::Error);
		return;
	}

	const auto & organizer = info->getOrganizer();
	const bool participantFound = (std::find(participants.cbegin(), participants.cend(), creator) != participants.cend());
	if ((creator != organizer) && !participantFound) {
		lWarning() << "[Conference Scheduler] Unable to find the address " << creator << " setting the conference information among the list of participants or the organizer (" << info->getOrganizer() << ") of conference " << info->getUri();
		setState(State::Error);
		return;
	}

	LinphoneConferenceInfo * foundConferenceInfo = nullptr;
#ifdef HAVE_DB_STORAGE
	auto conference_address = linphone_address_clone(linphone_conference_info_get_uri(info->toC()));
	foundConferenceInfo = linphone_core_find_conference_information_from_uri(getCore()->getCCore(), conference_address);
	linphone_address_unref(conference_address);
	if (foundConferenceInfo) {
		mConferenceInfo = ConferenceInfo::toCpp(foundConferenceInfo)->getSharedFromThis();
	}
#endif // HAVE_DB_STORAGE


	ConferenceAddress conferenceAddress;
	if (mConferenceInfo == nullptr) {
		setState(State::AllocationPending);
		if (info->getUri().isValid()) {
			// This is a hack for the tester
			lError() << "[Conference Scheduler] This is a hack for liblinphone-tester, you shouldn't see this in production!";
			mConferenceInfo = info;
			setState(State::Ready);
			return;
		}
	} else {
		conferenceAddress = mConferenceInfo->getUri();
		info->setUri(conferenceAddress);
		setState(State::Updating);
	}
	mConferenceInfo = info;
	if (foundConferenceInfo) {
		linphone_conference_info_unref(foundConferenceInfo);
	}

	shared_ptr<LinphonePrivate::ConferenceParams> conferenceParams = ConferenceParams::create(getCore()->getCCore());
	const auto identityAddress = mConferenceInfo->getOrganizer();
	conferenceParams->enableAudio(true);
	conferenceParams->enableVideo(true);
	conferenceParams->setSubject(mConferenceInfo->getSubject());

	if (mConferenceInfo->getDateTime() <= 0) {
		if (!foundConferenceInfo) {
			// Set start time only if a conference is going to be created
			mConferenceInfo->setDateTime(ms_time(NULL));
		}
	} else {
		const auto & startTime = info->getDateTime();
		conferenceParams->setStartTime(startTime);
		const auto & duration = info->getDuration();
		if (duration > 0) {
			const auto endTime = startTime + static_cast<time_t>(duration) * 60; // duration is in minutes
			conferenceParams->setEndTime(endTime);
		}
	}

	if (foundConferenceInfo) {
		// Updating an existing conference
		mSession = getCore()->createOrUpdateConferenceOnServer(conferenceParams, creator, mConferenceInfo->getParticipants(), conferenceAddress);
	} else {
		// Creating conference
		mSession = getCore()->createConferenceOnServer(conferenceParams, identityAddress, mConferenceInfo->getParticipants());
	}
	if (mSession == nullptr) {
		lError() << "[Conference Scheduler] createConferenceOnServer returned a null session!";
		setState(State::Error);
		return;
	}
	mSession->setListener(this);
}

void ConferenceScheduler::onChatMessageStateChanged (const shared_ptr<ChatMessage> &message, ChatMessage::State state) {
	shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();
	IdentityAddress participantAddress = message->getRecipientAddress();

	if (state == ChatMessage::State::NotDelivered) { // Message wasn't delivered
		if (chatRoom->getState() == Conference::State::Created) { // Chat room was created successfully
			if (chatRoom->getCapabilities() & ChatRoom::Capabilities::OneToOne) { // Message was sent using a 1-1 chat room
				lError() << "[Conference Scheduler] Invitation couldn't be sent to participant [" << participantAddress << "]";
				mInvitationsInError.push_back(Address(participantAddress.asAddress()));
			} else { // Message was sent using a group chat room
				lError() << "[Conference Scheduler] At least some participants of the chat room haven't received the invitation!";
				mInvitationsSent += (unsigned long)(message->getParticipantsByImdnState(ChatMessage::State::Delivered).size());
				for (auto &participant : message->getParticipantsByImdnState(ChatMessage::State::NotDelivered)) {
					participantAddress = participant.getParticipant()->getAddress();
					lError() << "[Conference Scheduler] Invitation couldn't be sent to participant [" << participantAddress << "]";
					mInvitationsInError.push_back(Address(participantAddress.asAddress()));
				}
			}
		} else { // Chat room wasn't created
			if (chatRoom->getCapabilities() & ChatRoom::Capabilities::OneToOne) { // Message was sent using a 1-1 chat room
				lError() << "[Conference Scheduler] Invitation couldn't be sent to participant [" << participantAddress << "]";
				mInvitationsInError.push_back(Address(participantAddress.asAddress()));
			} else { // Message was sent using a group chat room
				lError() << "[Conference Scheduler] Chat room wasn't creatd, so no one received the invitation!";
				for (auto &participant : mConferenceInfo->getParticipants()) {
					mInvitationsInError.push_back(Address(participant.asAddress()));
				}
			}
		}
	} else if (state == ChatMessage::State::Delivered) { // Message was delivered
		if (chatRoom->getCapabilities() & ChatRoom::Capabilities::OneToOne) { // A message was sent for each participant
			lInfo() << "[Conference Scheduler] Invitation to participant [" << participantAddress << "] was delivered";
			mInvitationsSent += 1;
		} else { // A single message was used for all participants
			// In case of a group chat room there is only 1 message being sent
			auto participants = chatRoom->getParticipants();
			if (participants.size() == mConferenceInfo->getParticipants().size()) {
				lInfo() << "[Conference Scheduler] Invitation to was delivered to all participants";
				mInvitationsSent = (unsigned long)(mConferenceInfo->getParticipants().size());
			} else { // In case someone couldn't be invited in the chat room, all others may have received the invitation
				for (auto &participant : mConferenceInfo->getParticipants()) {
					bool found = false;
					for (auto &chatRoomParticipant : participants) {
						if (participant.asAddress().weakEqual(chatRoomParticipant->getAddress().asAddress())) {
							found = true;
							break;
						}
					}
					if (!found) {
						lError() << "[Conference Scheduler] Invitation couldn't be sent to participant [" << participant << "]";
						mInvitationsInError.push_back(Address(participant.asAddress()));
					} else {
						mInvitationsSent += 1;
					}
				}
			}
		}
	} else {
		return;
	}

	if (mInvitationsSent + mInvitationsInError.size() == mConferenceInfo->getParticipants().size()) {
		linphone_conference_scheduler_notify_invitations_sent(toC(), L_GET_RESOLVED_C_LIST_FROM_CPP_LIST(mInvitationsInError));
	}
}

void ConferenceScheduler::setConferenceAddress(const ConferenceAddress& conferenceAddress) {
	if (mConferenceInfo == nullptr) {
		lError() << "[Conference Scheduler] Can't update conference address on null conference info";
		setState(State::Error);
		return;
	}

	mConferenceInfo->setUri(conferenceAddress);

#ifdef HAVE_DB_STORAGE
	auto &mainDb = getCore()->getPrivate()->mainDb;
	if (mainDb) {
		lInfo() << "[Conference Scheduler] Conference address is known, inserting conference info in database";
		mainDb->insertConferenceInfo(mConferenceInfo);
	}
#endif

	setState(State::Ready);
}

void ConferenceScheduler::onCallSessionSetTerminated (const shared_ptr<CallSession> &session) {
	const Address *remoteAddress = session->getRemoteContactAddress();
	if (remoteAddress == nullptr) {
		lError() << "[Conference Scheduler] Cannot update conference info focus URI because the remote contact address is invalid.";
		setState(State::Error);
	} else {
		auto conferenceAddress = ConferenceAddress(*remoteAddress);
		lInfo () << "[Conference Scheduler] Conference has been succesfully created: " << conferenceAddress;
		setConferenceAddress(conferenceAddress);
	}
}

void ConferenceScheduler::onCallSessionStateChanged (const shared_ptr<CallSession> &session, CallSession::State state, const string &message) {
	switch(state) {
		case CallSession::State::StreamsRunning:
			session->terminate();
			break;
		default:
			break;
	}
}

shared_ptr<ChatMessage> ConferenceScheduler::createInvitationChatMessage(shared_ptr<AbstractChatRoom> chatRoom) {
	shared_ptr<LinphonePrivate::ChatMessage> message;
	if (linphone_core_conference_ics_in_message_body_enabled(chatRoom->getCore()->getCCore())) {
		message = chatRoom->createChatMessageFromUtf8(mConferenceInfo->toIcsString());
		message->getPrivate()->setContentType(ContentType::Icalendar);
	} else {
		FileContent *content = new FileContent(); // content will be deleted by ChatMessage
		content->setContentType(ContentType::Icalendar);
		content->setFileName("conference.ics");
		content->setBodyFromUtf8(mConferenceInfo->toIcsString());
		message = chatRoom->createFileTransferMessage(content);
	}
	message->addListener(getSharedFromThis());
	return message;
}

void ConferenceScheduler::sendInvitations (shared_ptr<ChatRoomParams> chatRoomParams) {
	if (mState != State::Ready) {
		lWarning() << "[Conference Scheduler] Can't send conference invitation if state ins't Ready, current state is " << mState;
		return;
	}

	if (!mAccount) {
		auto default_account = linphone_core_get_default_account(getCore()->getCCore());
		if (default_account) {
			mAccount =  Account::toCpp(default_account)->getSharedFromThis();
		}
	}

	const auto sender =  mAccount ? IdentityAddress(*L_GET_CPP_PTR_FROM_C_OBJECT(mAccount->getAccountParams()->getIdentityAddress())) : IdentityAddress(linphone_core_get_identity(getCore()->getCCore()));
	if (!sender.isValid()) {
		lWarning() << "[Conference Scheduler] Core address attempting to send invitation isn't valid!";
		return;
	}

	const auto & participants = mConferenceInfo->getParticipants();
	const bool participantFound = (std::find(participants.cbegin(), participants.cend(), sender) != participants.cend());
	if ((sender != mConferenceInfo->getOrganizer()) && !participantFound) {
		lWarning() << "[Conference Scheduler] Unable to find the address " << sender << " sending invitations among the list of participants or the organizer (" << mConferenceInfo->getOrganizer() << ") of conference " << mConferenceInfo->getUri();
		return;
	}

	if (!chatRoomParams->isValid()) {
		lWarning() << "[Conference Scheduler] Given chat room params aren't valid!";
		return;
	}

	std::list<IdentityAddress> addresses;
	for (auto participant : participants) {
		if (participant != sender) {
			addresses.push_back(Address(participant.asAddress()));
		}
	}

	const auto & organizer = mConferenceInfo->getOrganizer();
	if (sender != organizer) {
		const bool organizerFound = (std::find(addresses.cbegin(), addresses.cend(), organizer) != addresses.cend());
		if (!organizerFound) {
			addresses.push_back(Address(organizer.asAddress()));
		}
	}

	mInvitationsInError.clear();
	mInvitationsSent = 0;

	if (chatRoomParams->isGroup()) {
		shared_ptr<AbstractChatRoom> chatRoom = getCore()->getPrivate()->createChatRoom(
			chatRoomParams,
			sender,
			mConferenceInfo->getParticipants());
		if (!chatRoom) {
			lError() << "[Conference Scheduler] Failed to create group chat room using given chat room params & conference information";
			for (auto participant : addresses) {
				mInvitationsInError.push_back(Address(participant.asAddress()));
			}
			linphone_conference_scheduler_notify_invitations_sent(toC(), L_GET_RESOLVED_C_LIST_FROM_CPP_LIST(mInvitationsInError));
			return;
		} else {
			shared_ptr<ChatMessage> message = createInvitationChatMessage(chatRoom);
			message->send();
		}
	} else {
		// Sending the ICS once for each participant in a separated chat room each time.
		for (auto participant : addresses) {
			list<IdentityAddress> participantList;
			participantList.push_back(participant);

			shared_ptr<AbstractChatRoom> chatRoom = getCore()->getPrivate()->searchChatRoom(
				chatRoomParams,
				sender,
				IdentityAddress(),
				participantList);

			if (!chatRoom) {
				lInfo() << "[Conference Scheduler] Existing chat room between [" << sender << "] and [" << participant << "] wasn't found, creating it.";
				chatRoom = getCore()->getPrivate()->createChatRoom(
					chatRoomParams,
					sender,
					participantList);
			} else {
				lInfo() << "[Conference Scheduler] Found existing chat room [" << chatRoom->getPeerAddress() << "] between [" << sender << "] and [" << participant << "], using it";
			}

			if (!chatRoom) {
				lError() << "[Conference Scheduler] Couldn't find nor create a chat room between [" << sender << "] and [" << participant << "]";
				mInvitationsInError.push_back(Address(participant.asAddress()));
				continue;
			}

			shared_ptr<ChatMessage> message = createInvitationChatMessage(chatRoom);
			message->getPrivate()->setRecipientAddress(participant);
			message->send();
		}
	}
}

string ConferenceScheduler::stateToString (ConferenceScheduler::State state) {
	switch (state) {
		case ConferenceScheduler::State::AllocationPending:
			return "AllocationPending";
		case ConferenceScheduler::State::Error:
			return "Error";
		case ConferenceScheduler::State::Ready:
			return "Ready";
		case ConferenceScheduler::State::Updating:
			return "Updating";
		case ConferenceScheduler::State::Idle:
			return "Idle";
	}
	return "<unknown>";
}

std::ostream& operator<<(std::ostream& lhs, ConferenceScheduler::State s) {
	switch (s) {
		case ConferenceScheduler::State::AllocationPending:
			return lhs << "AllocationPending";
		case ConferenceScheduler::State::Error:
			return lhs << "Error";
		case ConferenceScheduler::State::Ready:
			return lhs << "Ready";
		case ConferenceScheduler::State::Updating:
			return lhs << "Updating";
		case ConferenceScheduler::State::Idle:
			return lhs << "Idle";
	}
	return lhs;
}

LinphoneConferenceSchedulerCbsStateChangedCb ConferenceSchedulerCbs::getStateChanged() const {
	return mStateChangedCb;
}

void ConferenceSchedulerCbs::setStateChanged(LinphoneConferenceSchedulerCbsStateChangedCb cb) {
	mStateChangedCb = cb;
}

LinphoneConferenceSchedulerCbsInvitationsSentCb ConferenceSchedulerCbs::getInvitationsSent() const {
	return mInvitationsSent;
}

void ConferenceSchedulerCbs::setInvitationsSent(LinphoneConferenceSchedulerCbsInvitationsSentCb cb) {
	mInvitationsSent = cb;
}

LINPHONE_END_NAMESPACE
