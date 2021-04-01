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

#ifndef _L_MAGIC_SEARCH_ASYNC_DATA_H_
#define _L_MAGIC_SEARCH_ASYNC_DATA_H_

#include <string>
#include <list>
#include <queue>

#include "search-result.h"

LINPHONE_BEGIN_NAMESPACE

class MagicSearch;

class SearchAsyncData{
public:
// This is the final result to use
	std::shared_ptr<std::list<SearchResult>> mSearchResults;
	
// All results stored for each providers
	std::list<std::list<SearchResult> > mProviderResults;

// Callback interface that can be inherit to allow adding more providers
	class CbData{
	public:
		CbData(){
			mEnd = FALSE;
			mTimeout = 5;
		}
		virtual ~CbData();
		virtual void cancel(){}// Cancel transaction.
		static void resultsCb( LinphoneContactSearch* id, bctbx_list_t* friends, void* data );// Callback that is call when a results has to be used
		bool_t mEnd;					// Search is end
		int64_t mTimeout;				// Timeout in seconds
		std::list<SearchResult> *mResult;// Pointer to a result data that is inside parent

// Request
		std::string mFilter;
		std::string mWithDomain;
		const MagicSearch * mParent;// Used to get searchInAddress of parent but may be usefull for anything else if needed
	};

	SearchAsyncData();
	~SearchAsyncData();
// Create a room for results in the main list and return the location
	std::list<SearchResult> *createResult(std::list<SearchResult> data = std::list<SearchResult>());
// Get the current request if there is one in the queue. Return if it is not empty
	bool getCurrentRequest(std::pair<std::string, std::string> * result);
// Remove all request in queue and keep only the last entered
	bool keepOneRequest();
// Add a request in the queue (FIFO): Thread-safe. 
	int pushRequest(const std::pair<std::string, std::string>& request);
// Add a provider in the vector
	void pushData(std::shared_ptr<CbData> data);
// To be call when starting a search for timeout computation
	void initStartTime();
// Get the start time initialized by initStartTime()
	bctoolboxTimeSpec getStartTime() const;
// Set the final search result
	bool setSearchResults(std::shared_ptr<std::list<SearchResult> > resultList);

// Getter for the vector of provider
	const std::vector<std::shared_ptr<CbData> >& getData() const;
	std::vector<std::shared_ptr<CbData> >& getData();
// Clear results and providers
	void clear();
private:
// Queue of requests. Not very useful yet but can be used for historic search
	std::queue<std::pair<std::string, std::string> > mRequests;	// Filter/WithDomain
// Protect the queue for read/write : we can add requests on any threads. All requests are removed from the main iteration.
	ms_mutex_t mLockQueue;// Protect queue
// Start time of the request for timeout computation
	bctoolboxTimeSpec mStartTime;
// Store a vector of providers. By this way, we can use any providers we want on an asynchronous strategy.
	std::vector<std::shared_ptr<CbData> > mProvidersCbData;
	
	
};
LINPHONE_END_NAMESPACE

#endif //_L_MAGIC_SEARCH_ASYNC_DATA_H_
