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

#ifndef _L_DB_CONFERENCE_SCHEDULER_H_
#define _L_DB_CONFERENCE_SCHEDULER_H_

#include "conference/conference-scheduler.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC DBConferenceScheduler : public ConferenceScheduler {
public:
	DBConferenceScheduler(const std::shared_ptr<Core> &core, const std::shared_ptr<Account> &account = nullptr);
	virtual ~DBConferenceScheduler() = default;

	virtual void createOrUpdateConference(const std::shared_ptr<ConferenceInfo> &conferenceInfo) override;

	virtual void processResponse(const LinphoneErrorInfo *errorInfo,
	                             const std::shared_ptr<Address> conferenceAddress) override;

private:
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_DB_CONFERENCE_SCHEDULER_H_
