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

#ifndef LINPHONE_LDAP_CONTACT_SEARCH_H_
#define LINPHONE_LDAP_CONTACT_SEARCH_H_

#include "linphone/core.h"
#include "linphone/types.h"
#include "linphone/contactprovider.h"
#include "belle-sip/object++.hh"
#include "core/core.h"
#include "core/core-accessor.h"
#include <map>
#include <vector>
#include <string>
#include <list>

#ifdef _WIN32
#	include <winldap.h>
#	include <winber.h>
#else
#	include <ldap.h>
#endif

LINPHONE_BEGIN_NAMESPACE

class LDAPContactProvider;

class LINPHONE_PUBLIC LDAPContactSearch {
public:
	LDAPContactSearch(const int& msgId);
	LDAPContactSearch(LDAPContactProvider * parent, const std::string& predicate, ContactSearchCallback cb, void* cbData);
	virtual ~LDAPContactSearch();
	//static LDAPContactSearch* create(LDAPContactProvider * provider, const std::string& mPredicate, ContactSearchCallback mCb, void* mCbData);
	
	void callCallback();
	
	static int entryCompareWeak(const void*a, const void* b);
	
	int mMsgId;
	std::string mFilter;
	bool_t  complete;
	bctbx_list_t* mFoundEntries;
	unsigned int mFoundCount;
	
private:
	std::string mPredicate;
	ContactSearchCallback mCb;
	void* mCbData;
};

LINPHONE_END_NAMESPACE

#endif /* LINPHONE_LDAP_CONTACT_SEARCH_H_ */
