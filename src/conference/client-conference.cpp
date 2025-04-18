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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "call/call.h"
#include "chat/chat-message/chat-message-p.h"
#include "client-conference.h"
#include "conference-params.h"
#include "conference/participant-info.h"
#include "core/core-p.h"
#include "core/core.h"
#include "event/event.h"
#include "linphone/core.h"
#include "logger/logger.h"
#include "params/media-session-params-p.h"
#include "params/media-session-params.h"
#include "participant.h"
#include "sal/refer-op.h"
#include "session/media-session-p.h"
#include "session/media-session.h"
#include "session/ms2-streams.h"
#ifdef HAVE_ADVANCED_IM
#include "handlers/client-conference-event-handler.h"
#include "handlers/client-conference-list-event-handler.h"
#endif // HAVE_ADVANCED_IM

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ClientConference::ClientConference(const shared_ptr<Core> &core,
                                   std::shared_ptr<CallSessionListener> listener,
                                   const std::shared_ptr<const ConferenceParams> params)
    : Conference(core, listener, params) {
}

ClientConference::~ClientConference() {
	if (getState() != ConferenceInterface::State::Deleted) {
		terminate();
	}
	if (mJoiningParams) {
		delete mJoiningParams;
	}

#ifdef HAVE_ADVANCED_IM
	mEventHandler.reset();
#endif // HAVE_ADVANCED_IM
}

void ClientConference::createFocus(const std::shared_ptr<const Address> &focusAddr,
                                   const std::shared_ptr<CallSession> focusSession) {
	mFocus = Participant::create(getSharedFromThis(), focusAddr, focusSession);
	if (focusSession) {
		focusSession->addListener(getSharedFromThis());
	}
	lInfo() << *this << ": Create focus '" << *mFocus->getAddress() << "' from address : " << *focusAddr;
}

void ClientConference::configure(SalCallOp *op) {
	mConfParams->setUtf8Subject(op->getSubject());

	std::shared_ptr<Address> organizer = nullptr;
	const auto sipFrag = op->getContentInRemote(ContentType::SipFrag);
	bool isEmpty = !sipFrag || sipFrag.value().get().isEmpty();
	if (!isEmpty) {
		organizer = Address::create(Utils::getSipFragAddress(sipFrag.value()));
	}

	fillInvitedParticipantList(op, organizer, false);
}

void ClientConference::initFromDb(const std::shared_ptr<Participant> &me,
                                  const ConferenceId conferenceId,
                                  const unsigned int lastNotifyId,
                                  bool hasBeenLeft) {
	const auto &conferenceAddress = mConfParams->getConferenceAddress();
	createFocus(conferenceAddress, nullptr);
	mMe = Participant::create(getSharedFromThis(), me->getAddress());
	mMe->setAdmin(me->isAdmin());
	for (const auto &device : me->getDevices()) {
		mMe->addDevice(device);
	}
	bool_t forceFullState = linphone_config_get_bool(linphone_core_get_config(getCore()->getCCore()), "misc",
	                                                 "conference_event_package_force_full_state", FALSE);
	auto notifyId = forceFullState ? 0 : lastNotifyId;
	setLastNotify(notifyId);

#ifdef HAVE_ADVANCED_IM
	if (mConfParams->chatEnabled()) {
		setChatRoom((new ClientChatRoom(getCore(), getSharedFromThis()))->toSharedPtr());
	}
#endif // HAVE_ADVANCED_IM

	setState(ConferenceInterface::State::Instantiated);

	mPendingSubject = getUtf8Subject();
	mConferenceId = conferenceId;
	if (conferenceAddress) {
		setConferenceAddress(conferenceAddress);
	}

	if (!hasBeenLeft) {
		// No need to add a conference with media to the client conference list event handler
		initializeHandlers(this, isChatOnly());
	}
}

void ClientConference::initWithFocus(const std::shared_ptr<const Address> &focusAddr,
                                     const std::shared_ptr<CallSession> &focusSession,
                                     SalCallOp *op,
                                     ConferenceListener *confListener) {
	createFocus(focusAddr, focusSession);
	init(op, confListener);
}

void ClientConference::init(SalCallOp *op, BCTBX_UNUSED(ConferenceListener *confListener)) {
	// Set last notify to 0 in order to ensure that the 1st notify from local conference is correctly processed
	// Local conference sets last notify to 1 in its constructor
	setLastNotify(0);

	inititializeMe();

	const auto &core = getCore();
	std::shared_ptr<Address> organizerAddress = nullptr;
	auto conferenceAddress = mFocus ? mFocus->getAddress() : nullptr;
	std::shared_ptr<ConferenceInfo> conferenceInfo = nullptr;
#ifdef HAVE_DB_STORAGE
	if (conferenceAddress && core->getPrivate()->mainDb && supportsMedia()) {
		conferenceInfo = core->getPrivate()->mainDb->getConferenceInfoFromURI(conferenceAddress);
	}
#endif // HAVE_DB_STORAGE

	const auto focusSession = mFocus ? mFocus->getSession() : nullptr;
	if (conferenceInfo) {
		mConfParams->setUtf8Subject(conferenceInfo->getUtf8Subject());
		auto startTime = conferenceInfo->getDateTime();
		mConfParams->setStartTime(startTime);
		auto duration = conferenceInfo->getDuration();
		if (duration > 0) {
			// duration is in minutes therefore convert it to seconds by multiplying it by 60
			time_t endTime = startTime + static_cast<time_t>(duration) * 60;
			mConfParams->setEndTime(endTime);
		}

		fillInvitedParticipantList(conferenceInfo->getParticipants());
		const auto &organizerInfo = conferenceInfo->getOrganizer();
		organizerAddress = organizerInfo->getAddress();
		const auto organizerFound = (std::find_if(mInvitedParticipants.cbegin(), mInvitedParticipants.cend(),
		                                          [&organizerAddress](const auto &participant) {
			                                          const auto pAddress = participant->getAddress();
			                                          return organizerAddress->weakEqual(*pAddress);
		                                          }) != mInvitedParticipants.cend());
		if (!organizerFound) {
			auto organizer = Participant::create(organizerAddress);
			organizer->setRole(organizerInfo->getRole());
			mInvitedParticipants.push_back(organizer);
		}

		conferenceAddress = conferenceInfo->getUri();
	} else {
		if (focusSession) {
			const auto &remoteParams = static_pointer_cast<MediaSession>(focusSession)->getRemoteParams();
			mConfParams->setStartTime(remoteParams->getPrivate()->getStartTime());
			mConfParams->setEndTime(remoteParams->getPrivate()->getEndTime());
		}

		if (op) {
			configure(op);
		}
	}

	mPendingSubject = mConfParams->getUtf8Subject();
	mConfParams->enableLocalParticipant(false);

#ifdef HAVE_ADVANCED_IM
	if (isChatOnly()) {
		setChatRoom((new ClientChatRoom(core, getSharedFromThis()))->toSharedPtr());
	}
#endif // HAVE_ADVANCED_IM

	const auto &meAddress = mMe->getAddress();
	if (supportsMedia()) {
		getMe()->setAdmin((focusSession == nullptr) || (organizerAddress == nullptr) ||
		                  organizerAddress->weakEqual(*meAddress));
	}

	if (focusSession || conferenceInfo) {
		ConferenceId conferenceId(conferenceAddress, meAddress, getCore()->createConferenceIdParams());
		setConferenceId(conferenceId);
	}

	setState(ConferenceInterface::State::Instantiated);

	if (focusSession) {
		setConferenceAddress(conferenceAddress);
	}

	if (focusSession || isChatOnly()) {
		finalizeCreation();
	}
}

void ClientConference::createEventHandler(BCTBX_UNUSED(ConferenceListener *confListener),
                                          BCTBX_UNUSED(bool addToListEventHandler)) {
#ifdef HAVE_ADVANCED_IM
	bool eventLogEnabled = !!linphone_config_get_bool(linphone_core_get_config(getCore()->getCCore()), "misc",
	                                                  "conference_event_log_enabled", TRUE);
	if (eventLogEnabled) {
		if (!mEventHandler) {
			mEventHandler =
			    std::make_shared<ClientConferenceEventHandler>(getCore(), getSharedFromThis(), confListener);
		}
		if (addToListEventHandler) {
			getCore()->getPrivate()->clientListEventHandler->addHandler(mEventHandler);
		}
		const auto &conferenceId = getConferenceId();
		if (conferenceId.isValid()) {
			mEventHandler->subscribe(conferenceId);
		}
	} else {
#endif // HAVE_ADVANCED_IM
		lInfo() << "Unable to send SUBSCRIBE to finalize creation of " << *this
		        << " because conference event package (RFC 4575) is disabled or the SDK was not compiled with "
		           "ENABLE_ADVANCED_IM flag set to on";
#ifdef HAVE_ADVANCED_IM
	}
#endif // HAVE_ADVANCED_IM
}

void ClientConference::initializeHandlers(ConferenceListener *confListener, bool addToListEventHandler) {
	addListener(std::shared_ptr<ConferenceListenerInterface>(static_cast<ConferenceListenerInterface *>(this),
	                                                         [](BCTBX_UNUSED(ConferenceListenerInterface * p)) {}));
	createEventHandler(confListener, addToListEventHandler);
}

void ClientConference::updateAndSaveConferenceInformations() {
	const auto conferenceInfo = getUpdatedConferenceInfo();
	if (!conferenceInfo) return;

	// Until the full state is received, the core doesn't know the conference capabilities
	if (!mFullStateReceived) {
		conferenceInfo->setCapability(LinphoneStreamTypeAudio, mConfParams->audioEnabled());
		conferenceInfo->setCapability(LinphoneStreamTypeVideo, mConfParams->videoEnabled());
		conferenceInfo->setCapability(LinphoneStreamTypeText, mConfParams->chatEnabled());
	}

	conferenceInfo->setEarlierJoiningTime(mConfParams->getEarlierJoiningTime());
	conferenceInfo->setExpiryTime(mConfParams->getExpiryTime());

	// Update the organizer if it is not the same as the one guessed earlier on
	const auto organizer = getOrganizer();
	const auto infoOrganizer = conferenceInfo->getOrganizerAddress();
	if (!organizer->weakEqual(*infoOrganizer)) {
		conferenceInfo->setOrganizer(organizer);
	}

#ifdef HAVE_DB_STORAGE
	// Store into DB after the start incoming notification in order to have a valid conference address being the contact
	// address of the call
	auto &mainDb = getCore()->getPrivate()->mainDb;
	if (mainDb) {
		lInfo() << "Inserting or updating conference information to database related to " << *this;
		mConferenceInfoId = mainDb->insertConferenceInfo(conferenceInfo);
	}
#endif // HAVE_DB_STORAGE

	auto callLog = getMainSession() ? getMainSession()->getLog() : nullptr;
	if (callLog) {
		callLog->setConferenceInfo(conferenceInfo);
		callLog->setConferenceInfoId(mConferenceInfoId);
	}
}

std::shared_ptr<ConferenceInfo> ClientConference::createConferenceInfo() const {
	const auto organizer = getOrganizer();
	return createConferenceInfoWithCustomParticipantList(organizer, getFullParticipantList());
}

std::shared_ptr<CallSession> ClientConference::createSessionTo(const std::shared_ptr<const Address> &sessionTo) {
	MediaSessionParams csp;
	csp.addCustomHeader("Require", "recipient-list-invite");
	csp.addCustomContactParameter(Conference::TextParameter);
	if (!mConfParams->isGroup()) {
		csp.addCustomHeader("One-To-One-Chat-Room", "true");
	}
	if (mConfParams->getChatParams()->isEncrypted()) {
		csp.addCustomHeader("End-To-End-Encrypted", "true");
	}
	if (mConfParams->getChatParams()->getEphemeralMode() == AbstractChatRoom::EphemeralMode::AdminManaged) {
		csp.addCustomHeader("Ephemerable", "true");
		csp.addCustomHeader("Ephemeral-Life-Time", to_string(mConfParams->getChatParams()->getEphemeralLifetime()));
	}

	csp.enableAudio(mConfParams->audioEnabled());
	csp.enableVideo(mConfParams->videoEnabled());
	csp.getPrivate()->disableRinging(!supportsMedia());
	csp.getPrivate()->enableToneIndications(supportsMedia());

	auto chatRoom = getChatRoom();
	shared_ptr<CallSession> session = mFocus->createSession(*this, &csp, TRUE);
	session->addListener(getSharedFromThis());
	std::shared_ptr<Address> meCleanedAddress = Address::create(getMe()->getAddress()->getUriWithoutGruu());

	session->configure(LinphoneCallOutgoing, nullptr, nullptr, meCleanedAddress, sessionTo);
	session->initiateOutgoing();
	session->getPrivate()->createOp();

	return session;
}

std::shared_ptr<CallSession> ClientConference::createSession() {
	const std::shared_ptr<Address> &conferenceAddress = getConferenceAddress();
	const std::shared_ptr<Address> sessionTo =
	    (conferenceAddress && conferenceAddress->isValid()) ? conferenceAddress : mFocus->getAddress();
	return createSessionTo(sessionTo);
}

void ClientConference::setMainSession(const std::shared_ptr<CallSession> &session) {
	if (mFocus) {
		mFocus->setSession(session);
	}
}

