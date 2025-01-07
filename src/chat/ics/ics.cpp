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

#include <algorithm>
#include <iomanip>
#include <sstream>

#include "bctoolbox/utils.hh"

#include "chat/ics/parser/ics-parser.h"
#include "conference/conference-params.h"
#include "conference/participant-info.h"
#include "ics.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

const Ics::Event::organizer_t &Ics::Event::getOrganizer() const {
	return mOrganizer;
}

const std::string &Ics::Event::getOrganizerAddress() const {
	return getOrganizer().first;
}

void Ics::Event::setOrganizer(const std::string &organizer) {
	Ics::Event::attendee_params_t params;
	setOrganizer(organizer, params);
}

void Ics::Event::setOrganizer(const std::string &organizer, const Ics::Event::attendee_params_t &params) {
	mOrganizer = std::make_pair(organizer, params);
}

const Ics::Event::attendee_list_t &Ics::Event::getAttendees() const {
	return mAttendees;
}

void Ics::Event::addAttendee(const std::string &attendee) {
	Ics::Event::attendee_params_t params;
	addAttendee(attendee, params);
}

void Ics::Event::addAttendee(const std::string &attendee, const Ics::Event::attendee_params_t &params) {
	mAttendees.insert(std::make_pair(attendee, params));
}

tm Ics::Event::getDateTimeStart() const {
	return mDateTimeStart;
}

void Ics::Event::setDateTimeStart(tm dateTimeStart) {
	mDateTimeStart = dateTimeStart;
}

tm Ics::Event::getDuration() const {
	return mDuration;
}

void Ics::Event::setDuration(tm duration) {
	mDuration = duration;
}

const std::string Ics::Event::getUtf8Summary() const {
	return Utils::localeToUtf8(mSummary);
}

const std::string &Ics::Event::getSummary() const {
	return mSummary;
}

void Ics::Event::setUtf8Summary(const std::string &summary) {
	mSummary = Utils::trim(Utils::utf8ToLocale(summary));
}

void Ics::Event::setSummary(const std::string &summary) {
	mSummary = Utils::trim(summary);
}

unsigned int Ics::Event::getSequence() const {
	return mSequence;
}

void Ics::Event::setSequence(unsigned int sequence) {
	mSequence = sequence;
}

const std::string Ics::Event::getUtf8Uid() const {
	return Utils::localeToUtf8(mUid);
}

const std::string &Ics::Event::getUid() const {
	return mUid;
}

void Ics::Event::setUtf8Uid(const std::string &uid) {
	mUid = Utils::trim(Utils::utf8ToLocale(uid));
}

void Ics::Event::setUid(const std::string &uid) {
	mUid = Utils::trim(uid);
}

const std::string Ics::Event::getUtf8Description() const {
	return Utils::localeToUtf8(mDescription);
}

const std::string &Ics::Event::getDescription() const {
	return mDescription;
}

void Ics::Event::setUtf8Description(const std::string &description) {
	mDescription = Utils::trim(Utils::utf8ToLocale(description));
}

void Ics::Event::setDescription(const std::string &description) {
	mDescription = Utils::trim(description);
}

const std::string &Ics::Event::getXConfUri() const {
	return mXConfUri;
}

void Ics::Event::setXConfUri(const std::string &xConfUri) {
	mXConfUri = xConfUri;
}

static std::string escape(std::string str) {
	ostringstream output;

	std::for_each(str.cbegin(), str.cend(), [&output](char c) {
		switch (c) {
			case '\\':
				output << "\\\\";
				break;
			case '\n':
				output << "\\n";
				break;
			case ';':
				output << "\\;";
				break;
			case ',':
				output << "\\,";
				break;
			default:
				output << c;
		}
	});

	return output.str();
}

