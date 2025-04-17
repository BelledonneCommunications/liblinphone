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

#ifndef _L_MAGIC_SEARCH_H_
#define _L_MAGIC_SEARCH_H_

#include <list>
#include <memory>
#include <queue>
#include <regex>
#include <string>

#include "c-wrapper/c-wrapper.h"
#include "linphone/api/c-callbacks.h"
#include "linphone/api/c-types.h"

#include "linphone/api/c-types.h"
#include "linphone/types.h"

#include "linphone/utils/general.h"

#include "core/core-accessor.h"
#include "core/core.h"
#include "magic-search-plugin.h"
#include "search-async-data.h"
#include "search-request.h"
#include "search-result.h"

LINPHONE_BEGIN_NAMESPACE

class MagicSearchCbs;

typedef void (*MagicSearchCallback)(std::list<std::shared_ptr<Friend>> friends, void *data, bool_t haveMoreResults);

class LINPHONE_PUBLIC MagicSearch : public bellesip::HybridObject<LinphoneMagicSearch, MagicSearch>,
                                    public UserDataAccessor,
                                    public CallbacksHolder<MagicSearchCbs>,
                                    public CoreAccessor {
public:
	friend class SearchAsyncData;

	MagicSearch(const std::shared_ptr<Core> &core);
	MagicSearch(const MagicSearch &ms) = delete;
	virtual ~MagicSearch();

	MagicSearch *clone() const override;

	/**
	 * Set the minimum value used to calculate the weight in search
	 * @param[in] weight minimum weight
	 **/
	void setMinWeight(const unsigned int weight);

	/**
	 * @return the minimum value used to calculate the weight in search
	 **/
	unsigned int getMinWeight() const;

	/**
	 * Set the maximum value used to calculate the weight in search
	 * @param[in] weight maximum weight
	 **/
	void setMaxWeight(const unsigned int weight);

	/**
	 * @return the maximum value used to calculate the weight in search
	 **/
	unsigned int getMaxWeight() const;

	/**
	 * @return the delimiter used to find matched filter word
	 **/
	const std::string &getDelimiter() const;

	/**
	 * Set the delimiter used to find matched filter word
	 * @param[in] delimiter delimiter (example "-_.,")
	 **/
	void setDelimiter(const std::string &delimiter);

	/**
	 * @return if the delimiter search is used
	 **/
	bool getUseDelimiter() const;

	/**
	 * Enable or disable the delimiter in search
	 * @param[in] enable
	 **/
	void setUseDelimiter(bool enable);

	/**
	 * @return the number of the maximum SearchResult which will be return
	 **/
	unsigned int getSearchLimit() const;

	/**
	 * Set the number of the maximum SearchResult which will be return
	 * @param[in] limit
	 **/
	void setSearchLimit(const unsigned int limit);

	/**
	 * @return if the search is limited
	 **/
	bool getLimitedSearch() const;

	/**
	 * Enable or disable the limited search
	 * @param[in] limited
	 **/
	void setLimitedSearch(const bool limited);

	/**
	 * Reset the cache to begin a new search
	 **/
	void resetSearchCache();

	/**
	 * Create a sorted list of SearchResult from SipUri, Contact name,
	 * Contact displayname, Contact phone number, which match with a filter word
	 * The last item list will be an address formed with "filter" if a proxy config exist
	 * During the first search, a cache is created and used for the next search
	 * Use resetSearchCache() to begin a new search
	 * @param[in] filter word we search
	 * @param[in] withDomain
	 ** "" for searching in all contact
	 ** "*" for searching in contact with sip SipUri
	 ** "yourdomain" for searching in contact from "yourdomain" domain
	 * @param[in] sourceFlags Make a search in selected sources
	 * @param[in] aggregation a #LinphoneMagicSearchAggregation flag to indicate how to merge results
	 * @return sorted list of SearchResult with "filter" or an empty list if "filter" is empty
	 **/
	std::list<std::shared_ptr<SearchResult>>
	getContactListFromFilter(const std::string &filter,
	                         const std::string &withDomain = "",
	                         int sourceFlags = LinphoneMagicSearchSourceAll,
	                         LinphoneMagicSearchAggregation aggregation = LinphoneMagicSearchAggregationNone);

	/**
	 * Create a sorted list of SearchResult from SipUri, Contact name,
	 * Contact displayname, Contact phone number, which match with a filter word
	 * The last item list will be an address formed with "filter" if a proxy config exist
	 * During the first search, a cache is created and used for the next search
	 * Use linphone_magic_search_reset_search_cache() to begin a new search
	 * @param[in] filter word we search
	 * @param[in] withDomain
	 ** NULL or "" for searching in all contact
	 ** "*" for searching in contact with sip SipUri
	 ** "yourdomain" for searching in contact from "yourdomain" domain
	 * @param[in] sourceFlags Make a search in selected sources
	 * @param[in] aggregation a #LinphoneMagicSearchAggregation flag to indicate how to merge results
	 **/
	void getContactListFromFilterAsync(const std::string &filter,
	                                   const std::string &withDomain,
	                                   int sourceFlags = LinphoneMagicSearchSourceAll,
	                                   LinphoneMagicSearchAggregation aggregation = LinphoneMagicSearchAggregationNone);

	/**
	 * @return sorted list of SearchResult with "filter" or an empty list if "filter" is empty
	 *
	 **/
	std::list<std::shared_ptr<SearchResult>> getLastSearch() const;

	// When a new search start, let MagicSearch to clean its cache. Default to true.
	void setAutoResetCache(const bool &enable);

	/**
	 * @brief arePluginsProcessingDone Check if all Async processes are done.
	 * Test if getting addresses is over. It will cancel all providers that reach their timeout (this mecanism is an
	 * addition to the provider cancellation to make sure to stop all processes).
	 * @param asyncData Instance to use for all data storage.
	 * @return true if all is over.
	 */
	bool arePluginsProcessingDone(SearchAsyncData *asyncData) const;

	/**
	 * @brief mergeResults Merge all the providers results and build the final search vector, accessible with
	 * mSearchResults. It sorts on display name and if equal, the sort will be in this order : Friend's display name,
	 * address username, address domain, phone number
	 * @param asyncData Instance to use for all data storage.
	 */
	void mergeResults(SearchAsyncData *asyncData);

	/**
	 * Search informations in address given
	 * @param[in] lAddress address whose informations will be check
	 * @param[in] withDomain domain which we want to search only
	 * @private
	 **/
	unsigned int searchInAddress(const std::shared_ptr<const Address> &lAddress, const std::string &withDomain) const;

	/** Inits all magic search plugins, allowing external sources to be used, such as LDAP or distant CardDAV queries */
	void initPlugins();

	/** Returns whether the magic search plugin should make sure the returned results match the filter or not */
	bool filterPluginsResults() const;

	/**
	 * @return the cache of precedent result
	 * @private
	 **/
	const std::list<std::shared_ptr<SearchResult>> &getSearchCache() const;

	/**
	 * Save a result for future search
	 * @param[in] cache result we want to save
	 * @private
	 **/
	void setSearchCache(std::list<std::shared_ptr<SearchResult>> cache);

	/** Get SearchResults matching Friends in all FriendLists available in Core */
	std::list<std::shared_ptr<SearchResult>> getResultsFromFriends(bool onlyStarred, const std::string &withDomain);

	/**
	 * Get all addresses from call log
	 * @param[in] filter word we search
	 * @param[in] withDomain domain which we want to search only
	 * @param[in] currentList current list where we will check if address already exist
	 * @return all addresses from call log which match in a SearchResult list
	 * @private
	 **/
	std::list<std::shared_ptr<SearchResult>>
	getAddressFromCallLog(const std::string &filter,
	                      const std::string &withDomain,
	                      const std::list<std::shared_ptr<SearchResult>> &currentList) const;

	/**
	 * Get all addresses from chat rooms participants
	 * @param[in] filter word we search
	 * @param[in] withDomain domain which we want to search only
	 * @param[in] currentList current list where we will check if address already exist
	 * @return all address from chat rooms participants which match in a SearchResult list
	 * @private
	 **/
	std::list<std::shared_ptr<SearchResult>>
	getAddressFromGroupChatRoomParticipants(const std::string &filter,
	                                        const std::string &withDomain,
	                                        const std::list<std::shared_ptr<SearchResult>> &currentList) const;

	/**
	 * Get all addresses from conferences info participants & organizer
	 * @param[in] filter word we search
	 * @param[in] withDomain domain which we want to search only
	 * @param[in] currentList current list where we will check if address already exist
	 * @return all addresses from conferences info organizer and participants which match in a SearchResult list
	 * @private
	 **/
	std::list<std::shared_ptr<SearchResult>>
	getAddressFromConferencesInfo(const std::string &filter,
	                              const std::string &withDomain,
	                              const std::list<std::shared_ptr<SearchResult>> &currentList) const;

	/**
	 * Get all friends as SearchResult
	 * @param[in] withDomain domain which we want to search only
	 * @return all friends in a SearchResult list
	 * @private
	 **/
	std::list<std::shared_ptr<SearchResult>> getFriends(const std::string &withDomain) const;

	/**
	 * Begin the search from friend list
	 * @param[in] filter word we search
	 * @param[in] withDomain domain which we want to search only
	 * @param[in] sourceFlags Flags where to search #LinphoneMagicSearchSource
	 * @private
	 **/
	std::list<std::shared_ptr<SearchResult>>
	beginNewSearch(const std::string &filter, const std::string &withDomain, int sourceFlags);

	/**
	 * Continue the search from the cache of precedent search
	 * @param[in] withDomain domain which we want to search only
	 * @param[in] aggregation #LinphoneMagicSearchAggregation to use (Friend or None)
	 * @private
	 **/
	std::list<std::shared_ptr<SearchResult>> continueSearch(const std::string &withDomain,
	                                                        LinphoneMagicSearchAggregation aggregation) const;

	/**
	 * Search informations in friend given
	 * @param[in] lFriend friend whose informations will be check
	 * @param[in] withDomain domain which we want to search only
	 * @param[in] flags flags to set in SearchResult(s)
	 * @return list of result from friend
	 * @private
	 **/
	std::list<std::shared_ptr<SearchResult>>
	searchInFriend(const std::shared_ptr<Friend> &lFriend, const std::string &withDomain, int flags) const;

	/**
	 * Creates a search result from a given friend.
	 * @param[in] lFriend friend to use (no check will be made)
	 * @param[in] withDomain domain which we want to search only
	 * @param[in] flags flags to set in SearchResult(s)
	 * @return result from friend
	 * @private
	 **/
	std::shared_ptr<SearchResult>
	createResultFromFriend(const std::shared_ptr<Friend> &lFriend, const std::string &withDomain, int flags) const;

	/**
	 * Return a weight for a searched in with a filter
	 * @param[in] haystack which where we are searching
	 * @return calculate weight
	 * @private
	 **/
	unsigned int getWeight(const std::string &haystack) const;

	/**
	 * Return if the given address match domain policy
	 * @param[in] lFriend friend whose domain will be check
	 * @param[in] lAddress address whose domain will be check
	 * @param[in] withDomain domain policy
	 * @private
	 **/
	bool checkDomain(const std::shared_ptr<Friend> &lFriend,
	                 const std::shared_ptr<const Address> &lAddress,
	                 const std::string &withDomain) const;

	/** Adds all results items at the end of the resultsList list */
	void addResultsToResultsList(const std::list<std::shared_ptr<SearchResult>> &results,
	                             std::list<std::shared_ptr<SearchResult>> &resultsList) const;

	/** Removes duplicates from list, using Address to tell if same or not */
	void uniqueItemsList(std::list<std::shared_ptr<SearchResult>> &list) const;

	/** Removes duplicates from list, using Friend to tell if same or not */
	void uniqueFriendsInList(std::list<std::shared_ptr<SearchResult>> &list) const;

	enum { STATE_START, STATE_WAIT, STATE_SEND, STATE_END, STATE_CANCEL };

	/**
	 * @brief getContactListFromFilterStartAsync Start async requests by building providers and prepare SearchAsyncData.
	 * @param request : #SearchRequest that define filter, domain which we want to search only and source flags where to
	 * search (#LinphoneMagicSearchSource)
	 * @return true if results are already available (coming from cache).
	 */
	bool getContactListFromFilterStartAsync(const SearchRequest &request, SearchAsyncData *asyncData);

	/**
	 * @brief processResults Clean for unique items and set the cache.
	 * @return the cleaned list.
	 */
	std::list<std::shared_ptr<SearchResult>> processResults(std::list<std::shared_ptr<SearchResult>> &results);

	/**
	 * @brief beginNewSearchAsync Same as beginNewSearch but on an asynchronous version : it will build the
	 * SearchAsyncData from async providers like LDAP.
	 * @param request : #SearchRequest that define filter, domain which we want to search only and source flags where to
	 * search (#LinphoneMagicSearchSource)
	 * @param asyncData Instance to use for all data storage.
	 */
	void beginNewSearchAsync(const SearchRequest &request, SearchAsyncData *asyncData);

	/**
	 * @brief iterate Iteration that is executed in the main loop.
	 * State follows this flow:
	 * STATE_START => (STATE_WAIT) => STATE_SEND [<=] => STATE_END
	 * @return
	 */
	bool iterate(void);

private:
	void setupRegex(const std::string &filter);

	int mState = 0;
	unsigned int mMinWeight = 0;
	unsigned int mMaxWeight = 1000;
	unsigned int mSearchLimit = 30; // Number of ResultSearch maximum when the search is limited
	bool mLimitedSearch = true;     // Limit the search
	int mRequestDelay = 500;        // Delay the first request in ms
	std::string mDelimiter = "+_-"; // Delimiter use for the search
	bool mUseDelimiter = true;
	std::string mFilter;
	bool mAutoResetCache = true; // When a new search start, let MagicSearch to clean its cache
	bool returnEmptyFriends = false;
	std::string mFilterRegex;
	bool mFilterApplyFullSipUri =
	    false; // If true, searchInAddress will check the full SIP URI, otherwise only display name & username

	belle_sip_source_t *mIteration = nullptr;

	std::list<std::shared_ptr<SearchResult>> mCacheResult;
	SearchAsyncData mAsyncData;
	std::list<std::shared_ptr<MagicSearchPlugin>> mPlugins;
};