void ClientConference::confirmJoining(BCTBX_UNUSED(SalCallOp *op)) {
#ifdef HAVE_ADVANCED_IM
	auto chatRoom = getChatRoom();
	if (!chatRoom) return;

	auto focusSession = mFocus->getSession();
	bool previousSession = (focusSession != nullptr);

	auto remoteContact = Address::create(op->getRemoteContact());
	auto clientGroupChatRoom = dynamic_pointer_cast<ClientChatRoom>(chatRoom);
	auto previousConferenceIds = clientGroupChatRoom->getPreviousConferenceIds();
	bool found = std::find_if(previousConferenceIds.cbegin(), previousConferenceIds.cend(),
	                          [&remoteContact](const auto &confId) {
		                          return (*confId.getPeerAddress() == *remoteContact);
	                          }) != previousConferenceIds.cend();

	if (previousSession && !found) {
		// Prevents leak
		auto focusOp = focusSession->getPrivate()->getOp();
		if (focusOp) {
			auto focusOpFrom = Address::create(focusOp->getFrom());
			auto focusOpTo = Address::create(focusOp->getTo());
			lInfo() << *this << ": Releasing focus session " << focusSession << " (from: " << *focusOpFrom << " to "
			        << *focusOpTo << ")";
			focusOp->terminate();
			focusOp->release();
		}
	}

	auto session = mFocus->createSession(*this, nullptr, TRUE);
	session->addListener(getSharedFromThis());
	auto from = Address::create(op->getFrom());
	auto to = Address::create(op->getTo());
	session->configure(LinphoneCallIncoming, nullptr, op, from, to);
	session->startIncomingNotification(false);
	setConferenceAddress(remoteContact);

	// If INVITE is for a previous conference ID, only accept the session to acknowledge the BYE
	if (!previousSession && !found) {
		setState(ConferenceInterface::State::CreationPending);
		// Handle participants addition
		const auto resourceList = op->getContentInRemote(ContentType::ResourceLists);
		bool isEmpty = !resourceList || resourceList.value().get().isEmpty();
		if (!isEmpty) {
			const auto participantList = Utils::parseResourceLists(resourceList);
			for (const auto &participantInfo : participantList) {
				const auto &address = participantInfo->getAddress();
				if (!isMe(address)) {
					auto participant = findParticipant(address);
					if (!participant) {
						participant = Participant::create(getSharedFromThis(), address);
						mParticipants.push_back(participant);
					}
				}
			}
		}
	}

	acceptSession(session);
#endif // HAVE_ADVANCED_IM
}

void ClientConference::acceptSession(const shared_ptr<CallSession> &session) {
	if (session->getState() == CallSession::State::UpdatedByRemote) {
		session->acceptUpdate();
	} else {
		session->accept();
	}
}

void ClientConference::attachCall(const shared_ptr<CallSession> &session) {
	setMainSession(session);
	if (getConferenceAddress()) {
		setState(ConferenceInterface::State::CreationPending);
		session->addListener(getSharedFromThis());
		initializeHandlers(this, false);
	} else {
		setConferenceAddress(session->getRemoteContactAddress());
	}
}

std::shared_ptr<CallSession> ClientConference::getMainSession() const {
	auto session = mFocus ? mFocus->getSession() : nullptr;
	return session;
}

void ClientConference::setConferenceId(const ConferenceId &conferenceId) {
	Conference::setConferenceId(conferenceId);

	// Try to update the to field of the call log if the focus is defined.
	if (mFocus) {
		shared_ptr<CallSession> session = mFocus->getSession();
		if (session) {
			std::shared_ptr<Address> address;
			if (conferenceId.getPeerAddress()->isValid()) {
				// Use the peer address of the conference ID because it has also the conf-id param hence the To or From
				// field can be used to search in the map of conferences or chat rooms
				address = conferenceId.getPeerAddress();
			} else {
				address = mFocus->getAddress();
			}
			const auto &direction = session->getDirection();
			auto sessionLog = session->getLog();
			if (direction == LinphoneCallIncoming) {
				sessionLog->setFromAddress(address);
			} else {
				sessionLog->setToAddress(address);
			}
		}
	}
}

int ClientConference::startRecording(const std::string &path) {
	auto session = dynamic_pointer_cast<MediaSession>(getMainSession());
	if (session) {
		session->setRecordPath(path);
		session->startRecording();
	} else {
		lError() << *this << ": ClientConference::startRecording(): no audio session.";
		return -1;
	}
	return 0;
}

bool ClientConference::isIn() const {
	if (isChatOnly()) return true;
	if (mState != ConferenceInterface::State::Created) return false;
	const auto &session = getMainSession();
	if (!session) return false;
	CallSession::State callState = session->getState();
	const auto &focusContactAddress = session->getRemoteContactAddress();
	return (((callState == CallSession::State::UpdatedByRemote) || (callState == CallSession::State::Updating) ||
	         (callState == CallSession::State::StreamsRunning)) &&
	        focusContactAddress->hasUriParam(Conference::ConfIdParameter));
}

void ClientConference::setUtf8Subject(const std::string &subject) {
	if (!getMe()->isAdmin()) {
		lError() << *this << ": Unable to update conference subject because focus " << *getMe()->getAddress()
		         << " is not admin";
		return;
	}
	auto session = dynamic_pointer_cast<MediaSession>(getMainSession());
	if (session) {
		if ((subject.compare(mPendingSubject) != 0) || (getUtf8Subject().empty() && !subject.empty())) {
			mPendingSubject = subject;
			auto updateSubject = [this, subject]() -> LinphoneStatus {
				auto session = dynamic_pointer_cast<MediaSession>(getMainSession());
				if (session) {
					lInfo() << "Sending re-INVITE to update subject from \"" << getUtf8Subject() << "\" to \""
					        << subject << "\"";
					const MediaSessionParams *params = session->getMediaParams();
					MediaSessionParams *currentParams = params->clone();
					auto ret = session->update(currentParams, CallSession::UpdateMethod::Default, false, subject);
					delete currentParams;
					if (ret != 0) {
						lInfo() << "re-INVITE to update subject to \"" << subject << "\" cannot be sent right now";
					}
					return ret;
				}
				return -1;
			};

			if (updateSubject() != 0) {
				session->addPendingAction(updateSubject);
			}
		}
	} else if (!supportsMedia()) {
		lInfo() << *this << ": Sending re-INVITE to update subject from \"" << getUtf8Subject() << "\" to \"" << subject
		        << "\"";
		session = dynamic_pointer_cast<MediaSession>(createSession());
		if (session) {
			session->startInvite(nullptr, subject, nullptr);
		}
	} else {
		mPendingSubject = subject;
		lInfo() << *this << ": Unable to update subject to \"" << subject
		        << "\" right now because the focus session has not been established yet.";
	}
}

bool ClientConference::update(const ConferenceParamsInterface &newParameters) {
	// Any remote participant can change the layout of the conference
	bool ret = false;
	if (getMe()->isAdmin()) {
		ret = Conference::update(newParameters);
	} else {
		lError() << *this << " Unable to update conference parameters because focus " << *getMe()->getAddress()
		         << " is not admin";
	}
	return ret;
}

std::shared_ptr<Call> ClientConference::getCall() const {
	auto session = getMainSession();
	if (session) {
		auto op = session->getPrivate()->getOp();
		return getCore()->getCallByCallId(op->getCallId());
	}
	return nullptr;
}

void ClientConference::setParticipantAdminStatus(const shared_ptr<Participant> &participant, bool isAdmin) {
	if (isAdmin == participant->isAdmin()) return;

	if (!getMe()->isAdmin()) {
		lError() << *this << ": Unable to set admin status of participant " << *participant->getAddress() << " to "
		         << (isAdmin ? "true" : "false") << " because focus " << *getMe()->getAddress() << " is not admin";
		return;
	}

	LinphoneCore *cCore = getCore()->getCCore();
	auto session = getMainSession();

	const auto isChat = mConfParams->chatEnabled();
	SalReferOp *referOp = new SalReferOp(cCore->sal.get());
	linphone_configure_op(cCore, referOp, getConferenceAddress()->toC(), nullptr, false);
	Address referToAddr(*participant->getAddress());
	if (isChat) referToAddr.setParam(Conference::TextParameter);
	referToAddr.setParam(Conference::AdminParameter, Utils::toString(isAdmin));
	referOp->sendRefer(referToAddr.getImpl());
	referOp->unref();
}

void ClientConference::callFocus() {
	if (!mFocus->getSession()) {
		std::shared_ptr<Address> focusAddress = mFocus->getAddress();
		lInfo() << *this << ": Calling the conference focus (" << *focusAddress
		        << ") as there is no session towards the focus yet";
		LinphoneCallParams *params = linphone_core_create_call_params(getCore()->getCCore(), nullptr);
		if (!!linphone_core_get_add_admin_information_to_contact(getCore()->getCCore())) {
			// Participant with the focus call is admin
			L_GET_CPP_PTR_FROM_C_OBJECT(params)->addCustomContactParameter(Conference::AdminParameter,
			                                                               Utils::toString(true));
		}
		linphone_call_params_enable_video(params, mConfParams->videoEnabled());
		Conference::setUtf8Subject(mPendingSubject);
		inviteAddresses({}, params);
		linphone_call_params_unref(params);
	}
}

std::shared_ptr<ParticipantDevice> ClientConference::createParticipantDevice(std::shared_ptr<Participant> participant,
                                                                             std::shared_ptr<Call> call) {
	auto device = Conference::createParticipantDevice(participant, call);
	if (device) {
		// In a client conference, the participant has no session attached ot it.
		device->setSession(nullptr);
		if (!!linphone_core_get_add_admin_information_to_contact(getCore()->getCCore())) {
			const auto &p = device->getParticipant();
			if (p) {
				const_cast<MediaSessionParams *>(call->getParams())
				    ->addCustomContactParameter(Conference::AdminParameter, Utils::toString(p->isAdmin()));
			}
		}
	}
	return device;
}

bool ClientConference::addParticipant(BCTBX_UNUSED(const std::shared_ptr<ParticipantInfo> &info)) {
	lError() << *this << ": addParticipant() with a participant info is not allowed on a ClientConference";
	return false;
}

bool ClientConference::addParticipant(const std::shared_ptr<Address> &participantAddress) {
	// Search call that matches participant session
	const std::list<std::shared_ptr<Call>> &coreCalls = getCore()->getCalls();
	auto callIt = std::find_if(coreCalls.cbegin(), coreCalls.cend(), [&](const std::shared_ptr<Call> &c) {
		return (participantAddress->weakEqual(*c->getRemoteAddress()));
	});
	bool ret = false;
	if (callIt != coreCalls.cend()) {
		std::shared_ptr<Call> call = *callIt;
		ret = addParticipant(call);
	} else {
		const list<std::shared_ptr<Address>> addresses{participantAddress};
		ret = addParticipants(addresses);
	}
	return ret;
}

bool ClientConference::addParticipant(std::shared_ptr<Call> call) {
	if (getMe()->isAdmin()) {
		switch (mState) {
			case ConferenceInterface::State::None:
			case ConferenceInterface::State::Instantiated:
			case ConferenceInterface::State::CreationFailed:
			case ConferenceInterface::State::CreationPending:
			case ConferenceInterface::State::Created:
				callFocus();
				// The call is added to the invited participant list and not the actual participant list as the latter
				// is generated by the information received by the server. The object ClientConferenceEventHandler, in
				// fact, manages the participant list according to the informations received by the server. Upon
				// reception of a full state, it may remove participant that are not in the body of the NOTIFY to ensure
				// a synchronization between the server and the client. By doing that, the client will not be able
				// anymore to transfer the call to the server and make it join the conference
				if (focusIsReady()) {
					addInvitedParticipant(call);
					transferToFocus(call);
				} else {
					auto callIt = std::find(mPendingCalls.begin(), mPendingCalls.end(), call);
					if (callIt == mPendingCalls.end()) {
						lInfo() << "Adding " << *call << " to the list of call to add to " << *this;
						mPendingCalls.push_back(call);
						addInvitedParticipant(call);
					} else {
						lError() << "Trying to add " << *call << " twice to " << *this;
					}
				}
				return true;
			default:
				lError() << "Could not add " << *call << " to " << *this << ". Bad conference state ("
				         << Utils::toString(mState) << ")";
				return false;
		}
	} else if (!getMe()->isAdmin()) {
		lError() << "Could not add call " << *call << " to " << *this << " because local participant "
		         << *getMe()->getAddress() << " is not admin.";
	} else {
		const auto &endTime = mConfParams->getEndTime();
		const auto &startTime = mConfParams->getStartTime();
		const auto now = time(NULL);
		lError() << "Could not add " << *call << " to " << *this << " because the conference is not active right now.";
		if (startTime >= 0) {
			lError() << "Expected start time (" << startTime << "): " << mConfParams->getStartTimeString();
		} else {
			lError() << "Expected start time: none";
		}
		if (endTime >= 0) {
			lError() << "Expected end time (" << endTime << "): " << mConfParams->getEndTimeString();
		} else {
			lError() << "Expected end time: none";
		}
		lError() << "Now: " << Utils::timeToIso8601(now);
		return false;
	}
	return false;
}

// Removes own address and existing participants from the list.
// Also removes gruu from kept addresses
std::list<Address> ClientConference::cleanAddressesList(const list<std::shared_ptr<Address>> &addresses) const {
	std::list<Address> cleanedList;
	const auto &meAddress = getMe()->getAddress();
	for (const auto &address : addresses) {
		if (!findParticipant(address) && !(address->weakEqual(*meAddress))) {
			cleanedList.push_back(*address);
		}
	}
	cleanedList.sort([](const auto &addr1, const auto &addr2) { return addr1 < addr2; });
	cleanedList.unique([](const auto &addr1, const auto &addr2) { return addr1.weakEqual(addr2); });
	return cleanedList;
}

bool ClientConference::addParticipants(const std::list<std::shared_ptr<Call>> &calls) {
	const auto &coreCurrentCall = getCore()->getCurrentCall();
	const bool startingConference = (mState == ConferenceInterface::State::CreationPending);
	const auto &outputDevice = (coreCurrentCall) ? coreCurrentCall->getOutputAudioDevice() : nullptr;
	const auto &inputDevice = (coreCurrentCall) ? coreCurrentCall->getInputAudioDevice() : nullptr;
	if (getMe()->isAdmin()) {
		bool success = Conference::addParticipants(calls);
		// If current call is not NULL and the conference is in the creating pending state or instantied, then try to
		// change audio route to keep the one currently used Do not change audio route if participant addition is not
		// successful
		if (success && startingConference) {
			if (outputDevice) {
				setOutputAudioDevice(outputDevice);
			}
			if (inputDevice) {
				setInputAudioDevice(inputDevice);
			}
		}
	}

	return false;
}

