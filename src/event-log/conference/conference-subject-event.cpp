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

#include "conference-notified-event-p.h"
#include "conference-subject-event.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

class ConferenceSubjectEventPrivate : public ConferenceNotifiedEventPrivate {
public:
	string subject;
};

// -----------------------------------------------------------------------------

ConferenceSubjectEvent::ConferenceSubjectEvent (
	time_t creationTime,
	const ConferenceId &conferenceId,
	const string &subject
) : ConferenceNotifiedEvent(
	*new ConferenceSubjectEventPrivate,
	Type::ConferenceSubjectChanged,
	creationTime,
	conferenceId
) {
	L_D();
	d->subject = subject;
}

const string &ConferenceSubjectEvent::getSubject () const {
	L_D();
	return d->subject;
}

LINPHONE_END_NAMESPACE
