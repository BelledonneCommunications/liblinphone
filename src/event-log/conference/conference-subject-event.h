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

#ifndef _L_CONFERENCE_SUBJECT_EVENT_H_
#define _L_CONFERENCE_SUBJECT_EVENT_H_

#include <string>

#include "conference-notified-event.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ConferenceSubjectEventPrivate;

class LINPHONE_PUBLIC ConferenceSubjectEvent : public ConferenceNotifiedEvent {
	friend class Conference;
public:
	ConferenceSubjectEvent (
		time_t creationTime,
		const ConferenceId &conferenceId,
		unsigned int notifyId,
		const std::string &subject
	);

	const std::string &getSubject () const;

private:
	L_DECLARE_PRIVATE(ConferenceSubjectEvent);
	L_DISABLE_COPY(ConferenceSubjectEvent);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CONFERENCE_SUBJECT_EVENT_H_