std::string Ics::Event::asString() const {
	ostringstream output;

	output << "BEGIN:VEVENT\r\n";

	output << setfill('0') << "DTSTART:" << setw(4) << (mDateTimeStart.tm_year + 1900) << setw(2)
	       << (mDateTimeStart.tm_mon + 1) << setw(2) << mDateTimeStart.tm_mday << "T" << setw(2)
	       << mDateTimeStart.tm_hour << setw(2) << mDateTimeStart.tm_min << setw(2) << mDateTimeStart.tm_sec << "Z\r\n";

	if (mDuration.tm_hour > 0 || mDuration.tm_min > 0 || mDuration.tm_sec > 0) {
		output << "DURATION:PT";
		if (mDuration.tm_hour > 0) output << mDuration.tm_hour << "H";
		if (mDuration.tm_min > 0) output << mDuration.tm_min << "M";
		if (mDuration.tm_sec > 0) output << mDuration.tm_sec << "S";
		output << "\r\n";
	}

	const auto &organizerAddress = getOrganizerAddress();
	if (!organizerAddress.empty()) {
		output << "ORGANIZER";
		const auto &params = mOrganizer.second;
		for (const auto &[name, value] : params) {
			output << ";" << name << "=" << value;
		}
		output << ":" << organizerAddress;
		output << "\r\n";
	}
	if (!mAttendees.empty()) {
		for (const auto &[address, params] : mAttendees) {
			output << "ATTENDEE";
			for (const auto &[name, value] : params) {
				output << ";" << name << "=" << value;
			}
			output << ":" << address;
			output << "\r\n";
		}
	}
	if (!mXConfUri.empty()) output << "X-CONFURI:" << mXConfUri << "\r\n";
	if (!mSummary.empty()) output << "SUMMARY:" << Utils::localeToUtf8(escape(mSummary)) << "\r\n";
	if (!mDescription.empty()) output << "DESCRIPTION:" << Utils::localeToUtf8(escape(mDescription)) << "\r\n";
	if (mSequence != 0) output << "SEQUENCE:" << mSequence << "\r\n";

	// An EVENT needs two mandatory attributes DTSTAMP AND UID
	time_t usedTime = mCreationTime != (time_t)-1 ? mCreationTime : ms_time(NULL);
	tm stamp = Utils::getTimeTAsTm(usedTime);
	output << setfill('0') << "DTSTAMP:" << setw(4) << (stamp.tm_year + 1900) << setw(2) << (stamp.tm_mon + 1)
	       << setw(2) << stamp.tm_mday << "T" << setw(2) << stamp.tm_hour << setw(2) << stamp.tm_min << setw(2)
	       << stamp.tm_sec << "Z\r\n";

	// For UID RFC recommends to use a DATE-TIME [some unique value] @ domain
	output << setfill('0') << "UID:";

	if (mUid.empty()) {
		ostringstream uid;
		uid << setw(4) << (stamp.tm_year + 1900) << (stamp.tm_mon + 1) << stamp.tm_mday << "T" << stamp.tm_hour
		    << stamp.tm_min << stamp.tm_sec << "Z";

		size_t p;
		if (!organizerAddress.empty() && (p = organizerAddress.find("@")) != string::npos) {
			string domain = organizerAddress.substr(p + 1, organizerAddress.size());
			uid << "@" << domain;
		} else {
			uid << "@domain.invalid";
		}
		mUid = uid.str();
	}
	output << mUid << "\r\n";

	output << "END:VEVENT\r\n";

	return output.str();
}

const Ics::Icalendar::Method &Ics::Icalendar::getMethod() const {
	return mMethod;
}

void Ics::Icalendar::setMethod(const std::string &method) {
	if (method.compare("REQUEST") == 0) {
		setMethod(Ics::Icalendar::Method::Request);
	} else if (method.compare("CANCEL") == 0) {
		setMethod(Ics::Icalendar::Method::Cancel);
	} else if (method.compare("RETRIEVE") == 0) {
		setMethod(Ics::Icalendar::Method::Retrieve);
	} else {
		lError() << "ICS method " << method << " is not currently supported";
	}
}

void Ics::Icalendar::setMethod(const Ics::Icalendar::Method &method) {
	mMethod = method;
}

void Ics::Icalendar::addEvent(shared_ptr<Event> event) {
	mEvents.push_back(event);
}

std::string Ics::Icalendar::asString() const {
	ostringstream output;

	output << "BEGIN:VCALENDAR\r\n";
	output << "METHOD:";
	output << mMethod;
	output << "\r\n";
	output << "PRODID:-//Linphone//Conference calendar//EN\r\n";
	output << "VERSION:2.0\r\n";
	for (const auto &event : mEvents) {
		output << event->asString();
	}
	output << "END:VCALENDAR\r\n";

	return bctoolbox::Utils::fold(output.str());
}

