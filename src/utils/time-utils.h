/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#ifndef _L_APPLE_TIME_UTILS_H_
#define _L_APPLE_TIME_UTILS_H_

#include <string>
#include <time.h>

#include "linphone/utils/general.h"

LINPHONE_BEGIN_NAMESPACE

#ifdef __APPLE__
time_t iso8601ToTimeApple(std::string iso8601DateTime);
std::string timeToIso8601Apple(time_t t);
#endif // __APPLE__

LINPHONE_END_NAMESPACE

#endif // _L_APPLE_TIME_UTILS_H_
