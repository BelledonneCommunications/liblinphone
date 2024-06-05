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

SIPConferenceScheduler::~SIPConferenceScheduler() {
	if (mSession != nullptr) {
		mSession->removeListener(this);
	}
}

void SIPConferenceScheduler::createOrUpdateConference(const std::shared_ptr<ConferenceInfo> &conferenceInfo,
                                                      const std::shared_ptr<Address> &creator) {

	shared_ptr<LinphonePrivate::ConferenceParams> conferenceParams = ConferenceParams::create(getCore());
	conferenceParams->setAccount(getAccount());
	conferenceParams->enableAudio(conferenceInfo->getCapability(LinphoneStreamTypeAudio));
	conferenceParams->enableVideo(conferenceInfo->getCapability(LinphoneStreamTypeVideo));
	conferenceParams->setSubject(mConferenceInfo->getSubject());
	conferenceParams->setSecurityLevel(mConferenceInfo->getSecurityLevel());

	const auto &startTime = conferenceInfo->getDateTime();
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

	const auto &conferenceAddress = conferenceInfo->getUri();
	mSession =
	    getCore()->createOrUpdateConferenceOnServer(conferenceParams, creator, invitees, conferenceAddress, this);
	if (mSession == nullptr) {
		lError() << "[Conference Scheduler] [" << this << "] createConferenceOnServer returned a null session!";
		setState(State::Error);
		return;
	}

	if ((mConferenceInfo->getDateTime() <= 0) && (getState() == State::AllocationPending)) {
		// Set start time only if a conference is going to be created
		mConferenceInfo->setDateTime(ms_time(NULL));
	}

	if (getState() != State::Error) {
		// Update conference info in database with updated conference information
#ifdef HAVE_DB_STORAGE
		auto &mainDb = getCore()->getPrivate()->mainDb;
		mainDb->insertConferenceInfo(mConferenceInfo);
#endif // HAVE_DB_STORAGE
	}
}

void SIPConferenceScheduler::onCallSessionSetTerminated(const std::shared_ptr<CallSession> &session) {
	const std::shared_ptr<Address> remoteAddress = session->getRemoteContactAddress();
	if (remoteAddress == nullptr) {
		auto conferenceAddress = mConferenceInfo->getUri();
		lError() << "[Conference Scheduler] [" << this
		         << "] The session to update the conference information of conference "
		         << (conferenceAddress && conferenceAddress->isValid() ? conferenceAddress->toString()
		                                                               : std::string("sip:unknown"))
		         << " did not succesfully establish hence it is likely that the request wasn't taken into account by "
		            "the server";
		setState(State::Error);
	} else if (getState() != State::Error) {
		// Do not try to call impromptu conference if a participant updates its informations
		if ((getState() == State::AllocationPending) && (session->getParams()->getPrivate()->getStartTime() < 0)) {
			lInfo() << "Automatically rejoining conference " << *remoteAddress;
			auto new_params = linphone_core_create_call_params(getCore()->getCCore(), nullptr);
			// Participant with the focus call is admin
			L_GET_CPP_PTR_FROM_C_OBJECT(new_params)->addCustomContactParameter("admin", Utils::toString(true));
			std::list<Address> addressesList;
			for (const auto &participantInfo : mConferenceInfo->getParticipants()) {
				addressesList.push_back(Conference::createParticipantAddressForResourceList(participantInfo));
			}
			addressesList.sort([](const auto &addr1, const auto &addr2) { return addr1 < addr2; });
			addressesList.unique([](const auto &addr1, const auto &addr2) { return addr1.weakEqual(addr2); });

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
			const LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(getCore()->getCCore());
			bool_t initiate_video = !!linphone_video_activation_policy_get_automatically_initiate(pol);
			linphone_call_params_enable_video(
			    new_params,
			    static_pointer_cast<MediaSession>(session)->getMediaParams()->videoEnabled() && initiate_video);

			linphone_core_invite_address_with_params_2(getCore()->getCCore(), remoteAddress->toC(), new_params,
			                                           L_STRING_TO_C(mConferenceInfo->getSubject()), NULL);
			linphone_call_params_unref(new_params);
		}

		auto conferenceAddress = remoteAddress;
		setConferenceAddress(conferenceAddress);
	}
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
