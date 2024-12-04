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

#include <set>

#include "bctoolbox/utils.hh"
#include <belr/abnf.h>
#include <belr/grammarbuilder.h>

#include "linphone/utils/utils.h"

#include "chat/ics/ics.h"
#include "content/content-type.h"
#include "logger/logger.h"
#include "object/object-p.h"

#include "ics-parser.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

namespace {
string IcsGrammar("ics_grammar.belr");
}

namespace Ics {
class Node {
public:
	virtual ~Node() = default;
};

class DateTimeNode : public Node {
public:
	DateTimeNode() = default;

	const tm getDate() {
		tm time = {0};
		time.tm_year = mYear - 1900;
		time.tm_mon = mMonth - 1;
		time.tm_mday = mDay;
		time.tm_hour = mHour;
		time.tm_min = mMinute;
		time.tm_sec = mSecond;

		return time;
	}

	void setYear(const string &year) {
		mYear = Utils::stoi(year);
	}

	void setMonth(const string &month) {
		mMonth = Utils::stoi(month);
	}

	void setDay(const string &day) {
		mDay = Utils::stoi(day);
	}

	void setHour(const string &hour) {
		mHour = Utils::stoi(hour);
	}

	void setMinute(const string &minute) {
		mMinute = Utils::stoi(minute);
	}

	void setSecond(const string &second) {
		mSecond = Utils::stoi(second);
	}

	void setUtc(const string &utc) {
		mUtc = !utc.empty() && utc == "Z";
	}

private:
	int mYear;
	int mMonth;
	int mDay;
	int mHour;
	int mMinute;
	int mSecond;
	bool mUtc;
};

class DurationNode : public Node {
public:
	DurationNode() = default;

	tm getDuration() {
		tm duration = {0};

		duration.tm_hour = mHour;
		duration.tm_min = mMinute;
		duration.tm_sec = mSecond;

		return duration;
	}

	void setHour(const string &hour) {
		if (hour.empty()) return;

		// It can also contains minutes and seconds so remove all after 'H'
		size_t p = hour.find("H");
		if (p != string::npos) {
			string tmp = hour.substr(0, p - 1);
			mHour = Utils::stoi(hour);
		}
	}

	void setMinute(const string &minute) {
		if (minute.empty()) return;

		// It can also contains seconds so remove all after 'M'
		size_t p = minute.find("M");
		if (p != string::npos) {
			string tmp = minute.substr(0, p - 1);
			mMinute = Utils::stoi(minute);
		}
	}

	void setSecond(const string &second) {
		string tmp = second.substr(0, second.size() - 1); // Removes "S"
		mSecond = Utils::stoi(second);
	}

private:
	int mHour;
	int mMinute;
	int mSecond;
};
void replace_all(string &inout, string what, string with) {
	for (string::size_type pos{}; inout.npos != (pos = inout.find(what.data(), pos, what.length()));
	     pos += with.length()) {
		inout.replace(pos, what.length(), with.data(), with.length());
	}
}

class EventNode : public Node {
public:
	EventNode() = default;

	void setUtf8Summary(const string &summary) {
		setSummary(Utils::utf8ToLocale(summary));
	}

	void setSummary(const string &summary) {
		mSummary = summary;

		// We need to unescape "\n", "\", ";", ","
		replace_all(mSummary, "\\n", "\n");
		replace_all(mSummary, "\\;", ";");
		replace_all(mSummary, "\\,", ",");
		replace_all(mSummary, "\\\\", "\\");
	}

	void setUtf8Description(const string &description) {
		setDescription(Utils::utf8ToLocale(description));
	}

	void setDescription(const string &description) {
		mDescription = description;

		// We need to unescape "\n", "\", ";", ","
		replace_all(mDescription, "\\n", "\n");
		replace_all(mDescription, "\\;", ";");
		replace_all(mDescription, "\\,", ",");
		replace_all(mDescription, "\\\\", "\\");
	}

	void setXProp(const string &xProp) {
		if (xProp.empty()) return;
		string prop = Utils::trim(xProp);

		// Check if the prop is X-CONFURI
		size_t p = prop.find(":");
		if (p != string::npos) {
			string name = prop.substr(0, p);
			string value = prop.substr(p + 1, prop.size());

			p = name.find(";");
			if (p != string::npos) {
				name = name.substr(0, p - 1);
			}

			if (name == "X-CONFURI") {
				mXConfUri = value;
			}
		}
	}

