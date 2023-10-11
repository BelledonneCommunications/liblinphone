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

#include "account/account.h"
#include "c-wrapper/c-wrapper.h"
#include "chat/chat-message/chat-message-p.h"
#include "conference/conference-params.h"
#include "conference/conference-scheduler.h"
#include "conference/conference.h"
#include "conference/params/call-session-params-p.h"
#include "conference/participant-info.h"
#include "conference/session/media-session.h"
#include "content/file-content.h"
#include "core/core-p.h"
#include "participant.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ConferenceScheduler::ConferenceScheduler(const shared_ptr<Core> &core) : CoreAccessor(core) {
	mState = State::Idle;
	auto default_account = linphone_core_get_default_account(core->getCCore());
	if (default_account) {
		mAccount = Account::toCpp(default_account)->getSharedFromThis();
	}
}

ConferenceScheduler::~ConferenceScheduler() {
	if (mSession != nullptr) {
		mSession->setListener(nullptr);
	}
	if (mAccount) {
		mAccount = nullptr;
	}
}

const std::shared_ptr<Account> &ConferenceScheduler::getAccount() const {
	return mAccount;
}

void ConferenceScheduler::setAccount(std::shared_ptr<Account> account) {
	if ((mState == State::Idle) || (mState == State::AllocationPending) || (mState == State::Error)) {
		mAccount = account;
	} else {
		lWarning() << "[Conference Scheduler] [" << this << "] Unable to change account because scheduler is in state "
		           << mState;
	}
}

ConferenceScheduler::State ConferenceScheduler::getState() const {
	return mState;
}

void ConferenceScheduler::setState(State newState) {
	if (mState != newState) {
		lInfo() << "[Conference Scheduler] [" << this << "] moving from state " << mState << " to state " << newState;
		mState = newState;
		linphone_conference_scheduler_notify_state_changed(toC(), (LinphoneConferenceSchedulerState)newState);
	}
}

const std::shared_ptr<ConferenceInfo> ConferenceScheduler::getInfo() const {
	return mConferenceInfo;
}

void ConferenceScheduler::fillCancelList(const ConferenceInfo::participant_list_t &oldList,
                                         const ConferenceInfo::participant_list_t &newList) {
	mCancelToSend.clear();
	for (const auto &oldParticipantInfo : oldList) {
		const auto &address = oldParticipantInfo->getAddress();
		// Workaroud for CLang issue: https://github.com/llvm/llvm-project/issues/52720
		const bool participantFound =
		    (std::find_if(newList.cbegin(), newList.cend(), [&address = address](const auto &e) {
			     return (address->weakEqual(*e->getAddress()));
		     }) != newList.cend());
		if (!participantFound) {
			auto sequence = oldParticipantInfo->getSequenceNumber();
			if (sequence >= 0) {
				sequence++;
			}
			mCancelToSend.insert(std::make_pair(address, sequence));
		}
	}
}

void ConferenceScheduler::cancelConference(const std::shared_ptr<ConferenceInfo> &info) {
	if (info) {
		auto clone = info->clone()->toSharedPtr();
		while (!clone->getParticipants().empty()) {
			const auto &participants = clone->getParticipants();
			clone->removeParticipant(*(participants.begin()));
		}
		setInfo(clone);
	}
}

std::shared_ptr<Address>
ConferenceScheduler::createParticipantAddress(const ConferenceInfo::participant_list_t::value_type &p) const {
	std::shared_ptr<Address> address = Address::create(p->getAddress()->getUri());
	for (const auto &[name, value] : p->getAllParameters()) {
		address->setParam(name, value);
	}
	return address;
}