class MagicSearchCbs : public bellesip::HybridObject<LinphoneMagicSearchCbs, MagicSearchCbs>, public Callbacks {
public:
	LinphoneMagicSearchCbsSearchResultsReceivedCb getResultsReceived() const {
		return mResultsAvailableCb;
	}
	void setResultsReceived(LinphoneMagicSearchCbsSearchResultsReceivedCb cb) {
		mResultsAvailableCb = cb;
	}
	LinphoneMagicSearchCbsLdapHaveMoreResultsCb getLdapMoreResultsAvailable() const {
		return mLdapMoreResultsAvailableCb;
	}
	void setLdapMoreResultsAvailable(LinphoneMagicSearchCbsLdapHaveMoreResultsCb cb) {
		mLdapMoreResultsAvailableCb = cb;
	}
	LinphoneMagicSearchCbsMoreResultsAvailableCb getMoreResultsAvailable() const {
		return mMoreResultsAvailableCb;
	}
	void setMoreResultsAvailable(LinphoneMagicSearchCbsMoreResultsAvailableCb cb) {
		mMoreResultsAvailableCb = cb;
	}

private:
	LinphoneMagicSearchCbsSearchResultsReceivedCb mResultsAvailableCb = nullptr;
	LinphoneMagicSearchCbsLdapHaveMoreResultsCb mLdapMoreResultsAvailableCb = nullptr;
	LinphoneMagicSearchCbsMoreResultsAvailableCb mMoreResultsAvailableCb = nullptr;
};

LINPHONE_END_NAMESPACE

#endif //_L_MAGIC_SEARCH_H_
