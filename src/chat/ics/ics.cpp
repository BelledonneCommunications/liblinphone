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

#include <iomanip>
#include <sstream>

#include "ics.h"

#include "bctoolbox/utils.hh"
#include "chat/ics/parser/ics-parser.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

const std::string &Ics::Event::getOrganizer () const {
	return mOrganizer;
}

void Ics::Event::setOrganizer (const std::string &organizer) {
	mOrganizer = organizer;
}

const std::list<std::string> &Ics::Event::getAttendees () const {
	return mAttendees;
}

void Ics::Event::addAttendee (const std::string &attendee) {
	mAttendees.push_back(attendee);
}

tm Ics::Event::getDateTimeStart () const {
	return mDateTimeStart;
}

void Ics::Event::setDateTimeStart (tm dateTimeStart) {
	mDateTimeStart = dateTimeStart;
}

tm Ics::Event::getDuration () const {
	return mDuration;
}

void Ics::Event::setDuration (tm duration) {
	mDuration = duration;
}

const std::string &Ics::Event::getSummary () const {
	return mSummary;
}

void Ics::Event::setSummary (const std::string &summary) {
	mSummary = Utils::trim(summary);
}

const std::string &Ics::Event::getDescription () const {
	return mDescription;
}

void Ics::Event::setDescription (const std::string &description) {
	mDescription = Utils::trim(description);
}

const std::string &Ics::Event::getXConfUri () const {
	return mXConfUri;
}

void Ics::Event::setXConfUri (const std::string &xConfUri) {
	mXConfUri = xConfUri;
}

static std::string escape (std::string str) {
	ostringstream output;

	std::for_each(str.cbegin(), str.cend(), [&output] (char c) {
		switch(c) {
			case '\\': output << "\\\\"; break;
			case '\n': output << "\\n"; break;
			case ';': output << "\\;"; break;
			case ',': output << "\\,"; break;
			default: output << c;
		}
	});

	return output.str();
}

std::string Ics::Event::asString () const {
	ostringstream output;

	output << "BEGIN:VEVENT\r\n";

	output << setfill('0') << "DTSTART:"
		<< setw(4) << (mDateTimeStart.tm_year + 1900)
		<< setw(2) << (mDateTimeStart.tm_mon + 1)
		<< setw(2) << mDateTimeStart.tm_mday
		<< "T"
		<< setw(2) << mDateTimeStart.tm_hour
		<< setw(2) << mDateTimeStart.tm_min
		<< setw(2) << mDateTimeStart.tm_sec
		<< "Z\r\n";

	if (mDuration.tm_hour > 0 || mDuration.tm_min > 0 || mDuration.tm_sec > 0) {
		output << "DURATION:PT";
		if (mDuration.tm_hour > 0) output << mDuration.tm_hour << "H";
		if (mDuration.tm_min > 0) output << mDuration.tm_min << "M";
		if (mDuration.tm_sec > 0) output << mDuration.tm_sec << "S";
		output << "\r\n";
	}

	if (!mOrganizer.empty()) output << "ORGANIZER:" << mOrganizer << "\r\n";
	if (!mAttendees.empty()) {
		for (const auto &attendee : mAttendees) {
			output << "ATTENDEE:" << attendee << "\r\n";
		}
	}
	if (!mXConfUri.empty()) output << "X-CONFURI:" << mXConfUri << "\r\n";
	if (!mSummary.empty()) output << "SUMMARY:" << escape(mSummary) << "\r\n";
	if (!mDescription.empty()) output << "DESCRIPTION:" << escape(mDescription) << "\r\n";

	// An EVENT needs two mandatory attributes DTSTAMP AND UID
	time_t usedTime = mCreationTime != (time_t) -1 ? mCreationTime : ms_time(NULL);
	tm stamp = Utils::getTimeTAsTm(usedTime);
	output << setfill('0') << "DTSTAMP:"
		<< setw(4) << (stamp.tm_year + 1900)
		<< setw(2) << (stamp.tm_mon + 1)
		<< setw(2) << stamp.tm_mday
		<< "T"
		<< setw(2) << stamp.tm_hour
		<< setw(2) << stamp.tm_min
		<< setw(2) << stamp.tm_sec
		<< "Z\r\n";

	// For UID RFC recommends to use a DATE-TIME [some unique value] @ domain
	output << setfill('0') << "UID:"
		<< setw(4) << (stamp.tm_year + 1900)
		<< setw(2) << (stamp.tm_mon + 1)
		<< setw(2) << stamp.tm_mday
		<< "T"
		<< setw(2) << stamp.tm_hour
		<< setw(2) << stamp.tm_min
		<< setw(2) << stamp.tm_sec
		<< "Z";

	size_t p;
	if (!mOrganizer.empty() && (p = mOrganizer.find("@")) != string::npos) {
		string domain = mOrganizer.substr(p + 1, mOrganizer.size());
		output << "@" << domain << "\r\n";
	} else {
		output << "@sip.linphone.org\r\n";
	}

	output << "END:VEVENT\r\n";

	return output.str();
}

