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
	/**
	 * @brief mSearchResults This is the final result to use.
	 */
	std::shared_ptr<std::list<SearchResult>> mSearchResults;

	/**
	 * @brief mProviderResults All results stored for each providers.
	 */
	std::list<std::list<SearchResult> > mProviderResults;

	/**
	 * @brief The CbData class. Callback interface that can be inherit to allow adding more providers.
	 */
	class CbData{
	public:
		CbData(){
			mEnd = FALSE;
			mTimeout = 5;// 5s is the default
		}
		virtual ~CbData();

		/**
		 * @brief cancel Cancel transaction.
		 */
		virtual void cancel(){}

		/**
		 * @brief resultsCb Callback that is call when a result has to be used.
		 * By default, it will set mEnd to true on the first call of this callback.
		 * @param id The current search request.
		 * @param friends List of address as results. Data depends of the provider.
		 * @param data User data coming from Callback binding.
		 */
		static void resultsCb( LinphoneContactSearch* id, bctbx_list_t* friends, void* data );

		/**
		 * @brief mEnd Search is over.
		 */
		bool_t mEnd;

		/**
		 * @brief mTimeout Timeout in seconds.
		 */
		int64_t mTimeout;

		/**
		 * @brief mResult Pointer to a result data that is inside parent.
		 */
		std::list<SearchResult> *mResult;

		/**
		 * @brief mFilter Filter Request
		 */		
		std::string mFilter;
		/**
		 * @brief mWithDomain Domain Request
		 */
		std::string mWithDomain;
		/**
		 * @brief mParent Used to get searchInAddress of parent but may be usefull for anything else if needed.
		 */
		const MagicSearch * mParent;
	};

	SearchAsyncData();
	~SearchAsyncData();

	/**
	 * @brief createResult Create a room for results in the main list and return the location.
	 * @param data Initialize list as results.
	 * @return 
	 */
	std::list<SearchResult> *createResult(std::list<SearchResult> data = std::list<SearchResult>());

	/**
	 * @brief getCurrentRequest Get the current request if there is one in the queue.
	 * @param[out] result Current request.
	 * @return true if the queue is not empty.
	 */
	bool getCurrentRequest(std::pair<std::string, std::string> * result);

	/**
	 * @brief keepOneRequest Remove all request in queue and keep only the last entered.
	 * @return true the queue is empty.
	 */
	bool keepOneRequest();

	/**
	 * @brief pushRequest Add a request in the queue (FIFO): Thread-safe. 
	 * @param request Pair of <Filter, withDomain>.
	 * @return The queue size.
	 */
	int pushRequest(const std::pair<std::string, std::string>& request);

	/**
	 * @brief pushData Add a provider in the vector.
	 * @param data A pointer of #CbData to allow any types of provider.
	 */
	void pushData(std::shared_ptr<CbData> data);

	/**
	 * @brief initStartTime To be call when starting a search for timeout computation.
	 */
	void initStartTime();

	/**
	 * @brief getStartTime Get the start time initialized by initStartTime().
	 * @return The start time.
	 */
	bctoolboxTimeSpec getStartTime() const;

	/**
	 * @brief setSearchResults Set the final search result
	 * @param resultList The search result list.
	 * @return true if results are not null.
	 */
	bool setSearchResults(std::shared_ptr<std::list<SearchResult> > resultList);


	/**
	 * @brief getData Const getter for the vector of provider
	 * @return Const array of provider.
	 */

	const std::vector<std::shared_ptr<CbData> >& getData() const;
	/**
	 * @brief getData  Getter for the vector of provider
	 * @return Array of provider.
	 */
	std::vector<std::shared_ptr<CbData> >& getData();

	/**
	 * @brief clear Clear results and providers
	 */
	void clear();
private:

	/**
	 * @brief mRequests Queue of requests. Not very useful yet but can be used for historic search
	 * pair <Filter/WithDomain>
	 */
	std::queue<std::pair<std::string, std::string> > mRequests;	

	/**
	 * @brief mLockQueue Protect the queue for read/write : we can add requests on any threads. All requests are removed from the main iteration.
	 */
	ms_mutex_t mLockQueue;

	/**
	 * @brief mStartTime Start time of the request for timeout computation
	 */
	bctoolboxTimeSpec mStartTime;

	/**
	 * @brief mProvidersCbData Store a vector of providers. By this way, we can use any providers we want on an asynchronous strategy.
	 */
	std::vector<std::shared_ptr<CbData> > mProvidersCbData;
};
LINPHONE_END_NAMESPACE

#endif //_L_MAGIC_SEARCH_ASYNC_DATA_H_