void ConferenceScheduler::setInfo(const std::shared_ptr<ConferenceInfo> &info) {
	if (!info) {
		lWarning() << "[Conference Scheduler] [" << this
		           << "] Trying to set null conference info to the conference scheduler. Aborting conference creation!";
		setState(State::Error);
		return;
	}

	// Clone the conference info in order to be able to modify it freely
	auto clone = info->clone()->toSharedPtr();

	if (!mAccount) {
		auto default_account = linphone_core_get_default_account(getCore()->getCCore());
		if (default_account) {
			mAccount = Account::toCpp(default_account)->getSharedFromThis();
		}
	}

	const auto creator = mAccount ? mAccount->getAccountParams()->getIdentityAddress()
	                              : Address::create(linphone_core_get_identity(getCore()->getCCore()));
	if (!creator || !creator->isValid()) {
		lWarning() << "[Conference Scheduler] [" << this << "] Core address attempting to set conference information!";
		return;
	}

	const auto &organizer = clone->getOrganizerAddress();
	const auto &conferenceAddress = clone->getUri();
	const auto &participants = clone->getParticipants();
	const auto participantListEmpty = participants.empty();
	const bool participantFound = (std::find_if(participants.cbegin(), participants.cend(), [&creator](const auto &p) {
		                               return (creator->weakEqual(*p->getAddress()));
	                               }) != participants.cend());
	if (!creator->weakEqual(*organizer) && !participantFound) {
		lWarning() << "[Conference Scheduler] [" << this << "] Address " << creator->toString()
		           << " is trying to modify the conference information but he/she is neither an invited participant "
		              "nor the organizer ("
		           << organizer << ") of conference "
		           << (conferenceAddress ? conferenceAddress->toString() : std::string("<unknown>"));
		setState(State::Error);
		return;
	}

	bool isUpdate = false;
#ifdef HAVE_DB_STORAGE
	if (conferenceAddress && conferenceAddress->isValid()) {
		auto &mainDb = getCore()->getPrivate()->mainDb;
		auto confInfo = mainDb->getConferenceInfoFromURI(conferenceAddress);
		if (confInfo) {
			lInfo() << "[Conference Scheduler] [" << this
			        << "] Found matching conference info in database for address ["
			        << (conferenceAddress ? conferenceAddress->toString() : std::string("<unknown>")) << "]";
			isUpdate = true;
			setState(State::Updating);
			clone->updateFrom(confInfo);
			fillCancelList(confInfo->getParticipants(), clone->getParticipants());
		}
	}
#endif // HAVE_DB_STORAGE

	if (participantListEmpty && !isUpdate) {
		lWarning() << "[Conference Scheduler] [" << this
		           << "] Can't create a scheduled conference if no participants are added!";
		setState(State::Error);
		return;
	}

	if (mConferenceInfo == nullptr && !isUpdate) {
		setState(State::AllocationPending);
		if (conferenceAddress && conferenceAddress->isValid()) {
			// This is a hack for the tester
			lError() << "[Conference Scheduler] [" << this
			         << "] This is a hack for liblinphone-tester, you shouldn't see this in production!";
			mConferenceInfo = clone;
			setState(State::Ready);
			return;
		}
	} else if (mConferenceInfo != nullptr) {
		setState(State::Updating);
		clone->updateFrom(mConferenceInfo);
		fillCancelList(mConferenceInfo->getParticipants(), clone->getParticipants());
	}

	auto infoState = ConferenceInfo::State::New;
	if (getState() == State::Updating) {
		if (clone->getParticipants().size() == 0) {
			infoState = ConferenceInfo::State::Cancelled;
		} else {
			infoState = ConferenceInfo::State::Updated;
		}
	}
	clone->setState(infoState);
	mConferenceInfo = clone;

	shared_ptr<LinphonePrivate::ConferenceParams> conferenceParams = ConferenceParams::create(getCore()->getCCore());
	conferenceParams->enableAudio(true);
	conferenceParams->enableVideo(true);
	conferenceParams->setSubject(mConferenceInfo->getSubject());
	conferenceParams->setSecurityLevel(mConferenceInfo->getSecurityLevel());

	if (mConferenceInfo->getDateTime() <= 0) {
		if (!isUpdate) {
			// Set start time only if a conference is going to be created
			mConferenceInfo->setDateTime(ms_time(NULL));
		}
	} else {
		const auto &startTime = clone->getDateTime();
		conferenceParams->setStartTime(startTime);
		const auto &duration = clone->getDuration();
		if (duration > 0) {
			const auto endTime = startTime + static_cast<time_t>(duration) * 60; // duration is in minutes
			conferenceParams->setEndTime(endTime);
		}
	}

	std::list<std::shared_ptr<Address>> invitees;
	for (const auto &p : mConferenceInfo->getParticipants()) {
		invitees.push_back(createParticipantAddress(p));
	}

	if (isUpdate) {
		// Updating an existing conference
		mSession = getCore()->createOrUpdateConferenceOnServer(conferenceParams, creator, invitees, conferenceAddress);
	} else {
		// Creating conference
		mSession = getCore()->createConferenceOnServer(conferenceParams, creator, invitees);
	}
	if (mSession == nullptr) {
		lError() << "[Conference Scheduler] [" << this << "] createConferenceOnServer returned a null session!";
		setState(State::Error);
		return;
	}
	mSession->setListener(this);

	if (getState() != State::Error) {
		// Update conference info in database with updated conference information
#ifdef HAVE_DB_STORAGE
		auto &mainDb = getCore()->getPrivate()->mainDb;
		mainDb->insertConferenceInfo(mConferenceInfo);
#endif // HAVE_DB_STORAGE
	}
}

