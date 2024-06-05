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

#ifndef _L_CCMP_CONFERENCE_SCHEDULER_H_
#define _L_CCMP_CONFERENCE_SCHEDULER_H_

#include "conference/conference-scheduler.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC CCMPConferenceScheduler : public ConferenceScheduler {
public:
	CCMPConferenceScheduler(const std::shared_ptr<Core> &core, const std::shared_ptr<Account> &account = nullptr);
	virtual ~CCMPConferenceScheduler() = default;

	virtual void createOrUpdateConference(const std::shared_ptr<ConferenceInfo> &conferenceInfo,
	                                      const std::shared_ptr<Address> &creator) override;

	virtual void processResponse(const LinphoneErrorInfo *errorInfo,
	                             const std::shared_ptr<Address> conferenceAddress) override;

	void setCcmpUri(const std::string &ccmpUri);

	static void handleResponse(void *ctx, const belle_http_response_event_t *event);
	static void handleAuthRequested(BCTBX_UNUSED(void *ctx), belle_sip_auth_event_t *event);
	static void handleTimeout(BCTBX_UNUSED(void *ctx), BCTBX_UNUSED(const belle_sip_timeout_event_t *event));
	static void handleIoError(BCTBX_UNUSED(void *ctx), BCTBX_UNUSED(const belle_sip_io_error_event_t *event));

private:
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CCMP_CONFERENCE_SCHEDULER_H_