std::shared_ptr<ParticipantInfo>
Ics::Icalendar::fillParticipantInfo(const std::shared_ptr<Address> &address,
                                    const Ics::Event::attendee_params_t &params) const {
	auto participantInfo = ParticipantInfo::create(address);
	participantInfo->setParameters(params);
	return participantInfo;
}

std::shared_ptr<ConferenceInfo> Ics::Icalendar::toConferenceInfo() const {
	if (mEvents.empty()) return nullptr;

	auto confInfo = ConferenceInfo::create();
	const auto &event = mEvents.front(); // It should always be one event

	if (!event->getOrganizerAddress().empty()) {
		const auto &[orgAddressStr, orgParams] = event->getOrganizer();
		const auto &orgAddress = Address::create(orgAddressStr);
		if (orgAddress && orgAddress->isValid()) {
			const auto organizerInfo = fillParticipantInfo(orgAddress, orgParams);
			confInfo->setOrganizer(organizerInfo);
		} else {
			lWarning() << "Could not parse organizer's address: " << event->getOrganizerAddress()
			           << " because it is not a valid address";
		}
	}

	for (const auto &[address, params] : event->getAttendees()) {
		if (!address.empty()) {
			const auto addr = Address::create(address);
			if (addr && addr->isValid()) {
				const auto participantInfo = fillParticipantInfo(addr, params);
				confInfo->addParticipant(participantInfo);
			} else {
				lWarning() << "Could not parse attendee's address: " << address << " because it is not a valid address";
			}
		}
	}

	confInfo->setUtf8Subject(event->getUtf8Summary());
	confInfo->setUtf8Description(event->getUtf8Description());

	tm dur = event->getDuration();
	int duration = dur.tm_hour * 60 + dur.tm_min + dur.tm_sec / 60;
	if (duration >= 0) {
		confInfo->setDuration(static_cast<unsigned int>(duration));
	}

	if (!event->getXConfUri().empty()) {
		const std::shared_ptr<Address> uri = Address::create(event->getXConfUri());
		if (uri && uri->isValid()) {
			confInfo->setUri(uri);
		} else {
			lWarning() << "Could not parse conference's uri address:" << event->getXConfUri()
			           << " because it is not a valid address";
		}
	}

	ConferenceParams::SecurityLevel securityLevel = ConferenceParams::SecurityLevel::None;
	lInfo() << "Setting the conference security level to " << securityLevel
	        << " as we don't have received the notify full state yet";
	confInfo->setSecurityLevel(securityLevel);

	tm start = event->getDateTimeStart();
	confInfo->setDateTime(Utils::getTmAsTimeT(start));

	if (event->mCreationTime != (time_t)-1) {
		confInfo->setCreationTime(event->mCreationTime);
	}

	confInfo->setIcsSequence(event->getSequence());
	confInfo->setIcsUid(event->getUid());

	ConferenceInfo::State state = ConferenceInfo::State::New;
	switch (mMethod) {
		case Ics::Icalendar::Method::Retrieve:
		case Ics::Icalendar::Method::Request:
			state = ((event->getSequence() == 0) ? ConferenceInfo::State::New : ConferenceInfo::State::Updated);
			break;
		case Ics::Icalendar::Method::Cancel:
			state = ConferenceInfo::State::Cancelled;
			break;
	}
	confInfo->setState(state);
	return confInfo;
}

shared_ptr<const Ics::Icalendar> Ics::Icalendar::createFromString(const string &str) {
	return Ics::Parser::getInstance()->parseIcs(bctoolbox::Utils::unfold(str));
}

void Ics::Icalendar::setCreationTime(time_t time) {
	for (auto &event : mEvents) {
		event->mCreationTime = time;
	}
}

ostream &operator<<(ostream &stream, Ics::Icalendar::Method method) {
	switch (method) {
		case Ics::Icalendar::Method::Request:
			return stream << "REQUEST";
		case Ics::Icalendar::Method::Cancel:
			return stream << "CANCEL";
		case Ics::Icalendar::Method::Retrieve:
			return stream << "RETRIEVE";
	}
	return stream;
}
LINPHONE_END_NAMESPACE
