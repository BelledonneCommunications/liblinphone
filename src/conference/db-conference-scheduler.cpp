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

#include "conference/db-conference-scheduler.h"
#include "conference/params/call-session-params-p.h"
#include "conference/participant-info.h"
#include "conference/server-conference.h"
#include "conference/session/media-session.h"
#include "core/core-p.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

DBConferenceScheduler::DBConferenceScheduler(const shared_ptr<Core> &core, const std::shared_ptr<Account> &account)
    : ConferenceScheduler(core, account) {
}

void DBConferenceScheduler::createOrUpdateConference(
    BCTBX_UNUSED(const std::shared_ptr<ConferenceInfo> &conferenceInfo)) {
	std::shared_ptr<Address> conferenceAddress = nullptr;
	if (getState() == State::AllocationPending) {
		if (mConferenceInfo->getDateTime() <= 0) {
			// Set start time only if a conference is going to be created
			mConferenceInfo->setDateTime(ms_time(NULL));
		}
		const auto &account = getAccount() ? getAccount() : getCore()->getDefaultAccount();
		const auto &creator = account->getAccountParams()->getIdentityAddress();
		conferenceAddress = creator->clone()->toSharedPtr();
		char confId[LinphonePrivate::ServerConference::sConfIdLength];
		belle_sip_random_token(confId, sizeof(confId));
		conferenceAddress->setUriParam(Conference::ConfIdParameter, confId);
	} else {
		conferenceAddress = mConferenceInfo->getUri()->clone()->toSharedPtr();
	}
	setConferenceAddress(conferenceAddress);
}

void DBConferenceScheduler::processResponse(BCTBX_UNUSED(const LinphoneErrorInfo *errorInfo),
                                            BCTBX_UNUSED(const std::shared_ptr<Address> conferenceAddress)) {
}

LINPHONE_END_NAMESPACE
