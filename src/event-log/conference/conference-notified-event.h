/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _L_CONFERENCE_NOTIFIED_EVENT_H_
#define _L_CONFERENCE_NOTIFIED_EVENT_H_

#include "conference-event.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ConferenceNotifiedEventPrivate;

class LINPHONE_PUBLIC ConferenceNotifiedEvent : public ConferenceEvent {
public:
	ConferenceNotifiedEvent (
		Type type, time_t creationTime,
		const ConferenceId &conferenceId,
		unsigned int notifiyId
	);

	unsigned int getNotifyId () const;
	inline bool getFullState () const { return fullState; };

protected:
	ConferenceNotifiedEvent (
		ConferenceNotifiedEventPrivate &p,
		Type type,
		time_t creationTime,
		const ConferenceId &conferenceId,
		unsigned int notifyId
	);

	inline void setFullState (const bool fullState) { this->fullState = fullState; };

	bool fullState = false;

private:
	L_DECLARE_PRIVATE(ConferenceNotifiedEvent);
	L_DISABLE_COPY(ConferenceNotifiedEvent);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_NOTIFIED_EVENT_H_