void ConferenceScheduler::onChatMessageStateChanged(const shared_ptr<ChatMessage> &message, ChatMessage::State state) {
	shared_ptr<AbstractChatRoom> chatRoom = message->getChatRoom();
	auto participantAddress = message->getRecipientAddress();
	if (state == ChatMessage::State::NotDelivered) {              // Message wasn't delivered
		if (chatRoom->getState() == Conference::State::Created) { // Chat room was created successfully
			if (chatRoom->getCapabilities() &
			    ChatRoom::Capabilities::OneToOne) { // Message was sent using a 1-1 chat room
				lError() << "[Conference Scheduler] [" << this << "] Invitation couldn't be sent to participant ["
				         << *participantAddress << "]";
				mInvitationsInError.push_back(participantAddress);
			} else { // Message was sent using a group chat room
				lError() << "[Conference Scheduler] [" << this
				         << "] At least some participants of the chat room haven't received the invitation!";
				mInvitationsSent +=
				    (unsigned long)(message->getParticipantsByImdnState(ChatMessage::State::Delivered).size());
				for (auto &participant : message->getParticipantsByImdnState(ChatMessage::State::NotDelivered)) {
					participantAddress = participant.getParticipant()->getAddress();
					lError() << "[Conference Scheduler] [" << this << "] Invitation couldn't be sent to participant ["
					         << *participantAddress << "]";
					mInvitationsInError.push_back(participantAddress);
				}
			}
		} else { // Chat room wasn't created
			if (chatRoom->getCapabilities() &
			    ChatRoom::Capabilities::OneToOne) { // Message was sent using a 1-1 chat room
				lError() << "[Conference Scheduler] [" << this << "] Invitation couldn't be sent to participant ["
				         << *participantAddress << "]";
				mInvitationsInError.push_back(participantAddress);
			} else { // Message was sent using a group chat room
				lError() << "[Conference Scheduler] [" << this
				         << "] Chat room wasn't created, so no one received the invitation!";
				for (auto &participant : mInvitationsToSend) {
					mInvitationsInError.push_back(participant);
				}
			}
		}
	} else if (state == ChatMessage::State::Delivered || state == ChatMessage::State::DeliveredToUser ||
	           state == ChatMessage::State::Displayed) { // Message was delivered (first received state can be any of
		                                                 // those 3)
		lInfo() << "[Conference Scheduler] [" << this << "] Invitation to participant [" << *participantAddress
		        << "] was delivered (" << state << ")";
		mInvitationsSent += 1;
		message->removeListener(getSharedFromThis());
	} else {
		return;
	}

	if (mInvitationsSent + mInvitationsInError.size() == mInvitationsToSend.size()) {
		ListHolder<Address> erroredInvitations;
		erroredInvitations.mList = mInvitationsInError;
		linphone_conference_scheduler_notify_invitations_sent(toC(), erroredInvitations.getCList());
	}
}

void ConferenceScheduler::setConferenceAddress(const std::shared_ptr<Address> &conferenceAddress) {
	if (mConferenceInfo == nullptr) {
		lError() << "[Conference Scheduler] [" << this << "] Can't update conference address " << *conferenceAddress
		         << " on null conference info";
		setState(State::Error);
		return;
	}

	mConferenceInfo->setUri(conferenceAddress);

#ifdef HAVE_DB_STORAGE
	auto &mainDb = getCore()->getPrivate()->mainDb;
	if (mainDb) {
		lInfo() << "[Conference Scheduler] [" << this << "] Conference address " << *conferenceAddress
		        << " is known, inserting conference info in database";
		mainDb->insertConferenceInfo(mConferenceInfo);
	}
#endif

	setState(State::Ready);
}

