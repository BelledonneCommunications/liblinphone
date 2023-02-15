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

#ifndef _L_CONFERENCE_LISTENER_H_
#define _L_CONFERENCE_LISTENER_H_

#include <bctoolbox/defs.h>

#include <vector>

#include "event-log/events.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC ConferenceListener {
public:
	virtual ~ConferenceListener() = default;

	virtual void onConferenceCreated(BCTBX_UNUSED(const std::shared_ptr<Address> &addr)) {
	}
	virtual void onConferenceKeywordsChanged(BCTBX_UNUSED(const std::vector<std::string> &keywords)) {
	}
	virtual void onConferenceTerminated(BCTBX_UNUSED(const std::shared_ptr<Address> &addr)) {
	}
	virtual void onSecurityEvent(BCTBX_UNUSED(const std::shared_ptr<ConferenceSecurityEvent> &event)) {
	}
	virtual void onFirstNotifyReceived(BCTBX_UNUSED(const std::shared_ptr<Address> &addr)) {
	}
	virtual void onParticipantsCleared() {
	}
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_LISTENER_H_
