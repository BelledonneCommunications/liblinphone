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

ConferenceInfo::~ConferenceInfo () {
	if (mOrganizer) linphone_address_unref(mOrganizer);
	if (mParticipants) bctbx_list_free_with_data(mParticipants, (bctbx_list_free_func) linphone_address_unref);
	if (mUri) linphone_address_unref(mUri);
}

const LinphoneAddress *ConferenceInfo::getOrganizer () const {
	return mOrganizer;
}

void ConferenceInfo::setOrganizer (LinphoneAddress *organizer) {
	if (mOrganizer) linphone_address_unref(mOrganizer);

	mOrganizer = organizer ? linphone_address_clone(organizer) : nullptr;
}

const bctbx_list_t *ConferenceInfo::getParticipants () const {
	return mParticipants;
}

void ConferenceInfo::setParticipants (bctbx_list_t *participants) {
	if (mParticipants) {
		bctbx_list_free_with_data(mParticipants, (bctbx_list_free_func) linphone_address_unref);
	}

	mParticipants = participants;
}

void ConferenceInfo::addParticipant (LinphoneAddress *participant) {
	mParticipants = bctbx_list_append(mParticipants, linphone_address_clone(participant));
}

const LinphoneAddress *ConferenceInfo::getUri () const {
	return mUri;
}

void ConferenceInfo::setUri (LinphoneAddress *uri) {
	if (mUri) linphone_address_unref(mUri);

	mUri = uri ? linphone_address_clone(uri) : nullptr;
}

time_t ConferenceInfo::getDateTime () const {
	return mDateTime;
}

void ConferenceInfo::setDateTime (time_t dateTime) {
	mDateTime = dateTime;
}

int ConferenceInfo::getDuration () const {
	return mDuration;
}

void ConferenceInfo::setDuration (int duration) {
	mDuration = duration;
}

const std::string &ConferenceInfo::getSubject () const {
	return mSubject;
}

void ConferenceInfo::setSubject (const std::string &subject) {
	mSubject = subject;
}

const string &ConferenceInfo::getDescription () const {
	return mDescription;
}

void ConferenceInfo::setDescription (const string &description) {
	mDescription = description;
}

const string ConferenceInfo::toIcsString () const {
	Ics::Icalendar cal;
	auto event = make_shared<Ics::Event>();

	char *tmp = linphone_address_as_string_uri_only(mOrganizer);
	event->setOrganizer(tmp);
	bctbx_free(tmp);

	event->setSummary(mSubject);
	event->setDescription(mDescription);

	tmp = linphone_address_as_string_uri_only(mUri);
	event->setXConfUri(tmp);
	bctbx_free(tmp);
	
	bctbx_list_t *it;
	for (it = mParticipants; it != NULL; it = it->next) {
		tmp = linphone_address_as_string_uri_only((LinphoneAddress *) bctbx_list_get_data(it));
		event->addAttendee(tmp);
		bctbx_free(tmp);
	}

	event->setDateTimeStart(Utils::getTimeTAsTm(mDateTime));

	tm duration = {0};
	duration.tm_hour = mDuration / 60;
	duration.tm_min = mDuration % 60;
	event->setDuration(duration);

	cal.addEvent(event);

	return cal.asString();
}

LINPHONE_END_NAMESPACE