void ConferenceScheduler::onCallSessionSetTerminated(const shared_ptr<CallSession> &session) {
	const std::shared_ptr<Address> remoteAddress = session->getRemoteContactAddress();
	if (remoteAddress == nullptr) {
		auto conferenceAddress = mConferenceInfo->getUri();
		lError() << "[Conference Scheduler] [" << this
		         << "] The session to update the conference information of conference "
		         << (conferenceAddress && conferenceAddress->isValid() ? conferenceAddress->toString()
		                                                               : std::string("<unknown-address>"))
		         << " did not succesfully establish hence it is likely that the request wasn't taken into account by "
		            "the server";
		setState(State::Error);
	} else if (getState() != State::Error) {
		// Do not try to call inpromptu conference if a participant updates its informations
		if ((getState() == State::AllocationPending) && (session->getParams()->getPrivate()->getStartTime() < 0)) {
			lInfo() << "Automatically rejoining conference " << remoteAddress->toString();
			auto new_params = linphone_core_create_call_params(getCore()->getCCore(), nullptr);
			// Participant with the focus call is admin
			L_GET_CPP_PTR_FROM_C_OBJECT(new_params)->addCustomContactParameter("admin", Utils::toString(true));
			std::list<std::shared_ptr<Address>> addressesList;
			for (const auto &participantInfo : mConferenceInfo->getParticipants()) {
				addressesList.push_back(participantInfo->getAddress());
			}
			addressesList.sort([](const auto &addr1, const auto &addr2) { return *addr1 < *addr2; });
			addressesList.unique([](const auto &addr1, const auto &addr2) { return addr1->weakEqual(*addr2); });

			if (!addressesList.empty()) {
				auto content = Content::create();
				content->setBodyFromUtf8(Utils::getResourceLists(addressesList));
				content->setContentType(ContentType::ResourceLists);
				content->setContentDisposition(ContentDisposition::RecipientList);
				if (linphone_core_content_encoding_supported(getCore()->getCCore(), "deflate")) {
					content->setContentEncoding("deflate");
				}

				L_GET_CPP_PTR_FROM_C_OBJECT(new_params)->addCustomContent(content);
			}
			LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(getCore()->getCCore());
			bool_t initiate_video = !!linphone_video_activation_policy_get_automatically_initiate(pol);
			linphone_call_params_enable_video(
			    new_params,
			    static_pointer_cast<MediaSession>(session)->getMediaParams()->videoEnabled() && initiate_video);
			linphone_video_activation_policy_unref(pol);

			linphone_core_invite_address_with_params_2(getCore()->getCCore(), remoteAddress->toC(), new_params,
			                                           L_STRING_TO_C(mConferenceInfo->getSubject()), NULL);
			linphone_call_params_unref(new_params);
		}

		auto conferenceAddress = remoteAddress;
		lInfo() << "[Conference Scheduler] [" << this
		        << "] Conference has been succesfully created: " << *conferenceAddress;
		setConferenceAddress(conferenceAddress);
	}
}

void ConferenceScheduler::onCallSessionStateChanged(const shared_ptr<CallSession> &session,
                                                    CallSession::State state,
                                                    BCTBX_UNUSED(const string &message)) {
	switch (state) {
		case CallSession::State::Error:
			setState(State::Error);
			break;
		case CallSession::State::StreamsRunning:
			session->terminate();
			break;
		default:
			break;
	}
}

shared_ptr<ChatMessage> ConferenceScheduler::createInvitationChatMessage(shared_ptr<AbstractChatRoom> chatRoom,
                                                                         const std::shared_ptr<Address> participant,
                                                                         bool cancel) {
	shared_ptr<LinphonePrivate::ChatMessage> message;
	int sequence = -1;
	if (participant && participant->isValid()) {
		const auto cancelParticipant =
		    std::find_if(mCancelToSend.cbegin(), mCancelToSend.cend(),
		                 [&participant](const auto &p) { return (participant->weakEqual(*p.first)); });
		if (cancelParticipant == mCancelToSend.cend()) {
			const auto &participantInfo = mConferenceInfo->findParticipant(participant);
			if (participantInfo) {
				sequence = participantInfo->getSequenceNumber();
			}
		} else {
			sequence = (*cancelParticipant).second;
		}
	}
	if (linphone_core_conference_ics_in_message_body_enabled(chatRoom->getCore()->getCCore())) {
		message = chatRoom->createChatMessageFromUtf8(mConferenceInfo->toIcsString(cancel, sequence));
		message->getPrivate()->setContentType(ContentType::Icalendar);
	} else {
		auto content = FileContent::create<FileContent>(); // content will be deleted by ChatMessage
		content->setContentType(ContentType::Icalendar);
		content->setFileName("conference.ics");
		content->setBodyFromUtf8(mConferenceInfo->toIcsString(cancel, sequence));
		message = chatRoom->createFileTransferMessage(content);
	}

	// Update conference info in database with new sequence and uid
#ifdef HAVE_DB_STORAGE
	auto &mainDb = getCore()->getPrivate()->mainDb;
	mainDb->insertConferenceInfo(mConferenceInfo);
#endif // HAVE_DB_STORAGE
	message->addListener(getSharedFromThis());
	return message;
}