bool ClientConference::addParticipants(const list<std::shared_ptr<Address>> &addresses) {
	const auto isChat = mConfParams->chatEnabled();
	if (getMe()->isAdmin() || ((mState == ConferenceInterface::State::Instantiated) && isChat)) {
		const auto mediaSupported = supportsMedia();
		if ((mState == ConferenceInterface::State::Instantiated) ||
		    (mState == ConferenceInterface::State::CreationPending)) {
			if (mediaSupported) {
				inviteAddresses(addresses, nullptr);
			} else if (isChat) {
				auto addressesList = cleanAddressesList(addresses);
				if (addressesList.empty()) {
					lError() << *this << ": no new participants were given.";
					return false;
				}
				if (!mConfParams->isGroup() && (addressesList.size() > 1 || getParticipantCount() != 0)) {
					lError() << *this << ": Cannot add more than one participant in a one-to-one chatroom";
					return false;
				}

				auto content = Content::create();
				content->setBodyFromUtf8(Utils::getResourceLists(addressesList));
				content->setContentType(ContentType::ResourceLists);
				content->setContentDisposition(ContentDisposition::RecipientList);
				if (linphone_core_content_encoding_supported(getCore()->getCCore(), "deflate")) {
					content->setContentEncoding("deflate");
				}
				auto session = createSession();
				session->startInvite(nullptr, getUtf8Subject(), content);
			}
		} else if (mState == ConferenceInterface::State::Created) {
			SalReferOp *referOp = new SalReferOp(getCore()->getCCore()->sal.get());
			LinphoneAddress *lAddr = getConferenceAddress()->toC();
			linphone_configure_op(getCore()->getCCore(), referOp, lAddr, nullptr, true);

			const auto &factoryAddress = mConfParams->getConferenceFactoryAddress();
			std::list<std::string> addressParams;
			if (supportsMedia() && !factoryAddress) {
				addressParams.push_back(Conference::IsFocusParameter);
			}
			if (isChat) {
				addressParams.push_back(Conference::TextParameter);
			}

			for (const auto &addr : addresses) {
				std::shared_ptr<Address> referToAddr = addr->clone()->toSharedPtr();
				for (const auto &param : addressParams) {
					referToAddr->setParam(param);
				}
				referOp->sendRefer(referToAddr->getImpl());
			}
			referOp->unref();
		}
	} else {
		const auto &endTime = mConfParams->getEndTimeString();
		const auto now = time(NULL);
		lError() << "Could not add participants to " << *this << " because local participant " << *getMe()->getAddress()
		         << " is not admin or conference already ended (expected endtime: " << endTime
		         << " now: " << Utils::timeToIso8601(now);
		return false;
	}

	return true;
}

void ClientConference::endConference() {
	// Take a ref as conference may be deleted
	shared_ptr<Conference> ref = getSharedFromThis();
	setState(ConferenceInterface::State::TerminationPending);
	if (!mFinalized) {
		Conference::terminate();
		setState(ConferenceInterface::State::Terminated);
	}
}

bool ClientConference::focusIsReady() const {
	const auto &session = getMainSession();
	if (!session || !session->getRemoteContactAddress()) return false;
	const auto focusState = session->getState();
	return (focusState == CallSession::State::StreamsRunning) || (focusState == CallSession::State::Paused);
}

bool ClientConference::transferToFocus(std::shared_ptr<Call> call) {
	auto session = getMainSession();
	std::shared_ptr<Address> referToAddr(session->getRemoteContactAddress());
	const std::shared_ptr<Address> &remoteAddress = call->getRemoteAddress();
	std::shared_ptr<Participant> participant = findInvitedParticipant(remoteAddress);
	if (participant) {
		referToAddr->setParam(Conference::AdminParameter, Utils::toString(participant->isAdmin()));
		// Add the participant to the list of the participants of the conference if the core does not support RFC4575
		// (Conference Event Package) or the macro HAVE_ADVANCED_IM is not defined. In such a scenario, we make the best
		// effort to ensure the client has a list of participant as up to date as possible
#ifdef HAVE_ADVANCED_IM
		bool eventLogEnabled = !!linphone_config_get_bool(linphone_core_get_config(getCore()->getCCore()), "misc",
		                                                  "conference_event_log_enabled", TRUE);
		if (!eventLogEnabled) {
#endif // HAVE_ADVANCED_IM
			mParticipants.push_back(participant);
#ifdef HAVE_ADVANCED_IM
		}
#endif // HAVE_ADVANCED_IM
		lInfo() << *this << ": Transfering " << *call << " to focus " << *referToAddr;
		updateParticipantInConferenceInfo(participant);
		if (call->transfer(referToAddr->toString()) == 0) {
			mTransferingCalls.push_back(call);
			return true;
		} else {
			lError() << *this << ": could not transfer " << *call << " to focus " << *referToAddr << " right now";
			return false;
		}
	} else {
		lError() << *this << ": could not transfer " << *call << " to focus " << *referToAddr
		         << " because participant with address " << *remoteAddress << " cannot be found";
		return false;
	}
	return false;
}

void ClientConference::reset() {
	auto session = getMainSession();
	if (session) {
		auto op = session->getPrivate()->getOp();
		auto sessionCall = op ? getCore()->getCallByCallId(op->getCallId()) : nullptr;
		if (sessionCall) {
			sessionCall->setConference(nullptr);
		}
	}
	mPendingCalls.clear();
	mTransferingCalls.clear();
}

void ClientConference::onFocusCallStateChanged(CallSession::State state, BCTBX_UNUSED(const std::string &message)) {
	if (supportsMedia()) {
		auto session = getMainSession();
		std::shared_ptr<Address> focusContactAddress;
		std::shared_ptr<Call> call = nullptr;

		// Take a ref as conference may be deleted when the call goes to states PausedByRemote or End
		shared_ptr<Conference> ref = getSharedFromThis();
		SalCallOp *op = nullptr;

		if (session) {
			focusContactAddress = session->getRemoteContactAddress();
			op = session->getPrivate()->getOp();
			call = op ? getCore()->getCallByCallId(op->getCallId()) : nullptr;
		}

		const bool isFocusFound =
		    focusContactAddress ? focusContactAddress->hasParam(Conference::IsFocusParameter) : false;
		list<std::shared_ptr<Call>>::iterator it;
		switch (state) {
			case CallSession::State::StreamsRunning: {
#ifdef HAVE_ADVANCED_IM
				// Handle session reconnection if network dropped only for a short period of time
				if (mEventHandler && mEventHandler->needToSubscribe()) {
					const auto &conferenceId = getConferenceId();
					if (conferenceId.isValid()) {
						mEventHandler->subscribe(conferenceId);
					}
				}
				if (mClientEktManager) {
					mClientEktManager->subscribe();
				}
#endif // HAVE_ADVANCED_IM
				updateParticipantInConferenceInfo(getMe());
				for (const auto &device : getParticipantDevices()) {
					device->updateStreamAvailabilities();
				}
				if (isFocusFound &&
				    ((call && !call->mediaInProgress()) ||
				     !!!linphone_config_get_int(linphone_core_get_config(session->getCore()->getCCore()), "sip",
				                                "update_call_when_ice_completed", TRUE)) &&
				    mFinalized && mFullStateReceived && mScheduleUpdate) {
					auto requestStreams = [this]() -> LinphoneStatus {
						lInfo() << "Sending re-INVITE in order to get streams after joining " << *this;
						setState(ConferenceInterface::State::Created);
						auto ret = updateMainSession(!mFullStateUpdate);
						return ret;
					};

					if (requestStreams() != 0) {
						lInfo() << "Delaying re-INVITE in order to get streams after joining " << *this
						        << " because the dialog is not available yet to accept this transaction";
						session->addPendingAction(requestStreams);
					}
					// An update has been successfully sent therefore clear the flag mScheduleUpdate to avoid sending it
					// twice.
					mScheduleUpdate = false;
					mFullStateUpdate = false;
				}
			}
				BCTBX_NO_BREAK; /* Intentional no break */
			case CallSession::State::Connected:
				if (isFocusFound) {
					setConferenceAddress(focusContactAddress);
				}
				BCTBX_NO_BREAK; /* Intentional no break */
			case CallSession::State::Paused:
			case CallSession::State::UpdatedByRemote:
				if (isFocusFound) {
					it = mPendingCalls.begin();
					while (it != mPendingCalls.end()) {
						std::shared_ptr<Call> pendingCall = *it;
						CallSession::State pendingCallState = pendingCall->getState();
						if ((pendingCallState == CallSession::State::StreamsRunning) ||
						    (pendingCallState == CallSession::State::Paused)) {
							if (transferToFocus(pendingCall)) {
								it = mPendingCalls.erase(it);
							} else {
								// If the transfer failed, move on to the next one
								it++;
							}
						} else {
							it++;
						}
					}

					if (!mFinalized) {
						setConferenceId(ConferenceId(focusContactAddress, getMe()->getAddress(),
						                             getCore()->createConferenceIdParams()));
						if (call) {
							if (focusContactAddress->hasUriParam(Conference::ConfIdParameter)) {
								call->setConferenceId(
								    focusContactAddress->getUriParamValue(Conference::ConfIdParameter));
							}
							if (!call->mediaInProgress() ||
							    !!!linphone_config_get_int(linphone_core_get_config(session->getCore()->getCCore()),
							                               "sip", "update_call_when_ice_completed", TRUE)) {
								finalizeCreation();
							}
						}
					}
				}
				BCTBX_NO_BREAK; /* Intentional no break */
			case CallSession::State::PausedByRemote:
				if (!focusContactAddress->hasParam(Conference::IsFocusParameter) &&
				    (state != CallSession::State::Connected)) {
					// The call was in conference and the focus removed its attribute to show that the call exited the
					// conference
					lInfo() << "Ending " << *this << " because server removed '" << Conference::IsFocusParameter
					        << "' attribute from its remote address " << *focusContactAddress;
					endConference();
				}
				break;
			case CallSession::State::Error:
				setState(ConferenceInterface::State::CreationFailed);
				if (call) call->setConference(nullptr);
				it = mPendingCalls.begin();
				while (it != mPendingCalls.end()) {
					std::shared_ptr<Call> pendingCall = *it;
					pendingCall->setConference(nullptr);
					it++;
				}
				setMainSession(nullptr);
				break;
			case CallSession::State::End:
				lInfo() << "Ending " << *this << " because focus call (local address "
				        << (session ? session->getLocalAddress()->toString() : "sip:") << " remote address "
				        << (session ? session->getRemoteAddress()->toString() : "sip:") << ") has ended";
				endConference();
				break;
			default:
				break;
		}

		const auto &confState = ref->getState();
		auto ms = static_pointer_cast<MediaSession>(session);
		// Do not send updates if the conference is ending or about to end or the call is in temporary state
		if ((confState != ConferenceInterface::State::Terminated) &&
		    (confState != ConferenceInterface::State::TerminationPending) && mScheduleUpdate &&
		    ms->getPrivate()->canSoundResourcesBeFreed()) {
			lInfo() << "Executing scheduled update of the focus session of " << *this;
			if (updateMainSession(!mFullStateUpdate) == 0) {
				if (confState == ConferenceInterface::State::CreationPending) {
					setState(ConferenceInterface::State::Created);
				}
				mScheduleUpdate = false;
				mFullStateUpdate = false;
			} else {
				lInfo() << "Scheduled update of the focus session of " << *this
				        << " cannot be executed right now - Retrying at the next state change";
			}
		}
	} else if (isChatOnly()) {
#ifdef HAVE_ADVANCED_IM
		auto session = mFocus->getSession();
		auto chatRoom = getChatRoom();
		auto clientGroupChatRoom = dynamic_pointer_cast<ClientChatRoom>(chatRoom);
		switch (state) {
			case CallSession::State::Connected: {
				if ((mState == ConferenceInterface::State::Instantiated) ||
				    (mState == ConferenceInterface::State::CreationPending)) {
					if (!mConfParams->getAccount()) {
						mConfParams->setAccount(session->getParams()->getAccount());
					}
					if (clientGroupChatRoom->isLocalExhumePending()) {
						clientGroupChatRoom->onLocallyExhumedConference(session->getRemoteContactAddress());
					} else {
						clientGroupChatRoom->onChatRoomCreated(session->getRemoteContactAddress());
					}
					getCore()->getPrivate()->insertChatRoomWithDb(chatRoom, getLastNotify());
				} else if (mState == ConferenceInterface::State::TerminationPending) {
					/* This is the case where we have re-created the session in order to quit the chatroom.
					 * In this case, defer the sending of the bye so that it is sent after the ACK.
					 * Indeed, the ACK is sent immediately after being notified of the Connected state.*/
					getCore()->doLater([session]() {
						if (session) {
							session->terminate();
						}
					});
				}
			} break;
			case CallSession::State::End: {
				const auto errorInfo = session->getErrorInfo();
				const auto code = linphone_error_info_get_protocol_code(errorInfo);
				if (errorInfo != nullptr && code > 299) {
					lWarning() << *this
					           << ": received a BYE with reason: " << linphone_error_info_get_protocol_code(errorInfo)
					           << ", not leaving it.";
				} else {
					const auto &clientConferenceAddress = session->getRemoteAddress();
					bool found = false;
					auto previousConferenceIds = clientGroupChatRoom->getPreviousConferenceIds();
					for (auto it = previousConferenceIds.begin(); it != previousConferenceIds.end(); it++) {
						ConferenceId confId = static_cast<ConferenceId>(*it);
						if (*confId.getPeerAddress() == *clientConferenceAddress) {
							lInfo() << *this << ": found previous chat room conference ID [" << confId
							        << "] for chat room with current ID [" << getConferenceId() << "]";
							clientGroupChatRoom->removeConferenceIdFromPreviousList(confId);
							found = true;
							break;
						}
					}

					const auto isLocalExhume = clientGroupChatRoom && clientGroupChatRoom->isLocalExhumePending();

					if (found) {
						/* This is the case where we are accepting a BYE for an already exhumed chat room, don't change
						 * it's state */
						lInfo() << *this << ": received a BYE referred to previous conference address ["
						        << *clientConferenceAddress << "] before the exhume has been terminated";
					} else if (isLocalExhume) {
						lInfo() << *this << " has been successfully recreated";
					} else {
						setState(ConferenceInterface::State::TerminationPending);
					}
				}
			} break;
			case CallSession::State::Released: {
				if (mState == ConferenceInterface::State::TerminationPending) {
					const auto &reason = session->getReason();
					if ((reason == LinphoneReasonNone) || (reason == LinphoneReasonDeclined)) {
						// Everything is fine, the chat room has been left on the server side.
						// Or received 603 Declined, the chat room has been left on the server side but
						// remains local.
						setState(ConferenceInterface::State::Terminated);
					} else {
						// Go to state TerminationFailed and then back to Created since it has not been terminated
						setState(ConferenceInterface::State::TerminationFailed);
						setState(ConferenceInterface::State::Created);
					}
				}
				break;
			}
			case CallSession::State::Error: {
				const auto &reason = session->getReason();
				if ((mState == ConferenceInterface::State::Instantiated) ||
				    (mState == ConferenceInterface::State::CreationPending)) {
					setState(ConferenceInterface::State::CreationFailed);
					// If there are chat message pending chat room creation, set state to NotDelivered and remove them
					// from queue.
					const std::list<std::shared_ptr<ChatMessage>> &pendingCreationMessages =
					    clientGroupChatRoom->getPendingCreationMessages();
					for (const auto &msg : pendingCreationMessages) {
						msg->getPrivate()->setParticipantState(getMe()->getAddress(), ChatMessage::State::NotDelivered,
						                                       ::ms_time(nullptr));
					}
					clientGroupChatRoom->clearPendingCreationMessages();
					if (reason == LinphoneReasonForbidden) {
						setState(ConferenceInterface::State::Terminated);
						chatRoom->deleteFromDb();
					}
				} else if (mState == ConferenceInterface::State::TerminationPending) {
					if (reason == LinphoneReasonNotFound) {
						// Somehow the chat room is no longer known on the server, so terminate it
						setState(ConferenceInterface::State::Terminated);
						setState(ConferenceInterface::State::Deleted);
					} else {
						// Go to state TerminationFailed and then back to Created since it has not been terminated
						setState(ConferenceInterface::State::TerminationFailed);
						setState(ConferenceInterface::State::Created);
					}
				}
				setMainSession(nullptr);
			} break;
			default:
				lDebug() << "Unhandled state " << Utils::toString(state) << " in " << *this;
				break;
		}
		if (chatRoom) {
			linphone_chat_room_notify_session_state_changed(chatRoom->toC(), static_cast<LinphoneCallState>(state),
			                                                L_STRING_TO_C(message));
		}
#endif // HAVE_ADVANCED_IM
	} else {
		switch (state) {
			case CallSession::State::Error:
				if ((mState == ConferenceInterface::State::Instantiated) ||
				    (mState == ConferenceInterface::State::CreationPending)) {
					setState(ConferenceInterface::State::CreationFailed);
				} else if (mState == ConferenceInterface::State::Created) {
					setState(ConferenceInterface::State::TerminationPending);
				}
				break;
			case CallSession::State::End:
			case CallSession::State::Released:
				break;
			default:
				lError() << *this
				         << " has no capability set and it has been requested to handle a call session whose state "
				            "changed to "
				         << Utils::toString(state);
				abort();
				break;
		}
	}
}

