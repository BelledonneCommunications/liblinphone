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

#include "conference/conference-interface.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

std::ostream& operator<<(std::ostream& lhs, ConferenceInterface::State e) {
	switch(e) {
		case ConferenceInterface::State::None: lhs << "None"; break;
		case ConferenceInterface::State::Instantiated: lhs << "Instantiated"; break;
		case ConferenceInterface::State::CreationPending: lhs << "CreationPending"; break;
		case ConferenceInterface::State::Created: lhs << "Created"; break;
		case ConferenceInterface::State::CreationFailed: lhs << "CreationFailed"; break;
		case ConferenceInterface::State::TerminationPending: lhs << "TerminationPending"; break;
		case ConferenceInterface::State::Terminated: lhs << "Terminated"; break;
		case ConferenceInterface::State::TerminationFailed: lhs << "TerminationFailed"; break;
		case ConferenceInterface::State::Deleted: lhs << "Deleted"; break;
	}
	return lhs;
}

LINPHONE_END_NAMESPACE
