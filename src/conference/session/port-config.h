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

#ifndef _L_PORT_CONFIG_H_
#define _L_PORT_CONFIG_H_

#include <string>

#include "linphone/utils/general.h"
#include "c-wrapper/internal/c-sal.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

struct PortConfig {
	SalMulticastRole multicastRole = SalMulticastInactive;
	std::string multicastIp;
	std::string multicastBindIp;
	int rtpPort = -1;
	int rtcpPort = -1;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_PORT_CONFIG_H_