void ClientConference::onPendingCallStateChanged(std::shared_ptr<Call> call, CallSession::State callState) {
	auto session = getMainSession();
	switch (callState) {
		case CallSession::State::StreamsRunning:
		case CallSession::State::Paused: {
			if (session) {
				CallSession::State focusCallState = session->getState();
				// Transfer call to focus is the conference is in creation pending or created state and if the focus
				// call is in connected, streams running or updated by remote state
				if (((focusCallState == CallSession::State::StreamsRunning) ||
				     (focusCallState == CallSession::State::Connected) ||
				     (focusCallState == CallSession::State::UpdatedByRemote)) &&
				    ((mState == ConferenceInterface::State::CreationPending) ||
				     (mState == ConferenceInterface::State::Created))) {
					// Transfer call only if focus call remote contact address is available (i.e. the call has been
					// correctly established and passed through state StreamsRunning)
					if (session->getRemoteContactAddress()) {
						if (transferToFocus(call)) {
							mPendingCalls.remove(call);
						}
					}
				}
			}
		} break;
		case CallSession::State::Error:
		case CallSession::State::End:
			mPendingCalls.remove(call);
			Conference::removeParticipant(call);
			if ((mParticipants.size() + mPendingCalls.size() + mTransferingCalls.size()) == 0) terminate();
			break;
		default:
			break;
	}
}

void ClientConference::onCallSessionTransferStateChanged(const std::shared_ptr<CallSession> &session,
                                                         CallSession::State state) {
	auto op = session->getPrivate()->getOp();
	shared_ptr<Call> transferred = op ? getCore()->getCallByCallId(op->getCallId()) : nullptr;
	if (transferred) {
		switch (state) {
			case CallSession::State::Connected:
				mTransferingCalls.push_back(transferred);
				break;
			case CallSession::State::Error:
				mTransferingCalls.remove(transferred);
				Conference::removeParticipant(transferred);
				if ((mParticipants.size() + mPendingCalls.size() + mTransferingCalls.size()) == 0) terminate();
				break;
			default:
				break;
		}
	}
}

// -----------------------------------------------------------------------------
void ClientConference::onStateChanged(ConferenceInterface::State state) {
	auto session = getMainSession();
	string subject = getUtf8Subject();
	const auto mediaSupported = supportsMedia();

	shared_ptr<Call> sessionCall = nullptr;
	if (session) {
		auto op = session->getPrivate()->getOp();
		sessionCall = op ? getCore()->getCallByCallId(op->getCallId()) : nullptr;
	}

	switch (state) {
		case ConferenceInterface::State::None:
		case ConferenceInterface::State::Instantiated:
		case ConferenceInterface::State::CreationPending:
		case ConferenceInterface::State::TerminationFailed:
		case ConferenceInterface::State::Terminated:
			break;
		case ConferenceInterface::State::CreationFailed:
			reset();
			if (mediaSupported) {
				Conference::terminate();
			}
			break;
		case ConferenceInterface::State::Created:
			if (session && getMe()->isAdmin() && !mPendingSubject.empty() && (subject.compare(mPendingSubject) != 0)) {
				setUtf8Subject(mPendingSubject);
			}
			break;
		case ConferenceInterface::State::TerminationPending:
#ifdef HAVE_ADVANCED_IM
			if (mEventHandler) {
				mEventHandler->unsubscribe();
			}
#endif // HAVE_ADVANCED_IM
			resetLastNotify();
			if (session) {
				// Do not terminate focus call when terminating the client conference
				// This is required because the local conference creates a client conference for every participant and
				// the call from the participant to the local conference is the focus call
				if (sessionCall) {
					sessionCall->setConference(nullptr);
				}
			}
			if (mediaSupported) {
				getCore()->doLater([this]() {
					Conference::terminate();
					setState(ConferenceInterface::State::Terminated);
				});
			}
			break;
		case ConferenceInterface::State::Deleted:
			reset();
			break;
	}
}

void ClientConference::onParticipantAdded(const shared_ptr<ConferenceParticipantEvent> &event,
                                          BCTBX_UNUSED(const std::shared_ptr<Participant> &participant)) {
	const std::shared_ptr<Address> &pAddr = event->getParticipantAddress();
	if (mState == ConferenceInterface::State::Instantiated) {
		return; // The conference has just been instanted and it may be adding participants quite quickly
	}
	lInfo() << "Updating conference information of " << *this << " because the core has been notified that participant "
	        << *pAddr << " has been added";
	// When receiving a participant added notification, we must recreate the conference informations in order to get the
	// participant list up to date
	updateAndSaveConferenceInformations();

	if (isMe(pAddr)) {
		if (mState == ConferenceInterface::State::CreationPending) {
			createEventHandler(this);
		}
	} else if (findParticipant(pAddr)) {
		lInfo() << "Addition of participant with address " << *pAddr << " to " << *this << " has been successful";
	} else {
		lWarning() << "Addition of participant with address " << *pAddr
		           << " has been failed because the participant is not part of " << *this;
	}

	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		if (event->getFullState()) return;
		chatRoom->addEvent(event);
		_linphone_chat_room_notify_participant_added(chatRoom->toC(), L_GET_C_BACK_PTR(event));
	}
}

void ClientConference::onParticipantRemoved(const shared_ptr<ConferenceParticipantEvent> &event,
                                            BCTBX_UNUSED(const std::shared_ptr<Participant> &participant)) {
	if (mState == ConferenceInterface::State::Instantiated)
		return; // The conference has just been instanted and it may be removing participants quite quickly
	const std::shared_ptr<Address> &pAddr = event->getParticipantAddress();
	if (isMe(pAddr)) {
		lInfo() << "Unsubscribing all devices of me (address " << *pAddr << ") from " << this;
		// Unsubscribe all devices of me
		std::for_each(getMe()->getDevices().cbegin(), getMe()->getDevices().cend(),
		              [&](const std::shared_ptr<ParticipantDevice> &device) {
			              shared_ptr<EventSubscribe> ev = device->getConferenceSubscribeEvent();
			              if (ev) {
				              // try to terminate subscription if any, but do not wait for answer.
				              ev->clearCallbacksList();
				              ev->terminate();
			              }
		              });
	} else if (!findParticipant(pAddr)) {
		lInfo() << "Removal of participant with address " << *pAddr << " from " << *this << " has been successful";
	} else {
		lWarning() << "Removal of participant with address " << *pAddr
		           << " has been failed because the participant is still part of " << *this;
	}

	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		chatRoom->addEvent(event);
		_linphone_chat_room_notify_participant_removed(chatRoom->toC(), L_GET_C_BACK_PTR(event));
	}
}

void ClientConference::onParticipantSetAdmin(const shared_ptr<ConferenceParticipantEvent> &event,
                                             BCTBX_UNUSED(const std::shared_ptr<Participant> &participant)) {
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		if (event->getFullState()) return;
		chatRoom->addEvent(event);
		_linphone_chat_room_notify_participant_admin_status_changed(chatRoom->toC(), L_GET_C_BACK_PTR(event));
	}
}

void ClientConference::onParticipantSetRole(BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantEvent> &event),
                                            const std::shared_ptr<Participant> &participant) {
	lInfo() << "Updating conference information of " << *this << " because the core has been notified that participant "
	        << *participant->getAddress() << " has changed its role to " << participant->getRole();
	updateAndSaveConferenceInformations();
}

void ClientConference::onSubjectChanged(BCTBX_UNUSED(const std::shared_ptr<ConferenceSubjectEvent> &event)) {
	lInfo() << "Updating conference information of " << *this
	        << " because the core has been notified that the subject has been changed to " << getSubject();
	updateAndSaveConferenceInformations();

	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		if (event->getFullState()) return;
		chatRoom->addEvent(event);
		LinphoneChatRoom *cr = chatRoom->toC();
		_linphone_chat_room_notify_subject_changed(cr, L_GET_C_BACK_PTR(event));
		linphone_core_notify_chat_room_subject_changed(getCore()->getCCore(), cr);
	}
}

void ClientConference::onParticipantDeviceAdded(
    BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event),
    const std::shared_ptr<ParticipantDevice> &device) {
	auto session = dynamic_pointer_cast<MediaSession>(getMainSession());
	if (session && mConfParams->audioEnabled() && isMe(device->getAddress())) {
		notifyLocalMutedDevices(session->getPrivate()->getMicrophoneMuted());
	}

#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		// Check if new device degrades the chatroom security level and return corresponding security event
		shared_ptr<ConferenceSecurityEvent> securityEvent = nullptr;
		auto encryptionEngine = getCore()->getEncryptionEngine();
		if (encryptionEngine) {
			const std::shared_ptr<Address> &addr = event->getParticipantAddress();
			shared_ptr<Participant> participant;
			if (isMe(addr)) participant = getMe();
			else participant = findParticipant(addr);
			ChatRoom::SecurityLevel currentSecurityLevel = getSecurityLevelExcept(device);
			securityEvent =
			    encryptionEngine->onDeviceAdded(event->getDeviceAddress(), participant, chatRoom, currentSecurityLevel);
		}

		if (event->getFullState()) return;
		chatRoom->addEvent(event);

		if (securityEvent) onSecurityEvent(securityEvent);
		_linphone_chat_room_notify_participant_device_added(chatRoom->toC(), L_GET_C_BACK_PTR(event));
	}
#endif // HAVE_ADVANCED_IM
}

void ClientConference::onParticipantDeviceRemoved(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event,
                                                  const std::shared_ptr<ParticipantDevice> &device) {
	auto session = dynamic_pointer_cast<MediaSession>(getMainSession());
	if (session) {
		const MediaSessionParams *params = session->getMediaParams();

		const auto &confSecurityLevel = mConfParams->getSecurityLevel();
		const auto &deviceAddress = device->getAddress();
		const auto &audioAvailable = device->getStreamAvailability(LinphoneStreamTypeAudio);
		const auto audioNeedsReInvite = ((confSecurityLevel == ConferenceParams::SecurityLevel::EndToEnd) &&
		                                 mConfParams->audioEnabled() && params->audioEnabled() && audioAvailable);
		const auto videoNeedsReInvite = (mConfParams->videoEnabled() && params->videoEnabled());

		updateMinatureRequestedFlag();

		// If the device was screen sharing, then notify the application that it isn't anymore
		if (device->enableScreenSharing(false)) {
			notifyParticipantDeviceScreenSharingChanged(ms_time(NULL), event->getFullState(), device->getParticipant(),
			                                            device);
		}

		// If the device was the active speaker, then notify the application that it isn't anymore
		if (device == getActiveSpeakerParticipantDevice()) {
			notifyActiveSpeakerParticipantDevice(nullptr);
		}

		if ((audioNeedsReInvite || videoNeedsReInvite) && (mState == ConferenceInterface::State::Created) &&
		    !isMe(deviceAddress) && (device->getTimeOfJoining() >= 0)) {
			auto updateSession = [this, deviceAddress]() -> LinphoneStatus {
				lInfo() << "Sending re-INVITE in order to update streams because participant device " << *deviceAddress
				        << " has been removed from " << *this;
				auto ret = updateMainSession();
				if (ret != 0) {
					lInfo() << "re-INVITE to update streams because participant device " << deviceAddress
					        << " has been removed from " << *this << " cannot be sent right now";
				}
				return ret;
			};

			if (updateSession() != 0) {
				mScheduleUpdate = true;
			}
		}
	}

	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		chatRoom->addEvent(event);
		_linphone_chat_room_notify_participant_device_removed(chatRoom->toC(), L_GET_C_BACK_PTR(event));
	}
}

