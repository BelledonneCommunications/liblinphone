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

#include "search-async-data.h"
#include "magic-search.h"
#include "magic-search-p.h"

#include <bctoolbox/list.h>

using namespace std;

LINPHONE_BEGIN_NAMESPACE

SearchAsyncData::CbData::~CbData(){
}

void SearchAsyncData::CbData::resultsCb( LinphoneContactSearch* id, bctbx_list_t* friends, void* data ){
	SearchAsyncData::CbData * cbData = (SearchAsyncData::CbData*)data;
	for (const bctbx_list_t *f = friends ; f != nullptr ; f = bctbx_list_next(f)) {
		LinphoneAddress *addr = reinterpret_cast<LinphoneAddress*>(f->data);
		if (addr) {
			if (cbData->mFilter.empty() && cbData->mWithDomain.empty()) {
				cbData->mResult->push_back(SearchResult(0, addr, "", nullptr));
			} else {// We have constraints : add result with weight
				unsigned int weight = cbData->mParent->searchInAddress(addr, cbData->mFilter, cbData->mWithDomain);
				if( weight > cbData->mParent->getMinWeight())
					cbData->mResult->push_back(SearchResult(weight, addr, "", nullptr));
			}
		}
	}
	cbData->mEnd = TRUE;
}

SearchAsyncData::SearchAsyncData(){
	ms_mutex_init(&mLockQueue, NULL);
	mSearchResults = nullptr;
}
SearchAsyncData::~SearchAsyncData(){
	ms_mutex_destroy(&mLockQueue);
}

std::list<SearchResult> *SearchAsyncData::createResult(std::list<SearchResult> data){
	mProviderResults.push_back(data);
	return &mProviderResults.back();
}

bool SearchAsyncData::getCurrentRequest(std::pair<std::string, std::string> * result){
	bool haveRequest = true;
	ms_mutex_lock(&mLockQueue);
	if( mRequests.size())
		*result = mRequests.front();
	else
		haveRequest = false;
	ms_mutex_unlock(&mLockQueue);
	return haveRequest;
}

bool SearchAsyncData::keepOneRequest(){
	bool haveRequest;
	ms_mutex_lock(&mLockQueue);
	do{
		mRequests.pop();
	}while(mRequests.size() > 1);
	haveRequest = mRequests.size() >0;
	ms_mutex_unlock(&mLockQueue);
	return haveRequest;
}

int SearchAsyncData::pushRequest(const std::pair<std::string, std::string>& request){
	int currentSize = (int)mRequests.size()+1;
	ms_mutex_lock(&mLockQueue);
	mRequests.push(request);
	ms_mutex_unlock(&mLockQueue);
	return currentSize;
}

void SearchAsyncData::pushData(std::shared_ptr<CbData> data){
	mProvidersCbData.push_back(data);
}
		
void SearchAsyncData::initStartTime(){
	bctbx_get_cur_time(&mStartTime);
}

bctoolboxTimeSpec SearchAsyncData::getStartTime() const{
	return mStartTime;
}
const std::vector<std::shared_ptr<SearchAsyncData::CbData> >& SearchAsyncData::getData() const{
	return mProvidersCbData;
}
std::vector<std::shared_ptr<SearchAsyncData::CbData> >& SearchAsyncData::getData(){
	return mProvidersCbData;
}

void SearchAsyncData::clear(){
	mProvidersCbData.clear();
	mProviderResults.clear();
}

bool SearchAsyncData::setSearchResults(std::shared_ptr<list<SearchResult> > resultList){
	mSearchResults = resultList;
	return mSearchResults != nullptr;
}

LINPHONE_END_NAMESPACE
