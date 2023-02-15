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

#include "conference-info.h"

#include "chat/ics/ics.h"
#include "linphone/api/c-address.h"
#include "linphone/types.h"
#include "private.h"

#include "c-wrapper/c-wrapper.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

const std::string ConferenceInfo::sequenceParam = "X-SEQ";

ConferenceInfo::ConferenceInfo() {
}

const ConferenceInfo::organizer_t &ConferenceInfo::getOrganizer() const {
	return mOrganizer;
}

const std::shared_ptr<Address> &ConferenceInfo::getOrganizerAddress() const {
	return getOrganizer().first;
}

void ConferenceInfo::setOrganizer(const std::shared_ptr<Address> &organizer, const participant_params_t &params) {
	mOrganizer = std::make_pair(Address::create(organizer->getUri()), params);
}

void ConferenceInfo::setOrganizer(const std::shared_ptr<Address> &organizer) {
	ConferenceInfo::participant_params_t params;
	setOrganizer(organizer, params);
}

void ConferenceInfo::addOrganizerParam(const std::string &param, const std::string &value) {
	mOrganizer.second[param] = value;
}

const std::string ConferenceInfo::getOrganizerParam(const std::string &param) const {
	try {
		const auto &params = mOrganizer.second;
		return params.at(param);
	} catch (std::out_of_range &) {
		return std::string();
	}
}

const ConferenceInfo::participant_list_t &ConferenceInfo::getParticipants() const {
	return mParticipants;
}

void ConferenceInfo::setParticipants(const participant_list_t &participants) {
	for (const auto &p : participants) {
		addParticipant(p.first, p.second);
	}
}

void ConferenceInfo::addParticipant(const std::shared_ptr<Address> &participant) {
	ConferenceInfo::participant_params_t params;
	addParticipant(participant, params);
}

void ConferenceInfo::addParticipant(const std::shared_ptr<Address> &participant, const participant_params_t &params) {
	mParticipants.insert(std::make_pair(Address::create(participant->getUri()), params));
}

void ConferenceInfo::removeParticipant(const std::shared_ptr<Address> &participant) {
	auto it = std::find_if(mParticipants.begin(), mParticipants.end(),
	                       [&participant](const auto &p) { return (participant->weakEqual(*p.first)); });
	if (it == mParticipants.cend()) {
		lInfo() << "Unable to find participant with address " << participant << " in conference info " << this
		        << " (address " << *getUri() << ")";
	} else {
		mParticipants.erase(it);
	}
}

void ConferenceInfo::addParticipantParam(const std::shared_ptr<Address> &participant,
                                         const std::string &param,
                                         const std::string &value) {
	auto it = std::find_if(mParticipants.begin(), mParticipants.end(),
	                       [&participant](const auto &p) { return (participant->weakEqual(*p.first)); });
	if (it != mParticipants.end()) {
		auto &params = (*it).second;
		params[param] = value;
	}
}

const std::string ConferenceInfo::getParticipantParam(const std::shared_ptr<Address> &participant,
                                                      const std::string &param) const {
	try {
		auto it = std::find_if(mParticipants.begin(), mParticipants.end(),
		                       [&participant](const auto &p) { return (participant->weakEqual(*p.first)); });
		if (it != mParticipants.cend()) {
			const auto &params = (*it).second;
			return params.at(param);
		}
	} catch (std::out_of_range &) {
	}
	return std::string();
}

bool ConferenceInfo::isValidUri() const {
	return (mUri != nullptr) && mUri->isValid();
}

const std::shared_ptr<Address> &ConferenceInfo::getUri() const {
	return mUri;
}

void ConferenceInfo::setUri(const std::shared_ptr<Address> uri) {
	mUri = Address::create(uri->getUri());
}

time_t ConferenceInfo::getDateTime() const {
	return mDateTime;
}

void ConferenceInfo::setDateTime(time_t dateTime) {
	mDateTime = dateTime;
}

unsigned int ConferenceInfo::getDuration() const {
	return mDuration;
}

void ConferenceInfo::setDuration(unsigned int duration) {
	mDuration = duration;
}

const std::string &ConferenceInfo::getSubject() const {
	return mSubject;
}

const std::string ConferenceInfo::getUtf8Subject() const {
	return Utils::localeToUtf8(mSubject);
}

void ConferenceInfo::setSubject(const std::string &subject) {
	mSubject = Utils::trim(subject);
}

void ConferenceInfo::setUtf8Subject(const std::string &subject) {
	mSubject = Utils::trim(Utils::utf8ToLocale(subject));
}

unsigned int ConferenceInfo::getIcsSequence() const {
	return mIcsSequence;
}

