/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#include "conference/params/call-session-params-p.h"
#include "conference/participant-info.h"
#include "conference/session/media-session.h"
#include "conference/sip-conference-scheduler.h"
#include "core/core-p.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

SIPConferenceScheduler::SIPConferenceScheduler(const shared_ptr<Core> &core, const std::shared_ptr<Account> &account)
    : ConferenceScheduler(core, account) {
}

void SIPConferenceScheduler::createOrUpdateConference(const std::shared_ptr<ConferenceInfo> &conferenceInfo) {

	shared_ptr<LinphonePrivate::ConferenceParams> conferenceParams = ConferenceParams::create(getCore());
	conferenceParams->setAccount(getAccount());
	conferenceParams->enableAudio(conferenceInfo->getCapability(LinphoneStreamTypeAudio));
	conferenceParams->enableVideo(conferenceInfo->getCapability(LinphoneStreamTypeVideo));
	conferenceParams->enableChat(conferenceInfo->getCapability(LinphoneStreamTypeText));
	conferenceParams->setUtf8Subject(mConferenceInfo->getUtf8Subject());
	conferenceParams->setSecurityLevel(mConferenceInfo->getSecurityLevel());

	const auto &startTime = conferenceInfo->getDateTime();

	if (startTime < 0) {
		lError() << "[Conference Scheduler] [" << this << "] unable to schedule a conference with start time "
		         << startTime;
		setState(State::Error);
		return;
	}

	conferenceParams->setStartTime(startTime);
	const auto &duration = conferenceInfo->getDuration();
	if (duration > 0) {
		const auto endTime = startTime + static_cast<time_t>(duration) * 60; // duration is in minutes
		conferenceParams->setEndTime(endTime);
	}

	std::list<Address> invitees;
	for (const auto &p : mConferenceInfo->getParticipants()) {
		invitees.push_back(Conference::createParticipantAddressForResourceList(p));
	}

	auto ref = getSharedFromThis();
	const auto &conferenceAddress = conferenceInfo->getUri();
	mSession = getCore()->createOrUpdateConferenceOnServer(conferenceParams, invitees, conferenceAddress,
	                                                       dynamic_pointer_cast<SIPConferenceScheduler>(ref));
	if (mSession == nullptr) {
		lError() << "[Conference Scheduler] [" << this
		         << "]: Unable to create the conference because the SIP session is null!";
		setState(State::Error);
		return;
	}

	// Add the conference listener to the list held by the core in order to ensure that listener function are correctly
	// called. In fact, object CallSession takes a weak reference of the ConferenceScheduler therefore the core must
	// hold a strong one
	getCore()->addConferenceScheduler(ref);

	if ((mConferenceInfo->getDateTime() <= 0) && (getState() == State::AllocationPending)) {
		// Set start time only if a conference is going to be created
		mConferenceInfo->setDateTime(ms_time(NULL));
	}
}

void SIPConferenceScheduler::onCallSessionSetTerminated(const std::shared_ptr<CallSession> &session) {
	const std::shared_ptr<Address> remoteAddress = session->getRemoteContactAddress();
	if (remoteAddress == nullptr) {
		auto conferenceAddress = mConferenceInfo->getUri();
		lError() << "[Conference Scheduler] [" << this
		         << "] The session to update the conference information of conference "
		         << (conferenceAddress && conferenceAddress->isValid() ? conferenceAddress->toString()
		                                                               : std::string("sip:"))
		         << " did not successfully establish hence it is likely that the request wasn't taken into account by "
		            "the server";
		setState(State::Error);
	} else if (getState() != State::Error) {
		// Update conference info in database with updated conference information
#ifdef HAVE_DB_STORAGE
		auto &mainDb = getCore()->getPrivate()->mainDb;
		mainDb->insertConferenceInfo(mConferenceInfo);
#endif // HAVE_DB_STORAGE

		auto conferenceAddress = remoteAddress;
		setConferenceAddress(conferenceAddress);
	}
	getCore()->removeConferenceScheduler(getSharedFromThis());
}

void SIPConferenceScheduler::onCallSessionStateChanged(const shared_ptr<CallSession> &session,
                                                       CallSession::State state,
                                                       BCTBX_UNUSED(const string &message)) {
	switch (state) {
		case CallSession::State::Error:
			setState(State::Error);
			break;
		case CallSession::State::StreamsRunning: {
			const LinphoneErrorInfo *errorInfo = session->getErrorInfo();
			const std::shared_ptr<Address> address = session->getRemoteAddress();
			processResponse(errorInfo, address);
			break;
		}
		default:
			break;
	}
}

void SIPConferenceScheduler::processResponse(BCTBX_UNUSED(const LinphoneErrorInfo *errorInfo),
                                             BCTBX_UNUSED(const std::shared_ptr<Address> conferenceAddress)) {
	mSession->terminate();
}

LINPHONE_END_NAMESPACE