void ClientConference::onParticipantDeviceStateChanged(
    BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event),
    const std::shared_ptr<ParticipantDevice> &device) {
	auto session = dynamic_pointer_cast<MediaSession>(getMainSession());
	if (session) {
		const MediaSessionParams *params = session->getMediaParams();

		auto callIt = std::find_if(mPendingCalls.cbegin(), mPendingCalls.cend(), [&device](const auto &call) {
			if (!call) return false;
			const auto &devAddr = device->getAddress();
			const auto contactAddress = call->getActiveSession()->getRemoteContactAddress()->getUri();
			return (*devAddr == contactAddress);
		});

		bool changed = updateMinatureRequestedFlag();
		const auto &deviceAddress = device->getAddress();
		const auto &audioAvailable = device->getStreamAvailability(LinphoneStreamTypeAudio);
		const auto &confSecurityLevel = mConfParams->getSecurityLevel();
		const auto audioNeedsReInvite = ((confSecurityLevel == ConferenceParams::SecurityLevel::EndToEnd) &&
		                                 mConfParams->audioEnabled() && params->audioEnabled() && audioAvailable);
		const auto &videoAvailable = device->getStreamAvailability(LinphoneStreamTypeVideo);
		const auto videoNeedsReInvite = (mConfParams->videoEnabled() && params->videoEnabled() && videoAvailable);
		if ((mState == ConferenceInterface::State::Created) && (callIt == mPendingCalls.cend()) && isIn() &&
		    (device->getState() == ParticipantDevice::State::Present) &&
		    ((videoNeedsReInvite || audioNeedsReInvite || changed)) && !isMe(deviceAddress)) {
			auto updateSession = [this, deviceAddress]() -> LinphoneStatus {
				lInfo() << "Sending re-INVITE in order to get streams for participant device " << *deviceAddress
				        << " that joined recently the " << *this;
				auto ret = updateMainSession();
				if (ret != 0) {
					lInfo() << "re-INVITE to get streams for participant device " << *deviceAddress
					        << " that recently joined the " << *this << " cannot be sent right now";
				}
				return ret;
			};

			if (updateSession() != 0) {
				mScheduleUpdate = true;
			}
		}
	}

	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		if (event->getFullState()) return;
		chatRoom->addEvent(event);
		getCore()->getPrivate()->mainDb->updateChatRoomParticipantDevice(chatRoom, device);
		_linphone_chat_room_notify_participant_device_state_changed(chatRoom->toC(), L_GET_C_BACK_PTR(event),
		                                                            (LinphoneParticipantDeviceState)device->getState());
	}
}

void ClientConference::onParticipantDeviceMediaAvailabilityChanged(
    BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event),
    const std::shared_ptr<ParticipantDevice> &device) {
	if (supportsMedia()) {
		const auto &deviceAddress = device->getAddress();
		if ((!isMe(deviceAddress)) && (mState == ConferenceInterface::State::Created) && isIn()) {
			updateMinatureRequestedFlag();
			auto updateSession = [this, deviceAddress]() -> LinphoneStatus {
				lInfo() << *this << ": Sending re-INVITE because device " << *deviceAddress
				        << " has changed its stream availability";
				auto ret = updateMainSession();
				if (ret != 0) {
					lInfo() << *this << ": re-INVITE due to device " << *deviceAddress
					        << " changing its stream availability cannot be sent right now";
				}
				return ret;
			};

			if (updateSession() != 0) {
				mScheduleUpdate = true;
			}
		}
	}

	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		if (event->getFullState()) return;
		chatRoom->addEvent(event);
		_linphone_chat_room_notify_participant_device_media_availability_changed(chatRoom->toC(),
		                                                                         L_GET_C_BACK_PTR(event));
	}
}

void ClientConference::onAvailableMediaChanged(
    BCTBX_UNUSED(const std::shared_ptr<ConferenceAvailableMediaEvent> &event)) {
	auto session = dynamic_pointer_cast<MediaSession>(getMainSession());
	const bool videoEnabled = (session) ? session->getCurrentParams()->videoEnabled() : false;
	if (!mConfParams->videoEnabled() && videoEnabled && isIn()) {
		auto updateSession = [this]() -> LinphoneStatus {
			lInfo() << *this << ": Sending re-INVITE because the conference has no longer video capabilities";
			auto ret = updateMainSession();
			if (ret != 0) {
				lInfo() << *this << ": re-INVITE to remove video cannot be sent right now";
			}
			return ret;
		};

		if (updateSession() != 0) {
			mScheduleUpdate = true;
		}
	}
}

void ClientConference::onParticipantsCleared() {
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		// clear from db as well
		auto &mainDb = getCore()->getPrivate()->mainDb;
		for (const auto &participant : mParticipants) {
			mainDb->deleteChatRoomParticipant(chatRoom, participant->getAddress());
			for (const auto &device : participant->getDevices()) {
				mainDb->deleteChatRoomParticipantDevice(chatRoom, device);
			}
		}
		mainDb->deleteChatRoomParticipant(chatRoom, getMe()->getAddress());
		for (const auto &device : getMe()->getDevices()) {
			mainDb->deleteChatRoomParticipantDevice(chatRoom, device);
		}
	}
#endif // HAVE_ADVANCED_IM

	clearParticipants();
}

void ClientConference::onConferenceCreated(BCTBX_UNUSED(const std::shared_ptr<Address> &addr)) {
#ifdef HAVE_ADVANCED_IM
	const auto &conferenceId = getConferenceId();
	const ConferenceId newConferenceId(addr, conferenceId.getLocalAddress(), getCore()->createConferenceIdParams());
	if (!getCore()->getPrivate()->findExumedChatRoomFromPreviousConferenceId(newConferenceId)) {
		setConferenceId(newConferenceId);
	}
	setConferenceAddress(addr);
	lInfo() << *this << ": Conference [" << conferenceId << "] has been created with address "
	        << *getConferenceAddress();

	mFocus->setAddress(addr);
	mFocus->clearDevices();
	mFocus->addDevice(addr);

	setState(ConferenceInterface::State::Created);
#endif // HAVE_ADVANCED_IM
}

void ClientConference::onConferenceKeywordsChanged(BCTBX_UNUSED(const vector<string> &keywords)) {
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		if (find(keywords.cbegin(), keywords.cend(), "one-to-one") != keywords.cend())
			chatRoom->addCapability(ChatRoom::Capabilities::OneToOne);
		if (find(keywords.cbegin(), keywords.cend(), "ephemeral") != keywords.cend())
			chatRoom->addCapability(ChatRoom::Capabilities::Ephemeral);
	}
#endif // HAVE_ADVANCED_IM
}

void ClientConference::onConferenceTerminated(BCTBX_UNUSED(const std::shared_ptr<Address> &addr)) {
	// Keep a reference to the conference to be able to set the state to Deleted
	shared_ptr<Conference> ref = getSharedFromThis();

#ifdef HAVE_ADVANCED_IM
	auto chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		if (mEventHandler) {
			mEventHandler->unsubscribe();
		}
		resetLastNotify();
		// remove event handler from list event handler if used
		if (getCore()->getPrivate()->clientListEventHandler) {
			getCore()->getPrivate()->clientListEventHandler->removeHandler(mEventHandler);
		}

		// No need to notify such event during the core startup.
		// This method might be called from MainDb::getChatRooms() when the core has corrupted DB and there are two
		// chatroom having the same ConferenceId except for the address'gr parameter
		if (linphone_core_get_global_state(getCore()->getCCore()) != LinphoneGlobalStartup) {
			const auto &conferenceId = getConferenceId();
			auto event =
			    make_shared<ConferenceEvent>(EventLog::Type::ConferenceTerminated, time(nullptr), conferenceId);
			chatRoom->addEvent(event);

			_linphone_chat_room_notify_conference_left(chatRoom->toC(), L_GET_C_BACK_PTR(event));
		}
	}
	mClientEktManager = nullptr;
#endif // HAVE_ADVANCED_IM

	auto session = getMainSession();
	std::shared_ptr<Call> call = nullptr;
	if (session) {
		SalCallOp *op = session->getPrivate()->getOp();
		call = op ? getCore()->getCallByCallId(op->getCallId()) : nullptr;
	}

	resetCachedScreenSharingDevice();
	for (const auto &device : getMe()->getDevices()) {
		device->enableScreenSharing(false);
	}
	if (call) call->setConference(nullptr);
	if (!mConfParams->chatEnabled()) {
		setState(ConferenceInterface::State::Deleted);
	}
}

void ClientConference::onSecurityEvent(BCTBX_UNUSED(const shared_ptr<ConferenceSecurityEvent> &event)) {
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		shared_ptr<ConferenceSecurityEvent> finalEvent = nullptr;
		shared_ptr<ConferenceSecurityEvent> cleanEvent = nullptr;

		// Remove faulty device if its address is invalid
		auto faultyDevice = event->getFaultyDeviceAddress();
		if (!faultyDevice || !faultyDevice->isValid()) {
			cleanEvent = make_shared<ConferenceSecurityEvent>(event->getCreationTime(), event->getConferenceId(),
			                                                  event->getSecurityEventType());
		}
		finalEvent = cleanEvent ? cleanEvent : event;
		chatRoom->addEvent(event);
		_linphone_chat_room_notify_security_event(chatRoom->toC(), L_GET_C_BACK_PTR(event));
	}
#endif // HAVE_ADVANCED_IM
}

AbstractChatRoom::SecurityLevel
ClientConference::getSecurityLevelExcept(const std::shared_ptr<ParticipantDevice> &ignoredDevice) const {
	auto encryptionEngine = getCore()->getEncryptionEngine();
	if (!encryptionEngine) {
		lWarning() << *this << ": Asking participant security level but there is no encryption engine enabled";
		return AbstractChatRoom::SecurityLevel::ClearText;
	}

	if (!getCurrentParams()->getChatParams()->isEncrypted()) {
		lDebug() << *this << ": Chatroom SecurityLevel = ClearText";
		return AbstractChatRoom::SecurityLevel::ClearText;
	}

	// Until participant list & self devices list is populated, don't assume chat room is safe but encrypted
	if (getParticipants().size() == 0 && getMe()->getDevices().size() == 0) {
		lDebug() << *this << ": Chatroom SecurityLevel = Encrypted";
		return AbstractChatRoom::SecurityLevel::Encrypted;
	}

	// populate a list of all devices in the chatroom
	// first step, all participants, including me
	auto participants = getParticipants();
	participants.push_back(getMe());

	std::list<std::string> allDevices{};
	for (const auto &participant : participants) {
		for (const auto &device : participant->getDevices()) {
			allDevices.push_back(device->getAddress()->asStringUriOnly());
		}
	}
	if (ignoredDevice != nullptr) {
		allDevices.remove(ignoredDevice->getAddress()->asStringUriOnly());
	}
	allDevices.remove(getConferenceId().getLocalAddress()->asStringUriOnly()); // remove local device from the list

	if (allDevices.empty()) {
		return AbstractChatRoom::SecurityLevel::Safe;
	}
	auto level = encryptionEngine->getSecurityLevel(allDevices);
	lDebug() << *this << ": Chatroom SecurityLevel = " << level;
	return level;
}

void ClientConference::onFirstNotifyReceived(BCTBX_UNUSED(const std::shared_ptr<Address> &addr)) {
#ifdef HAVE_ADVANCED_IM
	auto chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		// If the chatroom is not in tbhe created state, ignore the forst notify callback. Nonetheless, it has media
		// capabilities, this callback might be called when the conference is in the CreationPending state
		if (!((mState == ConferenceInterface::State::Created) ||
		      (supportsMedia() && (mState == ConferenceInterface::State::CreationPending)))) {
			lWarning() << "First notify received in " << *this << " that is not in the Created state ["
			           << Utils::toString(getState()) << "], ignoring it!";
			return;
		}

		const auto &conferenceId = getConferenceId();
		auto event = make_shared<ConferenceEvent>(EventLog::Type::ConferenceCreated, time(nullptr), conferenceId);

		bool_t forceFullState = linphone_config_get_bool(linphone_core_get_config(getCore()->getCCore()), "misc",
		                                                 "conference_event_package_force_full_state", FALSE);
		if (!forceFullState) // to avoid this event to be repeated for each full state
			chatRoom->addEvent(event);

		_linphone_chat_room_notify_conference_joined(chatRoom->toC(), L_GET_C_BACK_PTR(event));
		dynamic_pointer_cast<ClientChatRoom>(chatRoom)->stopBgTask();
	}
#endif // HAVE_ADVANCED_IM
}

