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

#ifndef LINPHONE_LDAP_FRIEND_H_
#define LINPHONE_LDAP_FRIEND_H_

#include "linphone/types.h"

LINPHONE_BEGIN_NAMESPACE

class LDAPFriend {
public:
	LDAPFriend();
// From a request, the index is the index that matches to the attribute vector. It is used for giving priority on attributes (eg: mobile and then telephoneNumber if first is not good enough to take account)
	std::string mName;
	int mNameIndex;
	std::string mSip;
	int mSipIndex;
};

LINPHONE_END_NAMESPACE

#endif /* LINPHONE_LDAP_FRIEND_H_ */
