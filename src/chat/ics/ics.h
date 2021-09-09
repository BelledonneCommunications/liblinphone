/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
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

#ifndef _L_ICS_H_
#define _L_ICS_H_

#include <ctime>
#include <list>
#include <string>

#include "conference/conference-info.h"
#include "linphone/utils/utils.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

namespace Ics {
	class LINPHONE_PUBLIC Event {
	public:
		Event () = default;
		~Event () = default;

		const std::string &getOrganizer () const;
		void setOrganizer (const std::string &organizer);

		const std::list<std::string> &getAttendees () const;
		void addAttendee (const std::string &attendee);

		tm getDateTimeStart () const;
		void setDateTimeStart (tm dateTimeStart);

		tm getDuration () const;
		void setDuration (tm duration);

		const std::string &getSummary () const;
		void setSummary (const std::string &summary);

		const std::string &getDescription () const;
		void setDescription (const std::string &description);

		const std::string &getXConfUri () const;
		void setXConfUri (const std::string &xConfUri);

		std::string asString () const;

	private:
		std::string mOrganizer;
		std::list<std::string> mAttendees;
		tm mDateTimeStart;
		tm mDuration;
		std::string mSummary;
		std::string mDescription;
		std::string mXConfUri;
	};

	class LINPHONE_PUBLIC Icalendar {
	public:
		Icalendar () = default;
		~Icalendar () = default;

		void addEvent (std::shared_ptr<Event> event);

		std::string asString () const;

		std::shared_ptr<ConferenceInfo> toConferenceInfo ();

		static std::shared_ptr<const Icalendar> createFromString (const std::string &str);

	private:
		std::list<std::shared_ptr<Event>> mEvents;
	};
}

LINPHONE_END_NAMESPACE

#endif /* _L_ICS_H_ */