void Ics::Icalendar::addEvent (shared_ptr<Event> event) {
	mEvents.push_back(event);
}

std::string Ics::Icalendar::asString () const {
	ostringstream output;

	output << "BEGIN:VCALENDAR\r\n";
	output << "METHOD:REQUEST\r\n";
	output << "PRODID:-//Linphone//Conference calendar//EN\r\n";
	output << "VERSION:2.0\r\n";
	for (const auto &event : mEvents) {
		output << event->asString();
	}
	output << "END:VCALENDAR\r\n";

	return bctoolbox::Utils::fold(output.str());
}

std::shared_ptr<ConferenceInfo> Ics::Icalendar::toConferenceInfo () const {
	if (mEvents.empty()) return nullptr;

	auto confInfo = ConferenceInfo::create();
	const auto &event = mEvents.front(); // It should always be one event

	if (!event->getOrganizer().empty()) {
		const auto & org = IdentityAddress(event->getOrganizer());
		if (org.isValid()) {
			confInfo->setOrganizer(org);
		} else {
			lWarning() << "Could not parse organizer's address:" << event->getOrganizer() << " because it is not a valid address";
		}
	}

	for (const auto &attendee : event->getAttendees()) {
		if (!attendee.empty()) {
			const auto & addr = IdentityAddress(attendee);
			if (addr.isValid()) {
				confInfo->addParticipant(addr);
			} else {
				lWarning() << "Could not parse attendee's address:" << attendee << " because it is not a valid address";
			}
		}
	}

	confInfo->setSubject(event->getSummary());
	confInfo->setDescription(event->getDescription());

	tm dur = event->getDuration();
	int duration = dur.tm_hour*60 + dur.tm_min + dur.tm_sec/60;
	if (duration >= 0) {
		confInfo->setDuration(static_cast<unsigned int>(duration));
	}

	if (!event->getXConfUri().empty()) {
		const ConferenceAddress uri(event->getXConfUri());
		if (uri.isValid()) {
			confInfo->setUri(uri);
		} else {
			lWarning() << "Could not parse conference's uri address:" << event->getXConfUri() << " because it is not a valid address";
		}
	}

	tm start = event->getDateTimeStart();
	confInfo->setDateTime(Utils::getTmAsTimeT(start));

	if (event->mCreationTime != (time_t) -1) {
		confInfo->setCreationTime(event->mCreationTime);
	}

	return confInfo;
}

shared_ptr<const Ics::Icalendar> Ics::Icalendar::createFromString (const string &str) {
	return Ics::Parser::getInstance()->parseIcs(bctoolbox::Utils::unfold(str));
}

void Ics::Icalendar::setCreationTime(time_t time) {
	for (auto &event : mEvents) {
		event->mCreationTime = time;
	}
}

LINPHONE_END_NAMESPACE
