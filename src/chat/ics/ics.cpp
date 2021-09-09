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
	mSummary = summary;
}

const std::string &Ics::Event::getDescription () const {
	return mDescription;
}

void Ics::Event::setDescription (const std::string &description) {
	mDescription = description;
}

const std::string &Ics::Event::getXConfUri () const {
	return mXConfUri;
}

void Ics::Event::setXConfUri (const std::string &xConfUri) {
	mXConfUri = xConfUri;
}

std::string Ics::Event::asString () const {
	ostringstream output;

	output << "BEGIN:VEVENT\r\n";

	output << setfill('0') << "DTSTART:"
		<< setw(4) << mDateTimeStart.tm_year
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
	if (!mSummary.empty()) output << "SUMMARY:" << mSummary << "\r\n";
	if (!mDescription.empty()) output << "DESCRIPTION:" << mDescription << "\r\n";
	output << "END:VEVENT\r\n";

	return output.str();
}

void Ics::Icalendar::addEvent (shared_ptr<Event> event) {
	mEvents.push_back(event);
}

std::string Ics::Icalendar::asString () const {
	ostringstream output;

	output << "BEGIN:VCALENDAR\r\n";
	output << "PRODID:-//Linphone//Conference calendar//EN\r\n";
	output << "VERSION:2.0\r\n";
	for (const auto &event : mEvents) {
		output << event->asString();
	}
	output << "END:VCALENDAR\r\n";

	return output.str();
}

std::shared_ptr<ConferenceInfo> Ics::Icalendar::toConferenceInfo () {
	if (mEvents.empty()) return nullptr;

	auto confInfo = ConferenceInfo::create();
	const auto &event = mEvents.front(); // It should always be one event

	LinphoneAddress *org = linphone_address_new(event->getOrganizer().c_str());
	if (org) {
		confInfo->setOrganizer(org);
		linphone_address_unref(org);
	} else {
		lWarning() << "Could not parse organizer's address:" << event->getOrganizer();
	}

	for (const auto &attendee : event->getAttendees()) {
		LinphoneAddress *addr = linphone_address_new(attendee.c_str());
		if (addr) {
			confInfo->addParticipant(addr);
			linphone_address_unref(addr);
		} else {
			lWarning() << "Could not parse attendee's address:" << attendee;
		}
	}

	confInfo->setSubject(event->getSummary());
	confInfo->setDescription(event->getDescription());

	tm dur = event->getDuration();
	confInfo->setDuration(dur.tm_hour*60 + dur.tm_min + dur.tm_sec/60);

	LinphoneAddress *uri = linphone_address_new(event->getXConfUri().c_str());
	if (uri) {
		confInfo->setUri(uri);
		linphone_address_unref(uri);
	} else {
		lWarning() << "Could not parse conference's uri address:" << event->getSummary();
	}

	tm start = event->getDateTimeStart();
	confInfo->setDateTime(Utils::getTmAsTimeT(start));

	return confInfo;
}

shared_ptr<const Ics::Icalendar> Ics::Icalendar::createFromString (const string &str) {
	return Ics::Parser::getInstance()->parseIcs(str);
}

LINPHONE_END_NAMESPACE
