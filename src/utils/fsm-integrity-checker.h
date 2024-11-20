/*
 * Copyright (c) 2024 Belledonne Communications SARL.
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

#ifndef fsm_integrity_h
#define fsm_integrity_h

#include "linphone/utils/general.h"

#include <map>

LINPHONE_BEGIN_NAMESPACE

/* Template class to ease integrity checking of finite state machines.
 * It has to be brakets initialized, so that all allowed transitions are listed, for example:
 * FsmIntegrityChecker<State> fsmChecker {
 * 		{
 * 			{StateA, {StateB, StateC}},
 * 			{StateB, {StateC}},
 * 			{StateC, {}}
 * 		}
 * }
 * Then isValid() member function can be used to verify that a transition is valid.
 * If not, an error log is printed (if << operator is provided for the enum type it's better),
 * and false is returned.
 */
template <typename _enumT>
class FsmIntegrityChecker {
public:
	std::map<_enumT, std::set<_enumT>> mTransitions;
	bool isValid(_enumT origin, _enumT destination) const {
		auto srcIt = mTransitions.find(origin);
		if (srcIt == mTransitions.end()) {
			lError() << "FsmIntegrityChecker: Invalid origin state" << origin;
			return false;
		}
		auto destIt = (*srcIt).second.find(destination);
		if (destIt == (*srcIt).second.end()) {
			if (origin == destination) {
				lDebug() << "FsmIntegrityChecker: invalid state transition from [" << origin << "] to [" << destination
				         << "]";
			} else {
				lError() << "FsmIntegrityChecker: invalid state transition from [" << origin << "] to [" << destination
				         << "]";
			}
			return false;
		}
		return true;
	}
};

LINPHONE_END_NAMESPACE

#endif