void ClientConference::onParticipantDeviceScreenSharingChanged(
    BCTBX_UNUSED(const std::shared_ptr<ConferenceParticipantDeviceEvent> &event),
    const std::shared_ptr<ParticipantDevice> &device) {
	auto session = getMainSession();
	bool isMainSession = session && (device->getSession() == session);
	if (isMainSession && isIn()) {
		if (device->screenSharingEnabled()) {
			notifyActiveSpeakerParticipantDevice(device);
		} else {
			if (mDisplayedSpeaker != 0) {
				auto displayedDevice = findParticipantDeviceBySsrc(mDisplayedSpeaker, LinphoneStreamTypeVideo);
				if (displayedDevice) {
					// Reset the active speaker participant to the one was shown before screen sharing was enabled
					notifyActiveSpeakerParticipantDevice(displayedDevice);
				}
			}
		}
	}
	const auto ms = static_pointer_cast<MediaSession>(session);
	ConferenceLayout confLayout = ms->getMediaParams()->getConferenceVideoLayout();
	bool isGridLayout = (confLayout == ConferenceLayout::Grid);
	if (!isMainSession && (updateMinatureRequestedFlag() || isGridLayout)) {
		auto updateSession = [this, device]() -> LinphoneStatus {
			lInfo() << "Sending re-INVITE because participant thumbnails are "
			        << (areThumbnailsRequested(false) ? "" : "no longer") << " requested in " << *this
			        << " or the conference layout is Grid";
			auto ret = updateMainSession();
			if (ret != 0) {
				lInfo() << "Delaying re-INVITE because participant thumbnails are "
				        << (areThumbnailsRequested(false) ? "" : "no longer") << " requested in " << *this
				        << " or the conference layout is Grid cannot be sent right now";
			}
			return ret;
		};

		if (updateSession() != 0) {
			mScheduleUpdate = true;
		}
	}
}

void ClientConference::onFullStateReceived() {
	updateMinatureRequestedFlag();

	// When receiving a full state, we must recreate the conference informations in order to get the security level
	// and the participant list up to date
	updateAndSaveConferenceInformations();

	if (supportsMedia()) {
#ifdef HAVE_ADVANCED_IM
		if (mConfParams->getSecurityLevel() == ConferenceParamsInterface::SecurityLevel::EndToEnd) {
			if (mClientEktManager == nullptr) {
				shared_ptr<ClientConference> clientConference =
				    dynamic_pointer_cast<ClientConference>(getSharedFromThis());
				mClientEktManager = make_shared<ClientEktManager>(MSEKTCipherType::MS_EKT_CIPHERTYPE_AESKW256,
				                                                  MSCryptoSuite::MS_AEAD_AES_256_GCM);
				mClientEktManager->init(clientConference);
			}
			mClientEktManager->subscribe();
		}

		if (mConfParams->chatEnabled() && !getChatRoom()) {
			setChatRoom((new ClientChatRoom(getCore(), getSharedFromThis()))->toSharedPtr());
		}
#endif // HAVE_ADVANCED_IM

		auto session = mFocus ? dynamic_pointer_cast<MediaSession>(mFocus->getSession()) : nullptr;
		// Notify local participant that the microphone is muted when receiving the full state as participants are added
		// as soon as possible
		if (session) {
			notifyLocalMutedDevices(session->getPrivate()->getMicrophoneMuted());
		}

		if (!getCore()->getCCore()->sal->mediaDisabled()) {
			if (session && (!session->mediaInProgress() || !session->getPrivate()->isUpdateSentWhenIceCompleted())) {

				auto requestStreams = [this]() -> LinphoneStatus {
					lInfo() << "Sending re-INVITE in order to get streams after receiving a NOTIFY full state for "
					        << *this;
					setState(ConferenceInterface::State::Created);
					auto ret = updateMainSession(false);
					return ret;
				};

				if (requestStreams() != 0) {
					lInfo() << "Delaying re-INVITE in order to get streams after receiving a NOTIFY full state for "
					        << *this << " because it cannot be sent right now";
					session->addPendingAction(requestStreams);
				}
				mScheduleUpdate = false;
				mFullStateUpdate = false;
			} else {
				lInfo() << "Delaying re-INVITE in order to get streams after receiving a NOTIFY full state for "
				        << *this << " because ICE negotiations didn't end yet";
				mScheduleUpdate = true;
				mFullStateUpdate = true;
			}
		} else {
			// Set the state if no media so that the conference is correctly handled
			setState(ConferenceInterface::State::Created);
		}

	} else {
		if ((mState == ConferenceInterface::State::Instantiated) ||
		    (mState == ConferenceInterface::State::CreationPending)) {
			setState(ConferenceInterface::State::Created);
		}
	}

#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		getCore()->getPrivate()->insertChatRoomWithDb(chatRoom, getLastNotify());
	}
#endif // HAVE_ADVANCED_IM

	mFullStateReceived = true;
}

void ClientConference::onEphemeralModeChanged(BCTBX_UNUSED(const shared_ptr<ConferenceEphemeralMessageEvent> &event)) {
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		if (event->getFullState()) return;
		chatRoom->addEvent(event);
		_linphone_chat_room_notify_ephemeral_event(chatRoom->toC(), L_GET_C_BACK_PTR(event));
	}
#endif // HAVE_ADVANCED_IM
}

void ClientConference::onEphemeralMessageEnabled(
    BCTBX_UNUSED(const shared_ptr<ConferenceEphemeralMessageEvent> &event)) {
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		if (event->getFullState()) return;
		chatRoom->addEvent(event);
		_linphone_chat_room_notify_ephemeral_event(chatRoom->toC(), L_GET_C_BACK_PTR(event));
	}
#endif // HAVE_ADVANCED_IM
}

void ClientConference::onEphemeralLifetimeChanged(
    BCTBX_UNUSED(const shared_ptr<ConferenceEphemeralMessageEvent> &event)) {
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (mConfParams->chatEnabled() && chatRoom) {
		if (event->getFullState()) return;
		chatRoom->addEvent(event);
		_linphone_chat_room_notify_ephemeral_event(chatRoom->toC(), L_GET_C_BACK_PTR(event));
	}
#endif // HAVE_ADVANCED_IM
}

void ClientConference::notifyStateChanged(ConferenceInterface::State state) {
	// Call callbacks before calling listeners because listeners may change state
	linphone_core_notify_conference_state_changed(getCore()->getCCore(), toC(), (LinphoneConferenceState)mState);

	Conference::notifyStateChanged(state);
}

void ClientConference::notifyDisplayedSpeaker(uint32_t csrc) {
	if (mConfParams->videoEnabled()) {
		if (mLastNotifiedSsrc == csrc) return;

		mDisplayedSpeaker = csrc;

		if (csrc != 0) {
			auto device = findParticipantDeviceBySsrc(csrc, LinphoneStreamTypeVideo);

			if (device) {
				notifyActiveSpeakerParticipantDevice(device);
				mLastNotifiedSsrc = csrc;
			} else {
				auto meDevice = getMe()->findDeviceBySsrc(csrc, LinphoneStreamTypeVideo);
				if (meDevice) {
					lInfo() << "Received notification that the me participant " << *getMe()->getAddress()
					        << " is the active speaker of " << *this;
				} else {
					lError() << *this << ": Active speaker changed with csrc: " << csrc
					         << " but it does not correspond to any participant device";
				}
			}
		} else {
			if (mLouderSpeaker != mLastNotifiedSsrc) notifyLouderSpeaker(mLouderSpeaker);
		}
	}
}

void ClientConference::notifyLouderSpeaker(uint32_t ssrc) {
	// Ignore louder speaker notification as long as a participant device is screen sharing as the stream in the active
	// speaker window is the screen shared
	if (getScreenSharingDevice()) return;
	mLouderSpeaker = ssrc;

	auto device = findParticipantDeviceBySsrc(ssrc, LinphoneStreamTypeAudio);
	if (device == nullptr) {
		auto meDevice = getMe()->findDeviceBySsrc(ssrc, LinphoneStreamTypeAudio);
		if (meDevice) {
			lInfo() << "Received notification that the me participant " << *getMe()->getAddress()
			        << " is the louder speaker of " << *this;
		} else {
			lError() << *this << ": Active speaker changed with ssrc: " << ssrc
			         << " but it does not correspond to any participant device";
		}
		return;
	}

	if (!device->getStreamAvailability(LinphoneStreamTypeVideo)) {
		notifyActiveSpeakerParticipantDevice(device);
		mLastNotifiedSsrc = ssrc;
	} else {
		// If we are notified of an active speaker that has video but is already the one displayed then we have to
		// notify him again since notifyDisplayedSpeaker won't be called again.
		if (mDisplayedSpeaker == device->getSsrc(LinphoneStreamTypeVideo)) {
			notifyActiveSpeakerParticipantDevice(device);
			mLastNotifiedSsrc = ssrc;
		}
	}
}

bool ClientConference::isSubscriptionUnderWay() const {
	bool underWay = false;
#ifdef HAVE_ADVANCED_IM
	if (getCore()->getPrivate()->clientListEventHandler->findHandler(getConferenceId())) {
		underWay =
		    getCore()->getPrivate()->clientListEventHandler->getInitialSubscriptionUnderWayFlag(getConferenceId());
	} else {
		underWay = mEventHandler ? mEventHandler->getInitialSubscriptionUnderWayFlag() : false;
	}
#endif // HAVE_ADVANCED_IM

	return underWay;
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void ClientConference::multipartNotifyReceived(const std::shared_ptr<Event> &notifyLev, const Content &content) {
#ifdef HAVE_ADVANCED_IM
	if (mEventHandler) {
		const auto initialSubscription = mEventHandler->getInitialSubscriptionUnderWayFlag();
		mEventHandler->multipartNotifyReceived(notifyLev, content);
		const auto &chatRoom = getChatRoom();
		if (mConfParams->chatEnabled() && chatRoom && initialSubscription &&
		    !mEventHandler->getInitialSubscriptionUnderWayFlag()) {
			chatRoom->sendPendingMessages();
		}
		return;
	}
#endif // HAVE_ADVANCED_IM
	lInfo()
	    << *this
	    << ": Unable to handle multi part NOTIFY because conference event package (RFC 4575) is disabled or the SDK "
	       "was not compiled with ENABLE_ADVANCED_IM flag set to on";
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void ClientConference::notifyReceived(const std::shared_ptr<Event> &notifyLev, const Content &content) {
#ifdef HAVE_ADVANCED_IM
	if (notifyLev->getName() == "ekt") {
		if (mClientEktManager) {
			mClientEktManager->notifyReceived(content);
			return;
		}
	} else {
		if (mEventHandler) {
			const auto initialSubscription = mEventHandler->getInitialSubscriptionUnderWayFlag();
			mEventHandler->notifyReceived(notifyLev, content);
			const auto &chatRoom = getChatRoom();
			if (mConfParams->chatEnabled() && chatRoom && initialSubscription &&
			    !mEventHandler->getInitialSubscriptionUnderWayFlag()) {
				chatRoom->sendPendingMessages();
			}
			return;
		}
	}
#endif // HAVE_ADVANCED_IM
	lInfo() << *this
	        << ": Unable to handle NOTIFY because conference event package (RFC 4575) is disabled or the SDK was not "
	           "compiled with ENABLE_ADVANCED_IM flag set to on";
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

int ClientConference::inviteAddresses(const std::list<std::shared_ptr<Address>> &addresses,
                                      const LinphoneCallParams *params) {
	const auto &account = mConfParams->getAccount();
	const auto organizer = account ? account->getAccountParams()->getIdentityAddress()
	                               : Address::create(linphone_core_get_identity(getCore()->getCCore()));

	// This method only creates ad-hoc conferences
	mConfParams->setStartTime(-1);
	mConfParams->setEndTime(-1);

	auto ref = getSharedFromThis();
	getCore()->addConferencePendingCreation(ref);
	std::list<Address> invitees;
	for (const auto &address : addresses) {
		auto participant = Participant::create(Address::create(address->getUri()));
		participant->setRole(Participant::Role::Speaker);
		mInvitedParticipants.push_back(participant);
		invitees.push_back(Conference::createParticipantAddressForResourceList(participant));
	}

	// The main session for the time being is the one used to create the conference. It will later replaced by the
	// actual session used to join the conference
	auto session = getCore()->createOrUpdateConferenceOnServer(mConfParams, invitees, getConferenceAddress(), ref);
	if (!session) {
		lInfo() << "Aborting creation of conference " << *this << " because the call session cannot be established";
		setState(ConferenceInterface::State::CreationFailed);
		return -1;
	}
	setMainSession(session);
	if (params) {
		mJoiningParams = L_GET_CPP_PTR_FROM_C_OBJECT(params)->clone();
	}
	return 0;
}

bool ClientConference::dialOutAddresses(BCTBX_UNUSED(const std::list<std::shared_ptr<Address>> &addressList)) {
	lError() << *this << ": ClientConference::dialOutAddresses() not implemented";
	return false;
}

bool ClientConference::finalizeParticipantAddition(BCTBX_UNUSED(std::shared_ptr<Call> call)) {
	lError() << *this << ": ClientConference::finalizeParticipantAddition() not implemented";
	return false;
}

int ClientConference::removeParticipant(const std::shared_ptr<Address> &addr) {
	if (getMe()->isAdmin()) {
		std::shared_ptr<Participant> p = findParticipant(addr);
		if (p) {
			switch (mState) {
				case ConferenceInterface::State::Created:
				case ConferenceInterface::State::TerminationPending: {
					if (!findParticipant(addr)) {
						lError() << *this << ": could not remove participant \'" << *addr
						         << "\': not in the participants list";
						return -1;
					}
					LinphoneCore *cCore = getCore()->getCCore();
					SalReferOp *referOp = new SalReferOp(cCore->sal.get());
					LinphoneAddress *lAddr = getConferenceAddress()->toC();
					linphone_configure_op(cCore, referOp, lAddr, nullptr, false);
					std::shared_ptr<Address> referToAddr = addr;
					referToAddr->setMethodParam("BYE");
					auto res = referOp->sendRefer(referToAddr->getImpl());
					referOp->unref();

					if (res != 0) {
						lError() << *this << ": could not remove participant \'" << *addr
						         << "\': REFER with BYE has failed";
						return -1;
					}
				} break;
				default:
					lError() << "Could not remove participant " << *addr << " from " << *this
					         << ". Bad conference state (" << Utils::toString(mState) << ")";
					return -1;
			}
			return 0;
		} else {
			lWarning() << "Unable to remove participant " << *addr << " because it is not part of " << *this;
		}
	} else {
		lWarning() << "Unable to remove participant " << *addr << " from " << *this << " because local participant "
		           << *getMe()->getAddress() << " is not admin.";
	}
	return -1;
}

int ClientConference::removeParticipant(const std::shared_ptr<CallSession> &session,
                                        BCTBX_UNUSED(const bool preserveSession)) {
	const std::shared_ptr<Address> &remoteAddress = session->getRemoteAddress();
	std::shared_ptr<Participant> p = findParticipant(remoteAddress);
	if (getMe()->isAdmin()) {
		if (p) {
			return removeParticipant(p) ? 0 : -1;
		}
	} else {
		lError() << *this << ": Unable to remove participant " << *(p->getAddress()) << " because focus "
		         << *(getMe()->getAddress()) << " is not admin";
	}
	return -1;
}

bool ClientConference::removeParticipant(const std::shared_ptr<Participant> &participant) {
	const auto &participantAddress = participant->getAddress();
	if (getMe()->isAdmin()) {
		if (supportsMedia()) {
			return (removeParticipant(participantAddress) == 0) ? true : false;
		} else if (mConfParams->chatEnabled()) {
			LinphoneCore *cCore = getCore()->getCCore();
			// TODO handle one-to-one case ?
			SalReferOp *referOp = new SalReferOp(cCore->sal.get());
			LinphoneAddress *lAddr = getConferenceAddress()->toC();
			linphone_configure_op(cCore, referOp, lAddr, nullptr, false);
			Address referToAddr(*participantAddress);
			referToAddr.setParam(Conference::TextParameter);
			referToAddr.setUriParam("method", "BYE");
			referOp->sendRefer(referToAddr.getImpl());
			referOp->unref();
			return true;
		}
	} else {
		lError() << *this << ": Unable to remove participant " << *participantAddress << " because focus "
		         << *(getMe()->getAddress()) << " is not admin";
	}
	return false;
}

int ClientConference::participantDeviceMediaCapabilityChanged(
    BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
	lError() << *this << ": ClientConference::participantDeviceMediaCapabilityChanged() not implemented";
	return -1;
}
int ClientConference::participantDeviceMediaCapabilityChanged(BCTBX_UNUSED(const std::shared_ptr<Address> &addr)) {
	lError() << *this << ": ClientConference::participantDeviceMediaCapabilityChanged() not implemented";
	return -1;
}
int ClientConference::participantDeviceMediaCapabilityChanged(
    BCTBX_UNUSED(const std::shared_ptr<Participant> &participant),
    BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device)) {
	lError() << *this << ": ClientConference::participantDeviceMediaCapabilityChanged() not implemented";
	return -1;
}
int ClientConference::participantDeviceSsrcChanged(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
                                                   BCTBX_UNUSED(const LinphoneStreamType type),
                                                   BCTBX_UNUSED(uint32_t ssrc)) {
	lError() << *this << ": ClientConference::participantDeviceSsrcChanged() not implemented";
	return -1;
}
int ClientConference::participantDeviceSsrcChanged(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session),
                                                   BCTBX_UNUSED(uint32_t audioSsrc),
                                                   BCTBX_UNUSED(uint32_t videoSsrc)) {
	lError() << *this << ": ClientConference::participantDeviceSsrcChanged() not implemented";
	return -1;
}

int ClientConference::participantDeviceAlerting(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
	lError() << *this << ": ClientConference::participantDeviceAlerting() not implemented";
	return -1;
}
int ClientConference::participantDeviceAlerting(BCTBX_UNUSED(const std::shared_ptr<Participant> &participant),
                                                BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device)) {
	lError() << *this << ": ClientConference::participantDeviceAlerting() not implemented";
	return -1;
}
int ClientConference::participantDeviceJoined(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
	lError() << *this << ": ClientConference::participantDeviceJoined() not implemented";
	return -1;
}
int ClientConference::participantDeviceJoined(BCTBX_UNUSED(const std::shared_ptr<Participant> &participant),
                                              BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device)) {
	lError() << *this << ": ClientConference::participantDeviceJoined() not implemented";
	return -1;
}
int ClientConference::participantDeviceLeft(BCTBX_UNUSED(const std::shared_ptr<CallSession> &session)) {
	lError() << *this << ": ClientConference::participantDeviceLeft() not implemented";
	return -1;
}
int ClientConference::participantDeviceLeft(BCTBX_UNUSED(const std::shared_ptr<Participant> &participant),
                                            BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device)) {
	lError() << *this << ": ClientConference::participantDeviceLeft() not implemented";
	return -1;
}