	Ics::Event::organizer_t parseMemberLine(const string &line, const string keyword) {
		if (!line.empty()) {
			Ics::Event::attendee_params_t params;
			size_t paramStart = line.find(keyword);
			// Chop off ATTENDEE
			const auto &paramAddress = line.substr(paramStart + keyword.size());
			size_t addressStart = paramAddress.find(":");
			// Split parameters and address.
			// Parameters end at the first : sign
			const auto &paramsStr = paramAddress.substr(0, addressStart);
			// The address length is the size of the string contaning member parameters and address minus the position
			// at which the address starts minus 1
			const auto addressLength = paramAddress.size() - addressStart - 3;
			const auto &address = paramAddress.substr(addressStart + 1, addressLength);
			if (!paramsStr.empty()) {
				const auto &splittedValue = bctoolbox::Utils::split(Utils::trim(paramsStr), ";");
				for (const auto &param : splittedValue) {
					if (!param.empty()) {
						auto equal = param.find("=");
						string name = param.substr(0, equal);
						string value = param.substr(equal + 1, param.size());
						params.insert(std::make_pair(name, value));
					}
				}
			}
			return std::make_pair(address, params);
		}
		return Ics::Event::organizer_t();
	}

	void setOrganizer(const string &organizer) {
		mOrganizer = parseMemberLine(organizer, "ORGANIZER");
	}

	void addAttendee(const string &attendee) {
		auto mAttendee = parseMemberLine(attendee, "ATTENDEE");
		if (mAttendee != Ics::Event::organizer_t()) {
			mAttendees.insert(mAttendee);
		}
	}

	void setUid(const string &xUid) {
		string uid = Utils::trim(xUid);
		size_t p = uid.find(":");
		if (p != string::npos) {
			string name = uid.substr(0, p);
			string value = uid.substr(p + 1, uid.size());

			p = name.find(";");
			if (p != string::npos) {
				name = name.substr(0, p - 1);
			}

			if (name == "UID") {
				mUid = value;
			}
		}
	}

	void setSequence(const string &xSequence) {
		string sequence = Utils::trim(xSequence);
		size_t p = sequence.find(":");
		if (p != string::npos) {
			string name = sequence.substr(0, p);
			string value = sequence.substr(p + 1, sequence.size());

			p = name.find(";");
			if (p != string::npos) {
				name = name.substr(0, p - 1);
			}

			if (name == "SEQUENCE") {
				mSequence = static_cast<unsigned int>(Utils::stoi(value));
			}
		}
	}

	void setDateStart(const shared_ptr<DateTimeNode> &dateStart) {
		mDateStart = dateStart;
	}

	void setDateEnd(const shared_ptr<DateTimeNode> &dateEnd) {
		mDateEnd = dateEnd;
	}

	void setDuration(const shared_ptr<DurationNode> &duration) {
		mDuration = duration;
	}

	shared_ptr<Event> createEvent() {
		auto event = make_shared<Event>();

		event->setSummary(mSummary);
		event->setDescription(mDescription);
		tm startTimeTm = {0};
		if (mDateStart) {
			startTimeTm = mDateStart->getDate();
			event->setDateTimeStart(startTimeTm);
		}
		if (mDuration) {
			event->setDuration(mDuration->getDuration());
		} else if (mDateStart && mDateEnd) {
			time_t startTimeTimeT = Utils::getTmAsTimeT(startTimeTm);
			tm endTimeTm = mDateEnd->getDate();
			time_t endTimeTimeT = Utils::getTmAsTimeT(endTimeTm);
			time_t durationS = endTimeTimeT - startTimeTimeT;
			tm durationTm = {0};
			// There are 31536000 seconds in a year
			int yearToS = 31536000;
			durationTm.tm_year = (int)(durationS / yearToS);
			int dayToS = yearToS / 365;
			time_t durationDayS = (durationS % yearToS);
			durationTm.tm_yday = (int)(durationDayS / dayToS);
			int hourToS = dayToS / 24;
			time_t durationHourS = (durationDayS % dayToS);
			durationTm.tm_hour = (int)(durationHourS / hourToS);
			int minToS = hourToS / 60;
			time_t durationMinS = (durationHourS % hourToS);
			durationTm.tm_min = (int)(durationMinS / minToS);
			durationTm.tm_sec = (int)(durationMinS % minToS);
			event->setDuration(durationTm);
		}
		event->setOrganizer(mOrganizer.first, mOrganizer.second);
		event->setXConfUri(mXConfUri);

		for (const auto &[address, params] : mAttendees) {
			event->addAttendee(address, params);
		}

		event->setUid(mUid);
		event->setSequence(mSequence);

		return event;
	}

private:
	string mSummary;
	string mDescription;
	string mXConfUri;
	Ics::Event::organizer_t mOrganizer;
	string mUid;
	unsigned int mSequence = 0;
	Ics::Event::attendee_list_t mAttendees;
	shared_ptr<DateTimeNode> mDateStart;
	shared_ptr<DateTimeNode> mDateEnd;
	shared_ptr<DurationNode> mDuration;
};

class IcalendarNode : public Node {
public:
	IcalendarNode() = default;

	void setMethod(const string &xMethod) {
		string method = Utils::trim(xMethod);
		size_t p = method.find(":");
		if (p != string::npos) {
			string name = method.substr(0, p);
			string value = method.substr(p + 1, method.size());

			p = name.find(";");
			if (p != string::npos) {
				name = name.substr(0, p - 1);
			}

			if (name == "METHOD") {
				mMethod = value;
			}
		}
	}

