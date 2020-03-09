/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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

#ifndef linphone_if_addrs_h
#define linphone_if_addrs_h

#include <list>
#include <string>

LINPHONE_BEGIN_NAMESPACE

class IfAddrs{
public:
	static std::list<std::string> fetchLocalAddresses();
private:
	static std::list<std::string> fetchWithGetIfAddrs();
	static std::list<std::string> fetchWithGetAdaptersAddresses();
};

LINPHONE_END_NAMESPACE

#endif