void ClientConference::setLocalParticipantStreamCapability(const LinphoneMediaDirection &direction,
                                                           const LinphoneStreamType type) {
	auto session = dynamic_pointer_cast<MediaSession>(getMainSession());
	if (session) {
		const MediaSessionParams *params = session->getMediaParams();
		MediaSessionParams *currentParams = params->clone();
		if (!currentParams->rtpBundleEnabled()) {
			currentParams->enableRtpBundle(true);
		}
		lInfo() << "Setting direction of stream of type " << std::string(linphone_stream_type_to_string(type)) << " to "
		        << std::string(linphone_media_direction_to_string(direction)) << " of main session " << session;
		switch (type) {
			case LinphoneStreamTypeAudio:
				currentParams->enableAudio((direction != LinphoneMediaDirectionInactive) &&
				                           (direction != LinphoneMediaDirectionInvalid));
				currentParams->setAudioDirection(direction);
				break;
			case LinphoneStreamTypeVideo:
				currentParams->enableVideo((direction != LinphoneMediaDirectionInactive) &&
				                           (direction != LinphoneMediaDirectionInvalid));
				currentParams->setVideoDirection(direction);
				break;
			case LinphoneStreamTypeText:
				currentParams->enableRealtimeText((direction != LinphoneMediaDirectionInactive) &&
				                                  (direction != LinphoneMediaDirectionInvalid));
				break;
			case LinphoneStreamTypeUnknown:
				lError() << "Unable to set direction of stream of type "
				         << std::string(linphone_stream_type_to_string(type));
				return;
		}
		session->update(currentParams);
		delete currentParams;
	}
}

int ClientConference::getParticipantDeviceVolume(const std::shared_ptr<ParticipantDevice> &device) {
	AudioStream *as = getAudioStream();

	if (as != nullptr) {
		return audio_stream_get_participant_volume(as, device->getSsrc(LinphoneStreamTypeAudio));
	}

	return AUDIOSTREAMVOLUMES_NOT_FOUND;
}

int ClientConference::terminate() {
	if (supportsMedia()) {
		auto savedState = mState;
		auto session = getMainSession();
		shared_ptr<Call> sessionCall = nullptr;
		if (session) {
			auto op = session->getPrivate()->getOp();
			sessionCall = op ? getCore()->getCallByCallId(op->getCallId()) : nullptr;
		}

		switch (savedState) {
			case ConferenceInterface::State::Created:
			case ConferenceInterface::State::CreationPending:
			case ConferenceInterface::State::CreationFailed:
				if (sessionCall) {
					// Conference will be deleted by terminating the session
					session->terminate();
					return 0;
				}
				break;
			default:
				break;
		}

		if (mState == ConferenceInterface::State::Terminated) {
			setState(ConferenceInterface::State::Deleted);
		} else if (mState != ConferenceInterface::State::Deleted) {
			setState(ConferenceInterface::State::TerminationPending);
			if (!sessionCall) {
				setState(ConferenceInterface::State::Terminated);
				setState(ConferenceInterface::State::Deleted);
			}
		}
	}

	if (mConfParams->chatEnabled()) {
		setChatRoom(nullptr);
	}

	return 0;
}

void ClientConference::finalizeCreation() {
	if (mFinalized) {
		lDebug() << "Conference " << this << " has already been finalized";
		return;
	} else {
		mFinalized = true;
		initializeHandlers(this, false);
	}
}

int ClientConference::enter() {
	const auto &conferenceAddress = getConferenceAddress();
	if (!conferenceAddress || !conferenceAddress->isValid()) {
		lError() << "Could not enter in the conference because its conference address (" << conferenceAddress
		         << ") is not valid";
		return -1;
	}

	auto session = dynamic_pointer_cast<MediaSession>(getMainSession());
	if (session) {
		CallSession::State callState = session->getState();
		switch (callState) {
			case CallSession::State::StreamsRunning:
				break;
			case CallSession::State::Paused:
				session->resume();
				participantDeviceJoined(mMe, mMe->getDevices().front());
				break;
			default:
				lError() << "Could not join the conference: bad focus call state (" << Utils::toString(callState)
				         << ")";
				return -1;
		}
	} else {
		/* Start a new call by indicating that it has to be put into the conference directly */
		LinphoneCallParams *new_params = linphone_core_create_call_params(getCore()->getCCore(), nullptr);
		linphone_call_params_enable_video(new_params, mConfParams->videoEnabled());
		linphone_call_params_set_in_conference(new_params, FALSE);
		if (!!linphone_core_get_add_admin_information_to_contact(getCore()->getCCore())) {
			L_GET_CPP_PTR_FROM_C_OBJECT(new_params)
			    ->addCustomContactParameter(Conference::AdminParameter, Utils::toString(getMe()->isAdmin()));
		}

		const std::shared_ptr<Address> &address = getConferenceAddress();
		const string &confId = address->getUriParamValue(Conference::ConfIdParameter);
		linphone_call_params_set_conference_id(new_params, confId.c_str());

		std::string subject = getMe()->isAdmin() ? getUtf8Subject() : std::string();

		auto cCall = linphone_core_invite_address_with_params_2(getCore()->getCCore(), address->toC(), new_params,
		                                                        L_STRING_TO_C(subject), nullptr);

		linphone_call_params_unref(new_params);

		auto cppCall = Call::toCpp(cCall);
		cppCall->setConference(getSharedFromThis());
		auto callSession = cppCall->getActiveSession();
		callSession->addListener(getSharedFromThis());
		mFocus->setSession(callSession);
	}
	return 0;
}

void ClientConference::join(const std::shared_ptr<Address> &) {
#ifdef HAVE_ADVANCED_IM
	if (mConfParams->chatEnabled()) {
		shared_ptr<CallSession> session = mFocus->getSession();
		if (!session && ((mState == ConferenceInterface::State::Instantiated) ||
		                 (mState == ConferenceInterface::State::Terminated))) {
			session = createSession();
		}
		if (session) {
			if (mState != ConferenceInterface::State::TerminationPending) session->startInvite(nullptr, "", nullptr);
			const auto &chatRoom = getChatRoom();
			if (chatRoom && (mState != ConferenceInterface::State::Created))
				setState(ConferenceInterface::State::CreationPending);
		}
	}
#endif // HAVE_ADVANCED_IM
}

void ClientConference::leave() {
	if (supportsMedia()) {
		if (mState != ConferenceInterface::State::Created) {
			lError() << "Could not leave the conference: bad conference state (" << Utils::toString(mState) << ")";
		}

		const auto &meAddress = getMe()->getAddress();
		auto session = dynamic_pointer_cast<MediaSession>(getMainSession());
		CallSession::State callState = session->getState();
		switch (callState) {
			case CallSession::State::Paused:
				lInfo() << *meAddress << " is leaving " << *this << " while focus call is paused.";
				break;
			case CallSession::State::StreamsRunning:
				lInfo() << *meAddress << " is leaving " << *this << ". focus call is going to be paused.";
				session->pause();
				participantDeviceLeft(mMe, mMe->getDevices().front());
				break;
			default:
				lError() << *meAddress << " cannot leave " << *this << " because focus call is in state "
				         << Utils::toString(callState);
		}
#ifdef HAVE_ADVANCED_IM
	} else if (mConfParams->chatEnabled()) {
		if (mEventHandler) {
			mEventHandler->unsubscribe();
		}
		shared_ptr<CallSession> session = mFocus->getSession();
		if (session) {
			session->terminate();
		} else if (mState != ConferenceInterface::State::CreationFailed) {
			// No need to create a session if the creation already failed
			session = createSession();
			session->startInvite(nullptr, "", nullptr);
		}
		setState(ConferenceInterface::State::TerminationPending);
		if (!session) {
			setState(ConferenceInterface::State::Terminated);
			setState(ConferenceInterface::State::Deleted);
		}
#endif // HAVE_ADVANCED_IM
	}
}

const std::shared_ptr<Address> ClientConference::getOrganizer() const {
	if (mOrganizer) {
		return mOrganizer;
	}

	std::shared_ptr<Address> organizer = nullptr;
#ifdef HAVE_DB_STORAGE
	auto &mainDb = getCore()->getPrivate()->mainDb;
	if (mainDb) {
		const auto &confInfo = mainDb->getConferenceInfoFromURI(getConferenceAddress());
		// me is admin if the organizer is the same as me
		if (confInfo) {
			organizer = confInfo->getOrganizerAddress();
		}
	}
#endif

	if (!organizer) {
		// The me participant is designed as organizer as a last resort in our guesses, therefore it may not be right.
		// A search for a participant which joined the conference as focus owner is therefore needed.
		const auto focusOwnerDevice = getFocusOwnerDevice();
		if (focusOwnerDevice) {
			organizer = focusOwnerDevice->getParticipant()->getAddress();
		}
	}

	if (!organizer) {
		auto session = static_pointer_cast<MediaSession>(getMainSession());
		const auto referer = (session ? L_GET_PRIVATE(session->getMediaParams())->getReferer() : nullptr);
		if (referer) {
			organizer = referer->getRemoteAddress();
		}
	}

	if (!organizer) {
		// Guess the organizer and as last resort set it to ourselves
		organizer = getMe()->getAddress();
	}
	setOrganizer(organizer);

	return mOrganizer;
}

AudioControlInterface *ClientConference::getAudioControlInterface() const {
	auto session = getMainSession();
	if (!session) return nullptr;
	auto ms = static_pointer_cast<MediaSession>(session);
	return ms->getStreamsGroup().lookupMainStreamInterface<AudioControlInterface>(SalAudio);
}

VideoControlInterface *ClientConference::getVideoControlInterface() const {
	auto session = getMainSession();
	if (!session) return nullptr;
	auto ms = static_pointer_cast<MediaSession>(session);
	return ms->getStreamsGroup().lookupMainStreamInterface<VideoControlInterface>(SalVideo);
}

