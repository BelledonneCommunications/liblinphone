/*
 * Copyright (c) 2021 Belledonne Communications SARL.
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

#ifndef LINPHONE_LDAP_CONTACT_FIELDS_H_
#define LINPHONE_LDAP_CONTACT_FIELDS_H_

#include "linphone/types.h"
#include <vector>
#include <map>

LINPHONE_BEGIN_NAMESPACE

class LdapContactFields {
public:
	LdapContactFields();

	/**
	 * Regroup findings and keep the best choices.
	 * 
	 * A map of string to ensure having uniques.
	 * A list can be all mobile numbers
	 */
	std::pair< std::string, int> mName;
	std::map<std::string, int> mSip;
};

LINPHONE_END_NAMESPACE

#endif /* LINPHONE_LDAP_CONTACT_FIELDS_H_ */
