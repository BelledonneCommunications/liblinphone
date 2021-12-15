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
			conferenceParams->setEndTime(mConferenceInfo->getDateTime() + (mConferenceInfo->getDuration() * 60)); // duration is in minutes
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

void ConferenceScheduler::onCallSessionSetTerminated (const std::shared_ptr<CallSession> &session) {
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

void ConferenceScheduler::sendInvitations (shared_ptr<ChatRoomParams> chatRoomParams) {
	if (mState != State::Ready) {
		lWarning() << "[Conference Scheduler] Can't send conference invitation if state ins't Ready, current state is " << stateToString(mState);
		return;
	}

	if (!chatRoomParams->isValid()) {
		lWarning() << "[Conference Scheduler] Given chat room params aren't valid!";
		return;
	}

	bctbx_list_t *participantsInError = nullptr;

	if (chatRoomParams->isGroup()) {
		shared_ptr<AbstractChatRoom> chatRoom = getCore()->getPrivate()->createChatRoom(
			chatRoomParams,
			mConferenceInfo->getOrganizer(),
			mConferenceInfo->getParticipants());
		if (!chatRoom) {
			lError() << "[Conference Scheduler] Failed to create group chat room using given chat room params & conference information";
			for (auto participant : mConferenceInfo->getParticipants()) {
				LinphoneAddress *cAddress = L_GET_C_BACK_PTR(&(participant.asAddress()));
				participantsInError = bctbx_list_append(participantsInError, cAddress);
			}
		} else {
			FileContent *content = new FileContent();
			content->setContentType(ContentType::Icalendar);
			content->setFileName("conference.ics");
			content->setBodyFromUtf8(mConferenceInfo->toIcsString());
			shared_ptr<LinphonePrivate::ChatMessage> message = chatRoom->createFileTransferMessage(content);
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
				LinphoneAddress *cAddress = L_GET_C_BACK_PTR(&(participant.asAddress()));
				participantsInError = bctbx_list_append(participantsInError, cAddress);
				continue;
			}

			FileContent *content = new FileContent();
			content->setContentType(ContentType::Icalendar);
			content->setFileName("conference.ics");
			content->setBodyFromUtf8(mConferenceInfo->toIcsString());
			shared_ptr<LinphonePrivate::ChatMessage> message = chatRoom->createFileTransferMessage(content);
			message->send();
			// content will be deleted by ChatMessage
		}
	}

	linphone_conference_scheduler_notify_invitations_sent(toC(), participantsInError);
	
	if (participantsInError != nullptr) {
		bctbx_list_free(participantsInError);
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
