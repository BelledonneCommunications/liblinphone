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

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ConferenceScheduler::ConferenceScheduler (
	const shared_ptr<Core> &core
	) : CoreAccessor(core) {
		mState = State::Idle;
}

ConferenceScheduler::~ConferenceScheduler () {
	if (mSession != nullptr) {
		mSession->setListener(nullptr);
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
	if (info->getParticipants().empty()) {
		lWarning() << "[Conference Scheduler] Can't create a scheduled conference if no participants are added!";
		setState(State::Error);
		return;
	}

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
		setState(State::Updating);
	}
	mConferenceInfo = info;

	shared_ptr<LinphonePrivate::ConferenceParams> conferenceParams = ConferenceParams::create(getCore()->getCCore());
	const auto identityAddress = mConferenceInfo->getOrganizer();
	conferenceParams->enableVideo(true);
	conferenceParams->setSubject(mConferenceInfo->getSubject());

	if (mConferenceInfo->getDateTime() <= 0) {
		mConferenceInfo->setDateTime(ms_time(NULL));
	}
	if (mConferenceInfo->getDateTime() > -1) {
		conferenceParams->setStartTime(mConferenceInfo->getDateTime());
		if (mConferenceInfo->getDuration() > 0) {
			conferenceParams->setEndTime(mConferenceInfo->getDateTime() + (static_cast<time_t>(mConferenceInfo->getDuration()) * 60)); // duration is in minutes
		}
	}

	mSession = getCore()->createConferenceOnServer(conferenceParams, identityAddress, mConferenceInfo->getParticipants());
	if (mSession == nullptr) {
		lError() << "[Conference Scheduler] createConferenceOnServer returned a null session!";
		setState(State::Error);
		return;
	}
	mSession->setListener(this);
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
		lWarning() << "[Conference Scheduler] Can't send conference invitation if state ins't Ready, current state is " << stateToString(mState);
		return;
	}

	if (!chatRoomParams->isValid()) {
		lWarning() << "[Conference Scheduler] Given chat room params aren't valid!";
		return;
	}

	mInvitationsInError.clear();
	mInvitationsSent = 0;

	if (chatRoomParams->isGroup()) {
		shared_ptr<AbstractChatRoom> chatRoom = getCore()->getPrivate()->createChatRoom(
			chatRoomParams,
			mConferenceInfo->getOrganizer(),
			mConferenceInfo->getParticipants());
		if (!chatRoom) {
			lError() << "[Conference Scheduler] Failed to create group chat room using given chat room params & conference information";
			for (auto participant : mConferenceInfo->getParticipants()) {
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
		for (auto participant : mConferenceInfo->getParticipants()) {
			list<IdentityAddress> participantList;
			participantList.push_back(participant);

			shared_ptr<AbstractChatRoom> chatRoom = getCore()->getPrivate()->searchChatRoom(
				chatRoomParams,
				mConferenceInfo->getOrganizer(),
				IdentityAddress(),
				participantList);

			if (!chatRoom) {
				lInfo() << "[Conference Scheduler] Existing chat room between [" << mConferenceInfo->getOrganizer() << "] and [" << participant << "] wasn't found, creating it.";
				chatRoom = getCore()->getPrivate()->createChatRoom(
					chatRoomParams,
					mConferenceInfo->getOrganizer(),
					participantList);
			} else {
				lInfo() << "[Conference Scheduler] Found existing chat room [" << chatRoom->getPeerAddress() << "] between [" << mConferenceInfo->getOrganizer() << "] and [" << participant << "], using it";
			}

			if (!chatRoom) {
				lError() << "[Conference Scheduler] Couldn't find nor create a chat room between [" << mConferenceInfo->getOrganizer() << "] and [" << participant << "]";
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
		default:
			return "Idle";
	}
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