void ConferenceScheduler::sendInvitations(shared_ptr<ChatRoomParams> chatRoomParams) {
	if (mState != State::Ready) {
		lWarning() << "[Conference Scheduler] [" << this
		           << "] Can't send conference invitation if state ins't Ready, current state is " << mState;
		return;
	}

	if (!mAccount) {
		auto default_account = linphone_core_get_default_account(getCore()->getCCore());
		if (default_account) {
			mAccount = Account::toCpp(default_account)->getSharedFromThis();
		}
	}

	const auto sender = mAccount ? mAccount->getAccountParams()->getIdentityAddress()
	                             : Address::create(linphone_core_get_identity(getCore()->getCCore()));
	if (!sender || !sender->isValid()) {
		lWarning() << "[Conference Scheduler] [" << this << "] Core address attempting to send invitation isn't valid!";
		return;
	}

	const auto &participants = mConferenceInfo->getParticipants();
	const bool participantFound = (std::find_if(participants.cbegin(), participants.cend(), [&sender](const auto &p) {
		                               return (sender->weakEqual(*p->getAddress()));
	                               }) != participants.cend());
	const auto &conferenceAddress = mConferenceInfo->getUri();
	const auto organizer = mConferenceInfo->getOrganizerAddress();
	if (!sender->weakEqual(*organizer) && !participantFound) {
		lWarning() << "[Conference Scheduler] [" << this << "] Unable to find the address " << *sender
		           << " sending invitations among the list of participants or the organizer (" << *organizer
		           << ") of conference " << *conferenceAddress;
		return;
	}

	if (chatRoomParams->isGroup()) {
		lError() << "[Conference Scheduler] [" << this
		         << "] Unable to send invitations to a group chat. Participant must be notified using individual chat "
		            "rooms.";
		return;
	}
	if (!chatRoomParams->isValid()) {
		lWarning() << "[Conference Scheduler] [" << this << "] Given chat room params aren't valid!";
		return;
	}

	std::shared_ptr<ConferenceInfo> dbConferenceInfo = nullptr;
#ifdef HAVE_DB_STORAGE
	auto &mainDb = getCore()->getPrivate()->mainDb;
	if (mainDb && conferenceAddress) {
		dbConferenceInfo = getCore()->getPrivate()->mainDb->getConferenceInfoFromURI(conferenceAddress);
	}
#endif // HAVE_DB_STORAGE

	std::list<std::shared_ptr<Address>> invitees;
	for (const auto &participantInfo : participants) {
		invitees.push_back(participantInfo->getAddress());
	}
// https://gcc.gnu.org/bugzilla/show_bug.cgi?format=multiple&id=81767
#if __GNUC__ == 7
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif //  __GNUC__ == 7
	for (const auto &[address, value] : mCancelToSend) {
		invitees.push_back(address);
	}
#if __GNUC__ == 7
#pragma GCC diagnostic pop
#endif //  __GNUC__ == 7

	mInvitationsToSend.clear();
	for (auto participant : invitees) {
		if (!sender->weakEqual(*participant)) {
			mInvitationsToSend.push_back(participant);
		} else {
			lInfo() << "[Conference Scheduler] [" << this << "] Removed conference participant [" << *participant
			        << "] from chat room participants as it is ourselves";
		}

		const auto &participantInfo = mConferenceInfo->findParticipant(participant);
		if (participantInfo) {
			auto newParticipantInfo = participantInfo->clone()->toSharedPtr();
			if (newParticipantInfo->getRole() == Participant::Role::Unknown) {
				// Search if the role of the participant info stored in the database is different.
				// We hit this case, if a client creates an inpromptu conference on a server and sends the ICS
				// invitation as well. If the network connection is fast enough, it may happen that that the client
				// already received the NOTIFY full state, therefore the server already took the decision on the
				// participant role In such a scenario, the participant information stored in the conference scheduler
				// should stick with it
				const auto &dbParticipantInfo =
				    (dbConferenceInfo) ? dbConferenceInfo->findParticipant(newParticipantInfo->getAddress()) : nullptr;
				if (dbParticipantInfo) {
					newParticipantInfo->setRole(dbParticipantInfo->getRole());
				}
			}
			const auto &sequence = newParticipantInfo->getSequenceNumber();
			const auto newSequence = (sequence < 0) ? 0 : sequence + 1;
			newParticipantInfo->setSequenceNumber(newSequence);
			mConferenceInfo->updateParticipant(newParticipantInfo);
		}
	}

	const auto &organizerInfo = mConferenceInfo->getOrganizer();
	if (organizerInfo) {
		auto organizerAmongParticipants = (mConferenceInfo->findParticipant(organizerInfo->getAddress()) != nullptr);
		if (!organizerAmongParticipants) {
			auto newOrganizerInfo = organizerInfo->clone()->toSharedPtr();
			const auto &sequence = newOrganizerInfo->getSequenceNumber();
			const auto newSequence = (sequence < 0) ? 0 : sequence + 1;
			newOrganizerInfo->setSequenceNumber(newSequence);
			mConferenceInfo->setOrganizer(newOrganizerInfo);
		}
	}

	if (!sender->weakEqual(*organizer)) {
		const bool organizerFound =
		    (std::find(mInvitationsToSend.cbegin(), mInvitationsToSend.cend(), organizer) != mInvitationsToSend.cend());
		if (!organizerFound) {
			lInfo() << "[Conference Scheduler] [" << this << "] Organizer [" << *organizer
			        << "] not found in conference participants, adding it to chat room participants";
			mInvitationsToSend.push_back(organizer);
		}
		const bool organizerInInviteesFound =
		    (std::find(invitees.cbegin(), invitees.cend(), organizer) != invitees.cend());
		if (!organizerInInviteesFound) {
			invitees.push_back(organizer);
		}
	}

	mInvitationsInError.clear();
	mInvitationsSent = 0;

	// Sending the ICS once for each participant in a separated chat room each time.
	for (auto participant : mInvitationsToSend) {
		list<std::shared_ptr<Address>> chatRoomParticipantList;
		chatRoomParticipantList.push_back(participant);
		list<std::shared_ptr<Address>> participantList;
		std::shared_ptr<Address> remoteAddress = nullptr;
		if (chatRoomParams->getChatRoomBackend() == LinphonePrivate::ChatRoomParams::ChatRoomBackend::FlexisipChat) {
			participantList.push_back(participant);
		} else {
			remoteAddress = participant;
		}
		shared_ptr<AbstractChatRoom> chatRoom =
		    getCore()->getPrivate()->searchChatRoom(chatRoomParams, sender, remoteAddress, participantList);

		if (!chatRoom) {
			lInfo() << "[Conference Scheduler] [" << this << "] Existing chat room between [" << *sender << "] and ["
			        << *participant << "] wasn't found, creating it.";
			chatRoom = getCore()->getPrivate()->createChatRoom(chatRoomParams, sender, chatRoomParticipantList);
		} else {
			lInfo() << "[Conference Scheduler] [" << this << "] Found existing chat room ["
			        << *chatRoom->getPeerAddress() << "] between [" << *sender << "] and [" << *participant
			        << "], using it";
		}

		if (!chatRoom) {
			lError() << "[Conference Scheduler] [" << this << "] Couldn't find nor create a chat room between ["
			         << *sender << "] and [" << *participant << "]";
			mInvitationsInError.push_back(participant);
			continue;
		}

		const bool cancel = (mCancelToSend.find(participant) != mCancelToSend.cend()) ||
		                    (mConferenceInfo->getState() == ConferenceInfo::State::Cancelled);

		shared_ptr<ChatMessage> message = createInvitationChatMessage(chatRoom, participant, cancel);
		message->getPrivate()->setRecipientAddress(participant);
		message->send();
	}
}

string ConferenceScheduler::stateToString(ConferenceScheduler::State state) {
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

std::ostream &operator<<(std::ostream &lhs, ConferenceScheduler::State s) {
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
