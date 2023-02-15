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

#ifndef _L_CONFERENCE_AVAILABLE_MEDIA_EVENT_H_
#define _L_CONFERENCE_AVAILABLE_MEDIA_EVENT_H_

#include <map>

#include "conference-notified-event.h"
#include "conference/conference-enums.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ConferenceAvailableMediaEventPrivate;

class LINPHONE_PUBLIC ConferenceAvailableMediaEvent : public ConferenceNotifiedEvent {
	friend class Conference;

public:
	ConferenceAvailableMediaEvent(time_t creationTime,
	                              const ConferenceId &conferenceId,
	                              const std::map<ConferenceMediaCapabilities, bool> mediaCapabilties);

	const std::map<ConferenceMediaCapabilities, bool> &getAvailableMediaType() const;
	bool audioEnabled() const;
	bool videoEnabled() const;
	bool chatEnabled() const;

private:
	L_DECLARE_PRIVATE(ConferenceAvailableMediaEvent);
	L_DISABLE_COPY(ConferenceAvailableMediaEvent);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_AVAILABLE_MEDIA_EVENT_H_
