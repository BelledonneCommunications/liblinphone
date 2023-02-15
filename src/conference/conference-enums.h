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

#ifndef CONFERENCE_ENUMS_H
#define CONFERENCE_ENUMS_H

#include "linphone/enums/conference-enums.h"
#include "linphone/utils/general.h"
#include <string>

LINPHONE_BEGIN_NAMESPACE

/**
 * Conference media capabilities.
 * MediaCapabilities is used to index participant and media capabilities.
 */
enum class ConferenceMediaCapabilities {
	Audio = 0, // Audio text capabilities
	Video = 1, // Video capabilities
	Text = 2   // Text capabilities
};

std::ostream &operator<<(std::ostream &lhs, ConferenceMediaCapabilities e);

/**
 * Conference layout
 */
enum class ConferenceLayout {
	ActiveSpeaker =
	    LinphoneConferenceLayoutActiveSpeaker, /**< Active speaker - participant who speaks is prominently displayed in
	                                              the center of the screen and other participants are minimized */
	Grid = LinphoneConferenceLayoutGrid,       /**< Grid - each participant is given an equal sized image size */
};

std::ostream &operator<<(std::ostream &str, ConferenceLayout layout);
std::string operator+(const std::string &str, ConferenceLayout layout);

LINPHONE_END_NAMESPACE

#endif // ifndef CONFERENCE_ENUMS_H