AudioStream *ClientConference::getAudioStream() {
	auto session = getMainSession();
	if (!session) return nullptr;
	auto ms = static_pointer_cast<MediaSession>(session);
	MS2AudioStream *stream = ms->getStreamsGroup().lookupMainStreamInterface<MS2AudioStream>(SalAudio);
	return stream ? (AudioStream *)stream->getMediaStream() : nullptr;
}

bool ClientConference::hasBeenLeft() const {
	return (mState == State::TerminationPending) || (mState == State::Terminated) || (mState == State::Deleted);
}

void ClientConference::handleRefer(SalReferOp *op,
                                   const std::shared_ptr<LinphonePrivate::Address> &referAddr,
                                   const std::string method) {
	if (method == "BYE") {
		lInfo() << "The server requested " << *referAddr << " to leave the conference";
		// The server asks a participant to leave a chat room
		leave();
		op->reply(SalReasonNone);
	}
}

bool ClientConference::sessionParamsAllowThumbnails() const {
	auto session = dynamic_pointer_cast<MediaSession>(getMainSession());
	return session ? session->getMediaParams()->rtpBundleEnabled() : false;
}

std::pair<bool, LinphoneMediaDirection> ClientConference::getMainStreamVideoDirection(
    const std::shared_ptr<CallSession> &session, BCTBX_UNUSED(bool localIsOfferer), bool useLocalParams) const {
	const auto ms = dynamic_pointer_cast<MediaSession>(session);
	if (!ms) {
		return std::make_pair(false, LinphoneMediaDirectionInactive);
	}
	auto participantDevice = getMe()->findDevice(session);
	bool enableVideoStream =
	    (useLocalParams) ? ms->getMediaParams()->videoEnabled() : ms->getCurrentParams()->videoEnabled();
	const auto videoDirInParams = ms->getMediaParams()->getVideoDirection();
	const auto sendingVideo =
	    ((videoDirInParams == LinphoneMediaDirectionSendRecv) || (videoDirInParams == LinphoneMediaDirectionSendOnly));
	bool isScreenSharing = ms->getMediaParams()->screenSharingEnabled();
	ConferenceLayout confLayout = ms->getMediaParams()->getConferenceVideoLayout();
	const auto &cameraEnabled = ms->getMediaParams()->cameraEnabled();
	LinphoneMediaDirection videoDir = LinphoneMediaDirectionInactive;
	if (isScreenSharing) {
		// Set the video direction to inactive if the participant has enabled screen sharing but the video
		// direction in the call parameters has no send component
		videoDir = ((sendingVideo) ? LinphoneMediaDirectionSendOnly : LinphoneMediaDirectionInactive);
	} else {
		switch (confLayout) {
			case ConferenceLayout::ActiveSpeaker:
				if (cameraEnabled) {
					videoDir = videoDirInParams;
				} else {
					videoDir = LinphoneMediaDirectionRecvOnly;
				}
				break;
			case ConferenceLayout::Grid:
				if (sendingVideo) {
					if (cameraEnabled) {
						videoDir = LinphoneMediaDirectionSendOnly;
					} else {
						videoDir = LinphoneMediaDirectionInactive;
					}
				} else {
					videoDir = LinphoneMediaDirectionInactive;
				}
				break;
		}
	}
	return std::make_pair(enableVideoStream, videoDir);
}

void ClientConference::onCallSessionSetTerminated(const shared_ptr<CallSession> &session) {
	bool isLocalExhume = false;
#ifdef HAVE_ADVANCED_IM
	const auto &chatRoom = getChatRoom();
	if (chatRoom) {
		auto clientGroupChatRoom = dynamic_pointer_cast<ClientChatRoom>(chatRoom);
		isLocalExhume = clientGroupChatRoom && clientGroupChatRoom->isLocalExhumePending();
	}
#endif // HAVE_ADVANCED_IM

	if (!getConferenceAddress() || isLocalExhume) {
		const std::shared_ptr<Address> remoteAddress = session->getRemoteContactAddress();
		auto ref = getSharedFromThis();
		getCore()->removeConferencePendingCreation(ref);
		if (remoteAddress == nullptr) {
			lError() << *this
			         << " The session to update the conference information did not successfully establish hence it is "
			            "likely that the request wasn't taken into account by the server";
			setState(ConferenceInterface::State::CreationFailed);
		} else if (dynamic_pointer_cast<MediaSession>(session) &&
		           ((mState == ConferenceInterface::State::Instantiated) ||
		            (mState == ConferenceInterface::State::CreationPending)) &&
		           (session->getParams()->getPrivate()->getStartTime() < 0)) {
			auto conferenceAddress = remoteAddress;

			lInfo() << *this << " has been successfully created: " << *conferenceAddress;
			const auto &meAddress = getMe()->getAddress();
			ConferenceId conferenceId(conferenceAddress, meAddress, getCore()->createConferenceIdParams());
			// Do not change the conference ID yet if exhuming a chatroom
			if (!isLocalExhume) {
				setConferenceId(conferenceId);
			}
			setConferenceAddress(conferenceAddress);

			// The conference pointer must be set here because from now we are sure that the shared pointer ref count
			// will be always greater than 1 (the core holds it)
			getMe()->setConference(getSharedFromThis());

			lInfo() << "Automatically rejoining " << *this;
			MediaSessionParams *dialoutParams = nullptr;
			if (mJoiningParams) {
				dialoutParams = mJoiningParams->clone();
			} else {
				// No parameters were given to ClientConference::inviteAddresses()
				dialoutParams = new MediaSessionParams();
				dialoutParams->initDefault(getCore(), LinphoneCallOutgoing);
				// Copy conference capabilities as the application didn't specify any parameter to pass on to the call
				dialoutParams->enableAudio(mConfParams->audioEnabled());
				dialoutParams->enableVideo(mConfParams->videoEnabled());
				dialoutParams->enableRealtimeText(false);
			}
			if (!!linphone_core_get_add_admin_information_to_contact(getCore()->getCCore())) {
				// Participant with the focus call is admin
				dialoutParams->addCustomContactParameter(Conference::AdminParameter, Utils::toString(true));
			}
			if (mConfParams->chatEnabled()) {
				dialoutParams->addCustomHeader("Require", "recipient-list-invite");
			}

			std::list<Address> addressesList;
			for (const auto &participant : mInvitedParticipants) {
				const auto &address = participant->getAddress();
				// Do not add the me address to the list of participants to put in the INVITE
				if (!isMe(address)) {
					addressesList.push_back(Conference::createParticipantAddressForResourceList(participant));
				}
			}
			addressesList.sort([](const auto &addr1, const auto &addr2) { return addr1 < addr2; });
			addressesList.unique([](const auto &addr1, const auto &addr2) { return addr1.weakEqual(addr2); });

			bool mediaSupported = supportsMedia();
			std::shared_ptr<Content> content = nullptr;
			if (!addressesList.empty()) {
				auto resourceList = Content::create();
				resourceList->setBodyFromUtf8(Utils::getResourceLists(addressesList));
				resourceList->setContentType(ContentType::ResourceLists);
				resourceList->setContentDisposition(ContentDisposition::RecipientList);
				if (linphone_core_content_encoding_supported(getCore()->getCCore(), "deflate")) {
					resourceList->setContentEncoding("deflate");
				}

				// Adding a custom content to the call params allows to create a multipart message body.
				// For audio video conferences it is requires as the client has to initiate a media session. The chat
				// room code, instead, makes the assumption that the participant list is the body itself
				if (mediaSupported) {
					dialoutParams->addCustomContent(resourceList);
				} else {
					content = resourceList;
				}
			}

			if (!mediaSupported) {
				dialoutParams->addCustomContactParameter(Conference::TextParameter);
				if (!mConfParams->isGroup()) {
					dialoutParams->addCustomHeader("One-To-One-Chat-Room", "true");
				}
				if (mConfParams->getChatParams()->isEncrypted()) {
					dialoutParams->addCustomHeader("End-To-End-Encrypted", "true");
				}
				if (mConfParams->getChatParams()->getEphemeralMode() == AbstractChatRoom::EphemeralMode::AdminManaged) {
					dialoutParams->addCustomHeader("Ephemerable", "true");
					dialoutParams->addCustomHeader("Ephemeral-Life-Time",
					                               to_string(mConfParams->getChatParams()->getEphemeralLifetime()));
				}
			}
			dialoutParams->getPrivate()->disableRinging(!mediaSupported);
			dialoutParams->getPrivate()->enableToneIndications(mediaSupported);

			// If one of the pending calls has the video enabled, then force the activation of the video. Otherwise it
			// would be weird to be in a video call and once it is merged the participant has no video anymore
			bool forceVideoEnabled = false;
			for (const auto &call : mPendingCalls) {
				forceVideoEnabled |= call->getParams()->videoEnabled();
			}
			if (forceVideoEnabled) {
				lInfo() << "Forcing " << *mMe->getAddress() << " to enable video capabilities when joining " << *this
				        << " because at least one call that was merged had video capabilities on";
				dialoutParams->enableVideo(true);
			}

			const auto cCore = getCore()->getCCore();
			const auto &subject = mConfParams->getUtf8Subject();
			if (mediaSupported) {
				LinphoneCall *call = linphone_core_invite_address_with_params_2(
				    cCore, remoteAddress->toC(), L_GET_C_BACK_PTR(dialoutParams), L_STRING_TO_C(subject),
				    content ? content->toC() : nullptr);
				auto session = Call::toCpp(call)->getActiveSession();
				setMainSession(session);
				session->addListener(getSharedFromThis());
			} else {
				const auto &confAccount = mConfParams->getAccount();
				const auto &account =
				    confAccount ? confAccount
				                : Account::toCpp(linphone_core_get_default_account(cCore))->getSharedFromThis();
				const auto &from = account ? account->getAccountParams()->getIdentityAddress() : nullptr;
				if (!from) {
					lError() << "Unable to find account used to join " << *this;
					setState(ConferenceInterface::State::CreationFailed);
					setMainSession(nullptr);
					delete dialoutParams;
					return;
				}
				auto session = mMe->createSession(getCore(), dialoutParams, TRUE);
				setMainSession(session);
				session->addListener(getSharedFromThis());
				session->configure(LinphoneCallOutgoing, account, nullptr, from, remoteAddress);
				bool defer = session->initiateOutgoing(subject, content);
				if (!defer) {
					session->startInvite(nullptr, subject, content);
				}
			}
			delete dialoutParams;
		}
	}
}

void ClientConference::onCallSessionStateChanged(const shared_ptr<CallSession> &session,
                                                 CallSession::State state,
                                                 const string &message) {
	if (getConferenceAddress()) {
		if ((session == getMainSession()) || (mState == ConferenceInterface::State::CreationFailed))
			onFocusCallStateChanged(state, message);
		else {
			list<std::shared_ptr<Call>>::iterator it =
			    find_if(mPendingCalls.begin(), mPendingCalls.end(),
			            [&session](const auto &call) { return session == call->getActiveSession(); });
			if (it != mPendingCalls.end()) {
				onPendingCallStateChanged(*it, state);
			}
		}
	} else {
		switch (state) {
			case CallSession::State::Error:
				setState(ConferenceInterface::State::CreationFailed);
				setMainSession(nullptr);
				break;
			case CallSession::State::StreamsRunning:
				session->terminate();
				setMainSession(nullptr);
				break;
			default:
				break;
		}
		// During the creation of the conference or chatroom, the main session is the session used to create the
		// conference on the server
		const auto &chatRoom = getChatRoom();
		if (mConfParams->chatEnabled() && chatRoom) {
			linphone_chat_room_notify_session_state_changed(chatRoom->toC(), static_cast<LinphoneCallState>(state),
			                                                L_STRING_TO_C(message));
		}
	}
}

void ClientConference::onCallSessionSetReleased(const shared_ptr<CallSession> &session) {
#ifdef HAVE_ADVANCED_IM
	auto chatRoom = getChatRoom();
	auto clientGroupChatRoom = dynamic_pointer_cast<ClientChatRoom>(chatRoom);
#endif // HAVE_ADVANCED_IM

	bool creationFailed = (mState == ConferenceInterface::State::CreationFailed);
	if ((session == getMainSession()) || creationFailed) {
#ifdef HAVE_ADVANCED_IM
		if (clientGroupChatRoom && clientGroupChatRoom->getDeletionOnTerminationEnabled()) {
			shared_ptr<Conference> ref = getSharedFromThis();
			Core::deleteChatRoom(clientGroupChatRoom);
			clientGroupChatRoom->setDeletionOnTerminationEnabled(false);
			setState(ConferenceInterface::State::Deleted);
		}
#endif // HAVE_ADVANCED_IM
		setMainSession(nullptr);
		mFocus->removeSession();
	}
}

void ClientConference::requestFullState() {
#ifdef HAVE_ADVANCED_IM
	if (mEventHandler) {
		mEventHandler->requestFullState();
	}
#endif // HAVE_ADVANCED_IM
}

void ClientConference::unsubscribe() {
#ifdef HAVE_ADVANCED_IM
	if (mEventHandler) {
		mEventHandler->unsubscribe(); // Required for next subscribe to be sent
	}
#endif // HAVE_ADVANCED_IM
}

void ClientConference::subscribe(BCTBX_UNUSED(bool addToListEventHandler), BCTBX_UNUSED(bool unsubscribeFirst)) {
#ifdef HAVE_ADVANCED_IM
	if (mEventHandler) {
		if (unsubscribeFirst) {
			mEventHandler->unsubscribe(); // Required for next subscribe to be sent
		}
	} else {
		initializeHandlers(this, addToListEventHandler);
	}
	mEventHandler->subscribe(getConferenceId());
#endif // HAVE_ADVANCED_IM
}

#ifdef HAVE_ADVANCED_IM
shared_ptr<ClientEktManager> ClientConference::getClientEktManager() const {
	return mClientEktManager;
}
#endif // HAVE_ADVANCED_IM

LINPHONE_END_NAMESPACE
