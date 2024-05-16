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

#ifndef _L_MAGIC_SEARCH_P_H_
#define _L_MAGIC_SEARCH_P_H_

#include "magic-search.h"
#include "object/object-p.h"
#include "search-async-data.h"
#include <vector>

LINPHONE_BEGIN_NAMESPACE

class MagicSearchPrivate : public ObjectPrivate {
private:
	unsigned int mMinWeight = 0;
	unsigned int mMaxWeight = 1000;
	unsigned int mSearchLimit = 30; // Number of ResultSearch maximum when the search is limited
	bool mLimitedSearch = true;     // Limit the search
	int mRequestDelay = 500;        // Delay the first request in ms
	std::string mDelimiter = "+_-"; // Delimiter use for the search
	bool mUseDelimiter = true;
	std::string mFilter;
	bool mAutoResetCache = true; // When a new search start, let MagicSearch to clean its cache

	belle_sip_source_t *mIteration = nullptr;

	std::shared_ptr<std::list<std::shared_ptr<SearchResult>>> mCacheResult;
	SearchAsyncData mAsyncData;

	L_DECLARE_PUBLIC(MagicSearch);
};

LINPHONE_END_NAMESPACE

#endif //_L_MAGIC_SEARCH_P_H_