void ConferenceInfo::setIcsSequence(unsigned int icsSequence) {
	mIcsSequence = icsSequence;
}

const std::string ConferenceInfo::getUtf8IcsUid() const {
	return Utils::localeToUtf8(mIcsUid);
}

const std::string &ConferenceInfo::getIcsUid() const {
	return mIcsUid;
}

void ConferenceInfo::setUtf8IcsUid(const std::string &uid) {
	mIcsUid = Utils::trim(Utils::utf8ToLocale(uid));
}

void ConferenceInfo::setIcsUid(const std::string &uid) {
	mIcsUid = Utils::trim(uid);
}

const string ConferenceInfo::getUtf8Description() const {
	return Utils::localeToUtf8(mDescription);
}

const string &ConferenceInfo::getDescription() const {
	return mDescription;
}

void ConferenceInfo::setDescription(const string &description) {
	mDescription = Utils::trim(description);
}

void ConferenceInfo::setUtf8Description(const string &description) {
	mDescription = Utils::trim(Utils::utf8ToLocale(description));
}

const ConferenceInfo::State &ConferenceInfo::getState() const {
	return mState;
}

void ConferenceInfo::setState(const ConferenceInfo::State &state) {
	if (mState != state) {
		lInfo() << "[Conference Info] [" << this << "] moving from state " << mState << " to state " << state;
		mState = state;
	}
}

void ConferenceInfo::updateFrom(const std::shared_ptr<ConferenceInfo> &info) {
	setUri(info->getUri());
	setIcsUid(info->getIcsUid());
	setIcsSequence(info->getIcsSequence() + 1);

	const auto &participants = info->getParticipants();

	for (auto &participant : mParticipants) {
		const auto &otherParticipant =
		    std::find_if(participants.cbegin(), participants.cend(),
		                 [&participant](const auto &p) { return (p.first == participant.first); });

		if (otherParticipant != participants.cend()) {
			participant.second = otherParticipant->second;
		}
	}
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
const string ConferenceInfo::toIcsString(bool cancel, int sequence) const {
#ifdef HAVE_ADVANCED_IM
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
	const auto &organizerAddress = getOrganizerAddress();
	if (organizerAddress && organizerAddress->isValid()) {
		const auto uri = organizerAddress->asStringUriOnly();
		event->setOrganizer(uri, mOrganizer.second);
	}

	event->setSummary(mSubject);
	event->setDescription(mDescription);

	if (mUri && mUri->isValid()) {
		const auto uri = mUri->asStringUriOnly();
		event->setXConfUri(uri);
	}

	for (const auto &participant : mParticipants) {
		const auto &address = participant.first;
		if (address && address->isValid()) {
			const auto uri = address->asStringUriOnly();
			event->addAttendee(uri, participant.second);
		}
	}

	event->setDateTimeStart(Utils::getTimeTAsTm(mDateTime));

	tm duration = {0};
	duration.tm_hour = mDuration / 60;
	duration.tm_min = mDuration % 60;
	event->setDuration(duration);

	event->setSequence((sequence >= 0) ? static_cast<unsigned int>(sequence) : mIcsSequence);

	if (!mIcsUid.empty()) {
		event->setUid(mIcsUid);
	}

	cal.addEvent(event);

	if (mCreationTime != (time_t)-1) {
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
#else
	lWarning() << "No ICS support, ADVANCED_IM is disabled.";
	return string();
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

void ConferenceInfo::setCreationTime(time_t time) {
	mCreationTime = time;
}

const std::string ConferenceInfo::memberParametersToString(const ConferenceInfo::participant_params_t &params) {
	std::string str;
	for (const auto &param : params) {
		if (!str.empty()) {
			str.append(";");
		}
		str.append(param.first + "=" + param.second);
	}
	return str;
}

const ConferenceInfo::participant_params_t ConferenceInfo::stringToMemberParameters(const std::string &paramsString) {
	ConferenceInfo::participant_params_t params;
	if (!paramsString.empty()) {
		const auto &splittedValue = bctoolbox::Utils::split(Utils::trim(paramsString), ";");
		for (const auto &param : splittedValue) {
			auto equal = param.find("=");
			string name = param.substr(0, equal);
			string value = param.substr(equal + 1, param.size());
			params.insert(std::make_pair(name, value));
		}
	}

	return params;
}

std::ostream &operator<<(std::ostream &lhs, ConferenceInfo::State s) {
	switch (s) {
		case ConferenceInfo::State::New:
			return lhs << "New";
		case ConferenceInfo::State::Updated:
			return lhs << "Updated";
		case ConferenceInfo::State::Cancelled:
			return lhs << "Cancelled";
	}
	return lhs;
}

LINPHONE_END_NAMESPACE
