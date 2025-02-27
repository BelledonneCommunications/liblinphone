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

#ifndef _L_SIP_CONFERENCE_SCHEDULER_H_
#define _L_SIP_CONFERENCE_SCHEDULER_H_

#include "conference/conference-scheduler.h"
#include "conference/session/call-session-listener.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class CallSession;

class LINPHONE_PUBLIC SIPConferenceScheduler : public CallSessionListener, public ConferenceScheduler {
public:
	SIPConferenceScheduler(const std::shared_ptr<Core> &core, const std::shared_ptr<Account> &account = nullptr);

	virtual void onCallSessionSetTerminated(const std::shared_ptr<CallSession> &session) override;
	virtual void onCallSessionStateChanged(const std::shared_ptr<CallSession> &session,
	                                       CallSession::State state,
	                                       const std::string &message) override;
	virtual void createOrUpdateConference(const std::shared_ptr<ConferenceInfo> &conferenceInfo) override;
	virtual void processResponse(const LinphoneErrorInfo *errorInfo,
	                             const std::shared_ptr<Address> conferenceAddress) override;
	inline std::shared_ptr<CallSession> getSession() {
		return mSession;
	}

private:
	std::shared_ptr<CallSession> mSession = nullptr;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_SIP_CONFERENCE_SCHEDULER_H_
