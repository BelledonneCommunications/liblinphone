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

#ifndef _L_MAGIC_SEARCH_ASYNC_DATA_H_
#define _L_MAGIC_SEARCH_ASYNC_DATA_H_

#include <list>
#include <queue>
#include <string>

#include "search-request.h"
#include "search-result.h"

LINPHONE_BEGIN_NAMESPACE

class MagicSearch;

/** Class that stores data during asynchronous operations done by the MagicSearch object */
class SearchAsyncData {
public:
	SearchAsyncData();
	~SearchAsyncData();

	/**
	 * @brief isRequestPending Remove all request in queue and keep only the last entered.
	 * @return true the queue is not empty.
	 */
	bool isRequestPending();

	/**
	 * @brief pushRequest Add a request in the queue (FIFO): Thread-safe.
	 * @param request SearchRequest that define filter, domain and search sources.
	 * @return The queue size.
	 */
	int pushRequest(const SearchRequest &request);

	/**
	 * @brief getCurrentRequest Get the current request if there is one in the queue.
	 * @param[out] result Current request.
	 * @return true if the queue is not empty.
	 */
	bool getCurrentRequest(SearchRequest *result);

	const std::list<SearchRequest> &getRequestHistory() const {
		return mRequestHistory;
	}

	/**
	 * @brief initStartTime To be call when starting a search for timeout computation.
	 */
	void initStartTime();

	/**
	 * @brief getStartTime Get the start time initialized by initStartTime().
	 * @return The start time.
	 */
	bctoolboxTimeSpec getStartTime() const {
		return mStartTime;
	}

	/**
	 * @brief setSearchRequest Set the search request criteria
	 * @param request : #SearchRequest that define filter, domain which we want to search only and source flags where to
	 * search (#LinphoneMagicSearchSource)
	 */
	void setSearchRequest(const SearchRequest &request) {
		mSearchRequest = request;
	}

	const SearchRequest &getSearchRequest() const {
		return mSearchRequest;
	}

	/**
	 * @brief setSearchResults Set the final search result
	 * @param resultList The search result list.
	 * @return true if results are not null.
	 */
	bool setSearchResults(std::list<std::shared_ptr<SearchResult>> &resultList);

	const std::list<std::shared_ptr<SearchResult>> &getSearchResults() const {
		return mSearchResults;
	}

	void addToResults(std::list<std::shared_ptr<SearchResult>> &data);

	/**
	 * @brief clear Clear results and providers
	 */
	void clear();

private:
	/**
	 * @brief mSearchResults This is the final result to use.
	 */
	std::list<std::shared_ptr<SearchResult>> mSearchResults;

	/**
	 * @brief mSearchRequest Search criteria.
	 */
	SearchRequest mSearchRequest;

	/**
	 * @brief mRequests Queue of requests. Not very useful yet but can be used for historic search
	 * SearchRequest that define filter, domain and sources where to search.
	 */
	std::queue<SearchRequest> mRequests;

	/**
	 * @brief List of the requests previously made.
	 */
	std::list<SearchRequest> mRequestHistory;

	/**
	 * @brief mLockQueue Protect the queue for read/write : we can add requests on any threads. All requests are removed
	 * from the main iteration.
	 */
	ms_mutex_t mLockQueue;

	/**
	 * @brief mStartTime Start time of the request for timeout computation
	 */
	bctoolboxTimeSpec mStartTime;
};
LINPHONE_END_NAMESPACE

#endif //_L_MAGIC_SEARCH_ASYNC_DATA_H_
