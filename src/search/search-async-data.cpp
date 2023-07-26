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
#include "magic-search-p.h"
#include "magic-search.h"

#include "bctoolbox/defs.h"
#include <bctoolbox/list.h>

using namespace std;

LINPHONE_BEGIN_NAMESPACE

SearchAsyncData::CbData::~CbData() {
}

void SearchAsyncData::CbData::resultsCb(BCTBX_UNUSED(LinphoneContactSearch *id),
                                        bctbx_list_t *searchResults,
                                        void *data,
                                        bool_t haveMoreResults) {
	SearchAsyncData::CbData *cbData = (SearchAsyncData::CbData *)data;
	std::list<std::shared_ptr<SearchResult>> results = SearchResult::getCppListFromCList(searchResults);
	for (auto searchResult : results) {
		if (searchResult) {
			if (cbData->mFilter.empty() && cbData->mWithDomain.empty()) {
				searchResult->setWeight(0);
				cbData->mResult->push_back(searchResult);
			} else { // We have constraints : add result with weight
				unsigned int weight = cbData->mParent->searchInAddress(searchResult->getAddress()->toC(),
				                                                       cbData->mFilter, cbData->mWithDomain);
				if (weight >= cbData->mParent->getMinWeight()) {
					searchResult->setWeight(weight);
					cbData->mResult->push_back(searchResult);
				}
			}
		}
	}
	lInfo() << "[Magic Search] Found " << cbData->mResult->size() << " results in LDAP."
	        << (haveMoreResults ? " More results are available." : "");
	cbData->mHaveMoreResults = haveMoreResults;
	cbData->mEnd = TRUE;
}

SearchAsyncData::SearchAsyncData() {
	ms_mutex_init(&mLockQueue, NULL);
	mSearchResults = nullptr;
}
SearchAsyncData::~SearchAsyncData() {
	ms_mutex_destroy(&mLockQueue);
}

std::list<std::shared_ptr<SearchResult>> *SearchAsyncData::createResult(std::list<std::shared_ptr<SearchResult>> data) {
	mProviderResults.push_back(data);
	return &mProviderResults.back();
}

bool SearchAsyncData::getCurrentRequest(SearchRequest *result) {
	bool haveRequest = true;
	ms_mutex_lock(&mLockQueue);
	if (mRequests.size()) *result = mRequests.front();
	else haveRequest = false;
	ms_mutex_unlock(&mLockQueue);
	return haveRequest;
}

const std::list<SearchRequest> &SearchAsyncData::getRequestHistory() const {
	return mRequestHistory;
}

bool SearchAsyncData::keepOneRequest() {
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

void SearchAsyncData::pushData(std::shared_ptr<CbData> data) {
	mProvidersCbData.push_back(data);
}

void SearchAsyncData::initStartTime() {
	bctbx_get_cur_time(&mStartTime);
	ms_mutex_lock(&mLockQueue);
	mRequests.front().initStartTime();
	mRequestHistory.push_back(mRequests.front());
	ms_mutex_unlock(&mLockQueue);
}

bctoolboxTimeSpec SearchAsyncData::getStartTime() const {
	return mStartTime;
}
const std::vector<std::shared_ptr<SearchAsyncData::CbData>> &SearchAsyncData::getData() const {
	return mProvidersCbData;
}
std::vector<std::shared_ptr<SearchAsyncData::CbData>> &SearchAsyncData::getData() {
	return mProvidersCbData;
}

void SearchAsyncData::clear() {
	mProvidersCbData.clear();
	mProviderResults.clear();
}

void SearchAsyncData::setSearchRequest(const SearchRequest &request) {
	mSearchRequest = request;
}

bool SearchAsyncData::setSearchResults(std::shared_ptr<list<std::shared_ptr<SearchResult>>> resultList) {
	mSearchResults = resultList;
	return mSearchResults != nullptr;
}

LINPHONE_END_NAMESPACE
