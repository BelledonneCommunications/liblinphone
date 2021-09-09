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

#include <set>

#include <belr/abnf.h>
#include <belr/grammarbuilder.h>
#include "bctoolbox/utils.hh"

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
	string IcsGrammar("ics_grammar");
}

namespace Ics {
	class Node {
	public:
		virtual ~Node () = default;
	};

	class DateTimeNode : public Node {
	public:
		DateTimeNode () = default;

		const tm getDateStart () {
			tm time = {0};
			time.tm_year = mYear;
			time.tm_mon = mMonth - 1;
			time.tm_mday = mDay;
			time.tm_hour = mHour;
			time.tm_min = mMinute;
			time.tm_sec = mSecond;

			return time;
		}

		void setYear (const string &year) {
			mYear = Utils::stoi(year);
		}

		void setMonth (const string &month) {
			mMonth = Utils::stoi(month);
		}

		void setDay (const string &day) {
			mDay = Utils::stoi(day);
		}

		void setHour (const string &hour) {
			mHour = Utils::stoi(hour);
		}

		void setMinute (const string &minute) {
			mMinute = Utils::stoi(minute);
		}

		void setSecond (const string &second) {
			mSecond = Utils::stoi(second);
		}

		void setUtc (const string &utc) {
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
		DurationNode () = default;

		tm getDuration () {
			tm duration = {0};

			duration.tm_hour = mHour;
			duration.tm_min = mMinute;
			duration.tm_sec = mSecond;
			
			return duration;
		}

		void setHour (const string &hour) {
			if (hour.empty()) return;

			// It can also contains minutes and seconds so remove all after 'H'
			size_t p = hour.find("H");
			if (p != string::npos) {
				string tmp = hour.substr(0, p - 1);
				mHour = Utils::stoi(hour);
			}
		}

		void setMinute (const string &minute) {
			if (minute.empty()) return;

			// It can also contains seconds so remove all after 'M'
			size_t p = minute.find("M");
			if (p != string::npos) {
				string tmp = minute.substr(0, p - 1);
				mMinute = Utils::stoi(minute);
			}
		}

		void setSecond (const string &second) {
			string tmp = second.substr(0, second.size() - 1); // Removes "S"
			mSecond = Utils::stoi(second);
		}

	private:
		int mHour;
		int mMinute;
		int mSecond;
	};

	class EventNode : public Node {
	public:
		EventNode () = default;

		void setSummary (const string &summary) {
			mSummary = summary;
		}

		void setDescription (const string &description) {
			mDescription = description;
		}

		void setXProp (const string &xProp) {
			if (xProp.empty()) return;

			// Check if the prop is X-CONFURI
			size_t p = xProp.find(":");
			if (p != string::npos) {
				string name = xProp.substr(0, p - 1);
				string value = xProp.substr(p + 1, xProp.size());

				p = name.find(";");
				if (p != string::npos) {
					name = name.substr(0, p - 1);
				}

				if (name == "X-CONFURI") {
					mXConfUri = value;
				}
			}
		}

		void setOrganizer (const string &organizer) {
			mOrganizer = organizer;
		}

		void addAttendee (const string &attendee) {
			mAttendees.push_back(attendee);
		}

		void setDateStart (const shared_ptr<DateTimeNode> &dateStart) {
			mDateStart = dateStart;
		}

		void setDuration (const shared_ptr<DurationNode> &duration) {
			mDuration = duration;
		}

		shared_ptr<Event> createEvent () {
			auto event = make_shared<Event>();

			event->setSummary(mSummary);
			event->setDescription(mDescription);
			if (mDateStart) event->setDateTimeStart(mDateStart->getDateStart());
			if (mDuration) event->setDuration(mDuration->getDuration());
			event->setOrganizer(mOrganizer);
			event->setXConfUri(mXConfUri);

			for (const auto &attendee : mAttendees) {
				event->addAttendee(attendee);
			}

			return event;
		}

	private:
		string mSummary;
		string mDescription;
		string mXConfUri;
		string mOrganizer;
		list<string> mAttendees;
		shared_ptr<DateTimeNode> mDateStart;
		shared_ptr<DurationNode> mDuration;
	};

	class IcalendarNode : public Node {
	public:
		IcalendarNode () = default;

		void addEvent (const shared_ptr<EventNode> &event) {
			mEvents.push_back(event);
		}

		shared_ptr<Icalendar> createIcalendar () {
			if (mEvents.empty()) return nullptr;

			auto calendar = make_shared<Icalendar>();

			// An Ics file can have multiple Events but we only use one
			calendar->addEvent(mEvents.front()->createEvent());

			return calendar;
		}

	private:
		list<shared_ptr<EventNode>> mEvents;
	};
}

// -----------------------------------------------------------------------------

class Ics::ParserPrivate : public ObjectPrivate {
public:
	shared_ptr<belr::Parser<shared_ptr<Node> >> parser;
};

Ics::Parser::Parser () : Singleton(*new ParserPrivate) {
	L_D();
	
	shared_ptr<belr::Grammar> grammar = belr::GrammarLoader::get().load(IcsGrammar);
	if (!grammar)
		lFatal() << "Unable to load CPIM grammar.";
	d->parser = make_shared<belr::Parser<shared_ptr<Node>>>(grammar);
	
	d->parser->setHandler("icalobject", belr::make_fn(make_shared<IcalendarNode>))
		->setCollector("eventc", belr::make_sfn(&IcalendarNode::addEvent));

	d->parser->setHandler("eventc", belr::make_fn(make_shared<EventNode>))
		->setCollector("summvalue", belr::make_sfn(&EventNode::setSummary))
		->setCollector("descvalue", belr::make_sfn(&EventNode::setDescription))
		->setCollector("dtstval", belr::make_sfn(&EventNode::setDateStart))
		->setCollector("dur-value", belr::make_sfn(&EventNode::setDuration))
		->setCollector("orgvalue", belr::make_sfn(&EventNode::setOrganizer))
		->setCollector("attvalue", belr::make_sfn(&EventNode::addAttendee))
		->setCollector("x-prop", belr::make_sfn(&EventNode::setXProp));

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

shared_ptr<Ics::Icalendar> Ics::Parser::parseIcs (const string &input) {
	L_D();

	size_t parsedSize;
	shared_ptr<Node> node = d->parser->parseInput("icalobject", input, &parsedSize);
	if (!node) {
		lWarning() << "Unable to parse message.";
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
