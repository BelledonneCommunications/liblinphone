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
 * but WITHOUT ANY WARRANTY{
}
 without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "conference-info.h"

#include "chat/ics/ics.h"
#include "linphone/api/c-address.h"
#include "linphone/types.h"
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ConferenceInfo::ConferenceInfo () {
}

const IdentityAddress &ConferenceInfo::getOrganizer () const {
	return mOrganizer;
}

void ConferenceInfo::setOrganizer (const IdentityAddress organizer) {
	mOrganizer = organizer;
}

const std::list<IdentityAddress> & ConferenceInfo::getParticipants () const {
	return mParticipants;
}

void ConferenceInfo::setParticipants (const std::list<IdentityAddress> participants) {
	mParticipants = participants;
}

void ConferenceInfo::addParticipant (const IdentityAddress participant) {
	mParticipants.push_back(participant);
}

void ConferenceInfo::removeParticipant (const IdentityAddress participant) {
	const auto it = std::find(mParticipants.cbegin(), mParticipants.cend(), participant);
	if (it == mParticipants.cend()) {
		lInfo() << "Unable to find participant with address " << participant << " in conference info " << this << " (address " << getUri() << ")";
	} else {
		mParticipants.erase(it);
	}
}

const ConferenceAddress &ConferenceInfo::getUri () const {
	return mUri;
}

void ConferenceInfo::setUri (const ConferenceAddress uri) {
	mUri = uri;
}

time_t ConferenceInfo::getDateTime () const {
	return mDateTime;
}

void ConferenceInfo::setDateTime (time_t dateTime) {
	mDateTime = dateTime;
}

unsigned int ConferenceInfo::getDuration () const {
	return mDuration;
}

void ConferenceInfo::setDuration (unsigned int duration) {
	mDuration = duration;
}

const std::string &ConferenceInfo::getSubject () const {
	return mSubject;
}

void ConferenceInfo::setSubject (const std::string &subject) {
	mSubject = Utils::trim(subject);
}

unsigned int ConferenceInfo::getIcsSequence () const {
	return mIcsSequence;
}

void ConferenceInfo::setIcsSequence (unsigned int icsSequence) {
	mIcsSequence = icsSequence;
}

const std::string &ConferenceInfo::getIcsUid () const {
	return mIcsUid;
}

void ConferenceInfo::setIcsUid (const std::string &uid) {
	mIcsUid = Utils::trim(uid);
}

const string &ConferenceInfo::getDescription () const {
	return mDescription;
}

void ConferenceInfo::setDescription (const string &description) {
	mDescription = Utils::trim(description);
}

const ConferenceInfo::State &ConferenceInfo::getState () const {
	return mState;
}

void ConferenceInfo::setState (const ConferenceInfo::State &state) {
	mState = state;
}

const string ConferenceInfo::toIcsString (bool cancel) const {
	Ics::Icalendar cal;

	Ics::Icalendar::Method method = Ics::Icalendar::Method::Request;
	if (cancel) {
		method = Ics::Icalendar::Method::Cancel;
	} else {
		switch (getState()) {
			case ConferenceInfo::State::New:
			case ConferenceInfo::State::Updated:
				method = Ics::Icalendar::Method::Request;
				break;
			case ConferenceInfo::State::Cancelled:
				method = Ics::Icalendar::Method::Cancel;
				break;
		}
	}
	cal.setMethod(method);

	auto event = make_shared<Ics::Event>();
	if (mOrganizer.isValid()) {
		const auto uri = mOrganizer.getAddressWithoutGruu().asString();
		event->setOrganizer(uri);
	}

	event->setSummary(mSubject);
	event->setDescription(mDescription);

	if (mUri.isValid()) {
		const auto uri = mUri.asString();
		event->setXConfUri(uri);
	}
	
	for (const auto & participant : mParticipants) {
		if (participant.isValid()) {
			const auto uri = participant.getAddressWithoutGruu().asString();
			event->addAttendee(uri);
		}
	}

	event->setDateTimeStart(Utils::getTimeTAsTm(mDateTime));

	tm duration = {0};
	duration.tm_hour = mDuration / 60;
	duration.tm_min = mDuration % 60;
	event->setDuration(duration);

	event->setSequence(mIcsSequence);

	if (!mIcsUid.empty()) {
		event->setUid(mIcsUid);
	}

	cal.addEvent(event);

	if (mCreationTime != (time_t) -1) {
		cal.setCreationTime(mCreationTime);
	}
	const auto icsString = cal.asString();

	if (mIcsUid.empty()) {
		mIcsUid = event->getUid();
	}
	if (mIcsSequence == 0) {
		mIcsSequence = event->getSequence();
	}

	return icsString;
}

void ConferenceInfo::setCreationTime(time_t time) {
	mCreationTime = time;
}

LINPHONE_END_NAMESPACE
