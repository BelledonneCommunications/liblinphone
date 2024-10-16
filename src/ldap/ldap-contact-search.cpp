/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#include "ldap-contact-search.h"
#include "ldap-contact-provider.h"

#include <algorithm>
#include <bctoolbox/utils.hh>

#define FILTER_MAX_SIZE 512

LINPHONE_BEGIN_NAMESPACE

LdapContactSearch::LdapContactSearch(const int &msgId) {
	mMsgId = msgId;
	mFoundCount = 0;
	complete = 0;
	mHaveMoreResults = false;
}

LdapContactSearch::LdapContactSearch(LdapContactProvider *parent,
                                     std::string predicate,
                                     MagicSearchCallback cb,
                                     void *cbData) {
	mPredicate = predicate; // Save original predicate
	mCb = cb;
	mCbData = cbData;
	mMsgId = 0;
	mFoundCount = 0;
	complete = 0;
	mHaveMoreResults = 0;

	// Replace specials characters first : manual characters should be encoded
	bctoolbox::Utils::replace(predicate, "\\", "\\5c");
	bctoolbox::Utils::replace(predicate, "*", "\\2a");
	bctoolbox::Utils::replace(predicate, "(", "\\28");
	bctoolbox::Utils::replace(predicate, ")", "\\29");
	bctoolbox::Utils::replace(predicate, "/", "\\2f");

	// Replace space characters into wild characters
	std::replace(predicate.begin(), predicate.end(), ' ', '*');
	mFilter = parent->configValueToStr("filter");
	bctoolbox::Utils::replace(mFilter, "%s", predicate, false);

	// Replace all '**' by '*' in filter.
	bctoolbox::Utils::replace(mFilter, "**", "*", false); // Do not step as replacement can still contain double stars.

	if (!mFilter.empty() && mFilter[0] != '(') {
		mFilter.insert(0, 1, '(');
		mFilter.push_back(')');
	}
}

LdapContactSearch::~LdapContactSearch() {
}

void LdapContactSearch::callCallback() {
	mCb(mFoundEntries, mCbData, mHaveMoreResults);
}
LINPHONE_END_NAMESPACE
