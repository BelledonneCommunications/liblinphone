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

#ifndef _L_ICS_H_
#define _L_ICS_H_

#include <ctime>
#include <map>
#include <string>

#include "conference/conference-info.h"
#include "linphone/utils/utils.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

namespace Ics {
class LINPHONE_PUBLIC Event {
	friend class Icalendar;

public:
	using attendee_params_t = std::map<std::string, std::string>;
	using attendee_list_t = std::map<std::string, attendee_params_t>;
	using organizer_t = std::pair<std::string, attendee_params_t>;

	Event() = default;
	~Event() = default;

	const organizer_t &getOrganizer() const;
	const std::string &getOrganizerAddress() const;
	void setOrganizer(const std::string &organizer);
	void setOrganizer(const std::string &organizer, const attendee_params_t &params);

	const attendee_list_t &getAttendees() const;
	void addAttendee(const std::string &attendee);
	void addAttendee(const std::string &attendee, const attendee_params_t &params);

	tm getDateTimeStart() const;
	void setDateTimeStart(tm dateTimeStart);

	tm getDuration() const;
	void setDuration(tm duration);

	unsigned int getSequence() const;
	void setSequence(unsigned int duration);

	const std::string &getUid() const;
	const std::string getUtf8Uid() const;
	void setUtf8Uid(const std::string &uid);
	void setUid(const std::string &uid);

	const std::string &getSummary() const;
	const std::string getUtf8Summary() const;
	void setUtf8Summary(const std::string &summary);
	void setSummary(const std::string &summary);

	const std::string &getDescription() const;
	const std::string getUtf8Description() const;
	void setUtf8Description(const std::string &description);
	void setDescription(const std::string &description);

	const std::string &getXConfUri() const;
	void setXConfUri(const std::string &xConfUri);

	std::string asString() const;

private:
	organizer_t mOrganizer;
	attendee_list_t mAttendees;
	tm mDateTimeStart;
	tm mDuration;
	unsigned int mSequence = 0;
	std::string mSummary;
	std::string mDescription;
	std::string mXConfUri;
	mutable std::string mUid;

	time_t mCreationTime = (time_t)-1; // Used by tester
};

class LINPHONE_PUBLIC Icalendar {
public:
	enum class Method {
		Request = 0,
		Cancel = 1,
		Retrieve = 2,
	};

	Icalendar() = default;
	~Icalendar() = default;

	const Method &getMethod() const;
	void setMethod(const std::string &method);
	void setMethod(const Method &method);

	void addEvent(std::shared_ptr<Event> event);

	std::string asString() const;

	std::shared_ptr<ConferenceInfo> toConferenceInfo() const;

	static std::shared_ptr<const Icalendar> createFromString(const std::string &str);

	// Used by the tester
	void setCreationTime(time_t time);

private:
	std::shared_ptr<ParticipantInfo> fillParticipantInfo(const std::shared_ptr<Address> &address,
	                                                     const Ics::Event::attendee_params_t &params) const;

	Method mMethod = Method::Request;
	std::list<std::shared_ptr<Event>> mEvents;
};
} // namespace Ics

std::ostream &operator<<(std::ostream &stream, Ics::Icalendar::Method method);

LINPHONE_END_NAMESPACE

#endif /* _L_ICS_H_ */
