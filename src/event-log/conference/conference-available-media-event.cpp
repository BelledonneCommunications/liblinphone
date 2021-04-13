/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
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

#include <map>

#include "conference/conference-enums.h"
#include "conference-notified-event-p.h"
#include "conference-available-media-event.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

class ConferenceAvailableMediaEventPrivate : public ConferenceNotifiedEventPrivate {
public:
	std::map<ConferenceMediaCapabilities, bool> mediaCapabilties;
};

// -----------------------------------------------------------------------------

ConferenceAvailableMediaEvent::ConferenceAvailableMediaEvent (
	time_t creationTime,
	const ConferenceId &conferenceId,
	const std::map<ConferenceMediaCapabilities, bool> mediaCapabilties
) : ConferenceNotifiedEvent(
	*new ConferenceAvailableMediaEventPrivate,
	Type::ConferenceAvailableMediaChanged,
	creationTime,
	conferenceId
) {
	L_D();
	d->mediaCapabilties = mediaCapabilties;
}

const std::map<ConferenceMediaCapabilities, bool> &ConferenceAvailableMediaEvent::getAvailableMediaType() const {
	L_D();
	return d->mediaCapabilties;
}

bool ConferenceAvailableMediaEvent::audioEnabled() const {
	L_D();
	try {
		return d->mediaCapabilties.at(ConferenceMediaCapabilities::Audio);
	} catch (std::out_of_range&) {
		return false;
	}
}

bool ConferenceAvailableMediaEvent::videoEnabled() const {
	L_D();
	try {
		return d->mediaCapabilties.at(ConferenceMediaCapabilities::Video);
	} catch (std::out_of_range&) {
		return false;
	}
}

bool ConferenceAvailableMediaEvent::chatEnabled() const {
	L_D();
	try {
		return d->mediaCapabilties.at(ConferenceMediaCapabilities::Text);
	} catch (std::out_of_range&) {
		return false;
	}
}

LINPHONE_END_NAMESPACE
