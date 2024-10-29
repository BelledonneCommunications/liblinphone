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

#include "search-async-data.h"
#include "logger/logger.h"
#include "magic-search.h"

#include "bctoolbox/defs.h"
#include <bctoolbox/list.h>

using namespace std;

LINPHONE_BEGIN_NAMESPACE

SearchAsyncData::SearchAsyncData() {
	ms_mutex_init(&mLockQueue, NULL);
}

SearchAsyncData::~SearchAsyncData() {
	ms_mutex_destroy(&mLockQueue);
}

bool SearchAsyncData::getCurrentRequest(SearchRequest *result) {
	bool haveRequest = true;
	ms_mutex_lock(&mLockQueue);
	if (mRequests.size()) *result = mRequests.front();
	else haveRequest = false;
	ms_mutex_unlock(&mLockQueue);
	return haveRequest;
}

bool SearchAsyncData::isRequestPending() {
	bool haveRequest;
	ms_mutex_lock(&mLockQueue);
	do {
		mRequests.pop();
	} while (mRequests.size() > 1);
	haveRequest = mRequests.size() > 0;
	ms_mutex_unlock(&mLockQueue);
	return haveRequest;
}

int SearchAsyncData::pushRequest(const SearchRequest &request) {
	int currentSize = (int)mRequests.size() + 1;
	ms_mutex_lock(&mLockQueue);
	mRequests.push(request);
	ms_mutex_unlock(&mLockQueue);
	return currentSize;
}

void SearchAsyncData::initStartTime() {
	bctbx_get_cur_time(&mStartTime);
	ms_mutex_lock(&mLockQueue);
	mRequests.front().initStartTime();
	mRequestHistory.push_back(mRequests.front());
	ms_mutex_unlock(&mLockQueue);
}

void SearchAsyncData::clear() {
	mSearchResults.clear();
}

bool SearchAsyncData::setSearchResults(list<shared_ptr<SearchResult>> &resultList) {
	mSearchResults = resultList;
	return !mSearchResults.empty();
}

void SearchAsyncData::addToResults(list<shared_ptr<SearchResult>> &data) {
	if (mSearchResults.empty()) {
		mSearchResults = data;
	} else {
		mSearchResults.splice(mSearchResults.end(), data);
	}
}

LINPHONE_END_NAMESPACE
