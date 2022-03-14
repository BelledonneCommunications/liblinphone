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
#include <bctoolbox/utils.hh>

#define FILTER_MAX_SIZE      512

LINPHONE_BEGIN_NAMESPACE

LdapContactSearch::LdapContactSearch(const int& msgId){
	mMsgId = msgId;
	mFoundCount = 0;
	complete = 0;
	mHaveMoreResults = FALSE;
}

LdapContactSearch::LdapContactSearch(LdapContactProvider * parent, std::string predicate, ContactSearchCallback cb, void* cbData){
	mPredicate = predicate; // Save original predicate
	mCb = cb;
	mCbData = cbData;
	mMsgId = 0;
	mFoundCount = 0;
	complete = 0;
	mHaveMoreResults = 0;
	
// Replace specials characters first : manual characters should be encoded
	bctoolbox::Utils::replace( predicate, "\\", "\\5c");
	bctoolbox::Utils::replace( predicate, "*", "\\2a");
	bctoolbox::Utils::replace( predicate, "(", "\\28");
	bctoolbox::Utils::replace( predicate, ")", "\\29");
	bctoolbox::Utils::replace( predicate, "/", "\\2f");
	
// Replace space characters into wild characters
	std::replace( predicate.begin(), predicate.end(), ' ', '*');
// Apply predicate into requested filter
	char temp[FILTER_MAX_SIZE];
	snprintf(temp, FILTER_MAX_SIZE-1, parent->getFilter().c_str(), predicate.c_str());
	temp[FILTER_MAX_SIZE-1] = '\0';
	
// Replace all '**' by '*' in filter.
	mFilter = temp;
	bctoolbox::Utils::replace( mFilter, "**", "*", false);// Do not step as replacement can still contain double stars.
}

LdapContactSearch::~LdapContactSearch(){
}

void LdapContactSearch::callCallback(){
	bctbx_list_t* results = SearchResult::getCListFromCppList(mFoundEntries);
	mCb(NULL, results, mCbData, mHaveMoreResults);
	if (results) {
		bctbx_list_free_with_data(results, (bctbx_list_free_func)linphone_search_result_unref);
	}
}
LINPHONE_END_NAMESPACE