	void addEvent(const shared_ptr<EventNode> &event) {
		mEvents.push_back(event);
	}

	shared_ptr<Icalendar> createIcalendar() {
		if (mEvents.empty()) return nullptr;

		auto calendar = make_shared<Icalendar>();

		calendar->setMethod(mMethod);
		// An Ics file can have multiple Events but we only use one
		calendar->addEvent(mEvents.front()->createEvent());

		return calendar;
	}

private:
	string mMethod;
	list<shared_ptr<EventNode>> mEvents;
};
} // namespace Ics

// -----------------------------------------------------------------------------

class Ics::ParserPrivate : public ObjectPrivate {
public:
	shared_ptr<belr::Parser<shared_ptr<Node>>> parser;
};

Ics::Parser::Parser() : Singleton(*new ParserPrivate) {
	L_D();

	shared_ptr<belr::Grammar> grammar = belr::GrammarLoader::get().load(IcsGrammar);
	if (!grammar) lFatal() << "Unable to load ICS grammar.";

	d->parser = make_shared<belr::Parser<shared_ptr<Node>>>(grammar);

	d->parser->setHandler("icalobject", belr::make_fn(make_shared<IcalendarNode>))
	    ->setCollector("eventc", belr::make_sfn(&IcalendarNode::addEvent))
	    ->setCollector("method", belr::make_sfn(&IcalendarNode::setMethod));

	d->parser->setHandler("eventc", belr::make_fn(make_shared<EventNode>))
	    ->setCollector("summvalue", belr::make_sfn(&EventNode::setUtf8Summary))
	    ->setCollector("descvalue", belr::make_sfn(&EventNode::setUtf8Description))
	    ->setCollector("dtstval", belr::make_sfn(&EventNode::setDateStart))
	    ->setCollector("dtendval", belr::make_sfn(&EventNode::setDateEnd))
	    ->setCollector("dur-value", belr::make_sfn(&EventNode::setDuration))
	    ->setCollector("organizer", belr::make_sfn(&EventNode::setOrganizer))
	    ->setCollector("attendee", belr::make_sfn(&EventNode::addAttendee))
	    ->setCollector("uid", belr::make_sfn(&EventNode::setUid))
	    ->setCollector("seq", belr::make_sfn(&EventNode::setSequence))
	    ->setCollector("x-prop", belr::make_sfn(&EventNode::setXProp));

	d->parser->setHandler("dtendval", belr::make_fn(make_shared<DateTimeNode>))
	    ->setCollector("date-fullyear", belr::make_sfn(&DateTimeNode::setYear))
	    ->setCollector("date-month", belr::make_sfn(&DateTimeNode::setMonth))
	    ->setCollector("date-mday", belr::make_sfn(&DateTimeNode::setDay))
	    ->setCollector("time-hour", belr::make_sfn(&DateTimeNode::setHour))
	    ->setCollector("time-minute", belr::make_sfn(&DateTimeNode::setMinute))
	    ->setCollector("time-second", belr::make_sfn(&DateTimeNode::setSecond))
	    ->setCollector("time-utc", belr::make_sfn(&DateTimeNode::setUtc));

	d->parser->setHandler("dtstval", belr::make_fn(make_shared<DateTimeNode>))
	    ->setCollector("date-fullyear", belr::make_sfn(&DateTimeNode::setYear))
	    ->setCollector("date-month", belr::make_sfn(&DateTimeNode::setMonth))
	    ->setCollector("date-mday", belr::make_sfn(&DateTimeNode::setDay))
	    ->setCollector("time-hour", belr::make_sfn(&DateTimeNode::setHour))
	    ->setCollector("time-minute", belr::make_sfn(&DateTimeNode::setMinute))
	    ->setCollector("time-second", belr::make_sfn(&DateTimeNode::setSecond))
	    ->setCollector("time-utc", belr::make_sfn(&DateTimeNode::setUtc));

	d->parser->setHandler("dur-value", belr::make_fn(make_shared<DurationNode>))
	    ->setCollector("dur-hour", belr::make_sfn(&DurationNode::setHour))
	    ->setCollector("dur-minute", belr::make_sfn(&DurationNode::setMinute))
	    ->setCollector("dur-second", belr::make_sfn(&DurationNode::setSecond));
}

// -----------------------------------------------------------------------------

shared_ptr<Ics::Icalendar> Ics::Parser::parseIcs(const string &input) {
	L_D();

	size_t parsedSize;
	shared_ptr<Node> node = d->parser->parseInput("icalobject", input, &parsedSize);
	if (!node) {
		lWarning() << "Unable to parse ICS message " << input;
		return nullptr;
	}

	shared_ptr<IcalendarNode> icalendarNode = dynamic_pointer_cast<IcalendarNode>(node);
	if (!icalendarNode) {
		lWarning() << "Unable to cast belr result to icalendar node.";
		return nullptr;
	}

	shared_ptr<Icalendar> icalendar = icalendarNode->createIcalendar();

	return icalendar;
}

LINPHONE_END_NAMESPACE
