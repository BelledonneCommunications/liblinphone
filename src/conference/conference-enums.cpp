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

#include <iostream>

#include "conference/conference-enums.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

std::ostream &operator<<(std::ostream &lhs, ConferenceMediaCapabilities e) {
	switch (e) {
		case ConferenceMediaCapabilities::Audio:
			lhs << "Audio";
			break;
		case ConferenceMediaCapabilities::Video:
			lhs << "Video";
			break;
		case ConferenceMediaCapabilities::Text:
			lhs << "Text";
			break;
	}
	return lhs;
}

std::ostream &operator<<(std::ostream &str, ConferenceLayout layout) {
	switch (layout) {
		case ConferenceLayout::Grid:
			str << "Grid";
			break;
		case ConferenceLayout::ActiveSpeaker:
			str << "ActiveSpeaker";
			break;
	}
	return str;
}

std::string operator+(const std::string &str, ConferenceLayout layout) {
	std::string s(str);
	switch (layout) {
		case ConferenceLayout::Grid:
			s.append("Grid");
			break;
		case ConferenceLayout::ActiveSpeaker:
			s.append("ActiveSpeaker");
			break;
	}
	return s;
}

LINPHONE_END_NAMESPACE
