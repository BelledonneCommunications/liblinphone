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

#include "ldap-contact-search.h"
#include "ldap-contact-provider.h"

#include <algorithm>

#define FILTER_MAX_SIZE      512

LINPHONE_BEGIN_NAMESPACE

LdapContactSearch::LdapContactSearch(const int& msgId){
	mMsgId = msgId;
	mFoundCount = 0;
	mFoundEntries = NULL;
	complete = 0;
}
LdapContactSearch::LdapContactSearch(LdapContactProvider * parent, const std::string& predicate, ContactSearchCallback cb, void* cbData){
	mPredicate = predicate;
	mCb = cb;
	mCbData = cbData;
	mMsgId = 0;
	mFoundCount = 0;
	mFoundEntries = NULL;
	complete = 0;
	char temp[FILTER_MAX_SIZE];
	snprintf(temp, FILTER_MAX_SIZE-1, parent->getFilter().c_str(), predicate.c_str());
	temp[FILTER_MAX_SIZE-1] = '\0';
	if(predicate == "*"){// Full search : replace all "***" by "*"
		mFilter = temp;
		std::string from = "***";
		std::string to = "*";
		size_t start_pos = 0;
		while((start_pos = mFilter.find(from, start_pos)) != std::string::npos) {
			mFilter.replace(start_pos, from.length(), to);
			start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
		}
	}else
		mFilter = temp;
}

static void destroy_address( void* entry ){
	linphone_address_unref((LinphoneAddress*)entry);
}
LdapContactSearch::~LdapContactSearch(){
	if(mFoundEntries)
		bctbx_list_free_with_data(mFoundEntries, destroy_address);
}

void LdapContactSearch::callCallback(){
	mCb(NULL, mFoundEntries, mCbData);
}
LINPHONE_END_NAMESPACE
