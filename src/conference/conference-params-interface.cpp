/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#include <iostream>

#include "conference/conference-params-interface.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

std::ostream &operator<<(std::ostream &lhs, ConferenceParamsInterface::SecurityLevel e) {
	switch (e) {
		case ConferenceParamsInterface::SecurityLevel::None:
			lhs << "none";
			break;
		case ConferenceParamsInterface::SecurityLevel::PointToPoint:
			lhs << "point-to-point";
			break;
		case ConferenceParamsInterface::SecurityLevel::EndToEnd:
			lhs << "end-to-end";
			break;
	}
	return lhs;
}

std::string operator+(const std::string &str, ConferenceParamsInterface::SecurityLevel level) {
	std::string s(str);
	switch (level) {
		case ConferenceParamsInterface::SecurityLevel::None:
			s.append("none");
			break;
		case ConferenceParamsInterface::SecurityLevel::PointToPoint:
			s.append("point-to-point");
			break;
		case ConferenceParamsInterface::SecurityLevel::EndToEnd:
			s.append("end-to-end");
			break;
	}
	return s;
}

LINPHONE_END_NAMESPACE
