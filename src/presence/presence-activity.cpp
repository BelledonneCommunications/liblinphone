/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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
#include <array>

#include <bctoolbox/defs.h>

#include "linphone/types.h"
#include "presence-activity.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

struct PresenceActivityNameMap {
	std::string_view name;
	LinphonePresenceActivityType type;
};

static constexpr std::array<PresenceActivityNameMap, 27> presenceActivityNameMap{
    {{"appointment", LinphonePresenceActivityAppointment},
     {"away", LinphonePresenceActivityAway},
     {"breakfast", LinphonePresenceActivityBreakfast},
     {"busy", LinphonePresenceActivityBusy},
     {"dinner", LinphonePresenceActivityDinner},
     {"holiday", LinphonePresenceActivityHoliday},
     {"in-transit", LinphonePresenceActivityInTransit},
     {"looking-for-work", LinphonePresenceActivityLookingForWork},
     {"lunch", LinphonePresenceActivityLunch},
     {"meal", LinphonePresenceActivityMeal},
     {"meeting", LinphonePresenceActivityMeeting},
     {"on-the-phone", LinphonePresenceActivityOnThePhone},
     {"other", LinphonePresenceActivityOther},
     {"performance", LinphonePresenceActivityPerformance},
     {"permanent-absence", LinphonePresenceActivityPermanentAbsence},
     {"playing", LinphonePresenceActivityPlaying},
     {"presentation", LinphonePresenceActivityPresentation},
     {"shopping", LinphonePresenceActivityShopping},
     {"sleeping", LinphonePresenceActivitySleeping},
     {"spectator", LinphonePresenceActivitySpectator},
     {"steering", LinphonePresenceActivitySteering},
     {"travel", LinphonePresenceActivityTravel},
     {"tv", LinphonePresenceActivityTV},
     {"unknown", LinphonePresenceActivityUnknown},
     {"vacation", LinphonePresenceActivityVacation},
     {"working", LinphonePresenceActivityWorking},
     {"worship", LinphonePresenceActivityWorship}}};

// -----------------------------------------------------------------------------

PresenceActivity::PresenceActivity(LinphonePresenceActivityType activityType, const std::string &description) {
	mType = activityType;
	mDescription = description;
}

PresenceActivity *PresenceActivity::clone() const {
	return nullptr;
}

std::string PresenceActivity::toString() const {
	std::stringstream ss;
	ss << PresenceActivity::activityTypeToName(mType);
	if (!mDescription.empty()) ss << ": " << mDescription;
	return ss.str();
}

// -----------------------------------------------------------------------------

void PresenceActivity::setDescription(const std::string &description) {
	mDescription = description;
}

void PresenceActivity::setType(LinphonePresenceActivityType activityType) {
	mType = activityType;
}

// -----------------------------------------------------------------------------

const std::string &PresenceActivity::getDescription() const {
	return mDescription;
}

LinphonePresenceActivityType PresenceActivity::getType() const {
	return mType;
}

// -----------------------------------------------------------------------------

LinphoneStatus PresenceActivity::activityNameToType(const std::string &name,
                                                    LinphonePresenceActivityType *activityType) {
	const auto found = std::find_if(begin(presenceActivityNameMap), end(presenceActivityNameMap),
	                                [&](const auto &elem) { return elem.name == name; });
	if (found == end(presenceActivityNameMap)) return -1;
	*activityType = found->type;
	return 0;
}

std::string PresenceActivity::activityTypeToName(LinphonePresenceActivityType activityType) {
	const auto found = std::find_if(begin(presenceActivityNameMap), end(presenceActivityNameMap),
	                                [&](const auto &elem) { return elem.type == activityType; });
	if (found == end(presenceActivityNameMap)) return std::string();
	return std::string(found->name);
}

bool PresenceActivity::isValidActivityName(const std::string &name) {
	return end(presenceActivityNameMap) != std::find_if(begin(presenceActivityNameMap), end(presenceActivityNameMap),
	                                                    [&](const auto &elem) { return elem.name == name; });
}

#ifdef HAVE_XML2
int PresenceActivity::toXml(xmlTextWriterPtr writer) const {
	int err = xmlTextWriterStartElementNS(writer, (const xmlChar *)"rpid",
	                                      (const xmlChar *)L_STRING_TO_C(activityTypeToName(getType())), nullptr);
	if ((err >= 0) && !getDescription().empty()) {
		err = xmlTextWriterWriteString(writer, (const xmlChar *)L_STRING_TO_C(getDescription()));
	}
	if (err >= 0) {
		err = xmlTextWriterEndElement(writer);
	}
	return err;
}
#endif /* HAVE_XML2 */

LINPHONE_END_NAMESPACE
