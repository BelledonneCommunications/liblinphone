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

#ifndef _L_MAGIC_SEARCH_H_
#define _L_MAGIC_SEARCH_H_

#include <string>
#include <list>
#include <memory>
#include <queue>

#include "core/core.h"
#include "core/core-accessor.h"
#include "search-result.h"

LINPHONE_BEGIN_NAMESPACE

class MagicSearchPrivate;
class SearchAsyncData;

class LINPHONE_PUBLIC MagicSearch : public CoreAccessor, public Object{
public:
	friend class SearchAsyncData;
	MagicSearch (const std::shared_ptr<Core> &core);
	MagicSearch (const MagicSearch &ms) = delete;
	~MagicSearch ();

	/**
	 * Set the minimum value used to calculate the weight in search
	 * @param[in] weight minimum weight
	 **/
	void setMinWeight (const unsigned int weight);

	/**
	 * @return the minimum value used to calculate the weight in search
	 **/
	unsigned int getMinWeight () const;

	/**
	 * Set the maximum value used to calculate the weight in search
	 * @param[in] weight maximum weight
	 **/
	void setMaxWeight (const unsigned int weight);

	/**
	 * @return the maximum value used to calculate the weight in search
	 **/
	unsigned int getMaxWeight () const;

	/**
	 * @return the delimiter used to find matched filter word
	 **/
	const std::string& getDelimiter () const;

	/**
	 * Set the delimiter used to find matched filter word
	 * @param[in] delimiter delimiter (example "-_.,")
	 **/
	void setDelimiter (const std::string &delimiter);

	/**
	 * @return if the delimiter search is used
	 **/
	bool getUseDelimiter () const;

	/**
	 * Enable or disable the delimiter in search
	 * @param[in] enable
	 **/
	void setUseDelimiter (bool enable);

	/**
	 * @return the number of the maximum SearchResult which will be return
	 **/
	unsigned int getSearchLimit () const;

	/**
	 * Set the number of the maximum SearchResult which will be return
	 * @param[in] limit
	 **/
	void setSearchLimit (const unsigned int limit);

	/**
	 * @return if the search is limited
	 **/
	bool getLimitedSearch () const;

	/**
	 * Enable or disable the limited search
	 * @param[in] limited
	 **/
	void setLimitedSearch (const bool limited);

	/**
	 * Reset the cache to begin a new search
	 **/
	void resetSearchCache ();

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
	 * @return sorted list of SearchResult with "filter" or an empty list if "filter" is empty
	 **/
	std::list<SearchResult> getContactListFromFilter (const std::string &filter, const std::string &withDomain = "");

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
	 **/
	void getContactListFromFilterAsync (const std::string &filter, const std::string &withDomain);
	
	/**
	 * @return sorted list of SearchResult with "filter" or an empty list if "filter" is empty
	 * 
	 **/
	std::list<SearchResult> getLastSearch() const;
	
// When a new search start, let MagicSearch to clean its cache. Default to true.
	void setAutoResetCache(const bool_t& enable);
	
	
private:

	/**
	 * @return the cache of precedent result
	 * @private
	 **/
	std::shared_ptr<std::list<SearchResult> > getSearchCache () const;

	/**
	 * Save a result for future search
	 * @param[in] cache result we want to save
	 * @private
	 **/
	void setSearchCache (std::shared_ptr<std::list<SearchResult>> cache);

	/**
	 * Get all addresses from call log
	 * @param[in] filter word we search
	 * @param[in] withDomain domain which we want to search only
	 * @param[in] currentList current list where we will check if address already exist
	 * @return all addresses from call log which match in a SearchResult list
	 * @private
	 **/
	std::list<SearchResult> getAddressFromCallLog (
		const std::string &filter,
		const std::string &withDomain,
		const std::list<SearchResult> &currentList
	) const;

	/**
	 * Get all addresses from chat rooms participants
	 * @param[in] filter word we search
	 * @param[in] withDomain domain which we want to search only
	 * @param[in] currentList current list where we will check if address already exist
	 * @return all address from chat rooms participants which match in a SearchResult list
	 * @private
	 **/
	std::list<SearchResult> getAddressFromGroupChatRoomParticipants (
		const std::string &filter,
		const std::string &withDomain,
		const std::list<SearchResult> &currentList
	) const;

#ifdef LDAP_ENABLED
	/**
	 * Get all addresses from LDAP Server
	 * @param[in] filter word we search
	 * @param[in] withDomain domain which we want to search only
	 * @return all address from chat rooms participants which match in a SearchResult list
	 * @private
	 **/
	std::list<std::list<SearchResult> > getAddressFromLDAPServer(
		const std::string &filter,
		const std::string &withDomain
	);
#endif

	/**
	 * Get all friends as SearchResult
	 * @param[in] withDomain domain which we want to search only
	 * @return all friends in a SearchResult list
	 * @private
	 **/
	std::list<SearchResult> getFriends (const std::string &withDomain) const;

	/**
	 * Begin the search from friend list
	 * @param[in] filter word we search
	 * @param[in] withDomain domain which we want to search only
	 * @private
	 **/
	std::shared_ptr<std::list<SearchResult> > beginNewSearch (const std::string &filter, const std::string &withDomain);

	/**
	 * Continue the search from the cache of precedent search
	 * @param[in] filter word we search
	 * @param[in] withDomain domain which we want to search only
	 * @private
	 **/
	std::shared_ptr<std::list<SearchResult> > continueSearch (const std::string &filter, const std::string &withDomain) const;

	/**
	 * Search informations in friend given
	 * @param[in] lFriend friend whose informations will be check
	 * @param[in] filter word we search
	 * @param[in] withDomain domain which we want to search only
	 * @return list of result from friend
	 * @private
	 **/
	std::list<SearchResult> searchInFriend (const LinphoneFriend* lFriend, const std::string &filter, const std::string &withDomain) const;

	/**
	 * Search informations in address given
	 * @param[in] lAddress address whose informations will be check
	 * @param[in] filter word we search
	 * @param[in] withDomain domain which we want to search only
	 * @private
	 **/
	unsigned int searchInAddress (const LinphoneAddress *lAddress, const std::string &filter, const std::string &withDomain) const;

	/**
	 * Return a weight for a searched in with a filter
	 * @param[in] stringWords which where we are searching
	 * @param[in] filter what we are searching
	 * @return calculate weight
	 * @private
	 **/
	unsigned int getWeight (const std::string &stringWords, const std::string &filter) const;

	/**
	 * Return if the given address match domain policy
	 * @param[in] lFriend friend whose domain will be check
	 * @param[in] lAddress address whose domain will be check
	 * @param[in] withDomain domain policy
	 * @private
	 **/
	bool checkDomain (const LinphoneFriend* lFriend, const LinphoneAddress *lAddress, const std::string &withDomain) const;

	void addResultsToResultsList (std::list<SearchResult> &results, std::list<SearchResult> &srL) const;

	void uniqueItemsList (std::list<SearchResult> &list) const;
	enum{
		STATE_START,
		STATE_WAIT,
		STATE_SEND,
		STATE_END
	};

	/**
	 * @brief getContactListFromFilterStartAsync Start async requests by building providers and prepare SearchAsyncData.
	 * @param filter word we search.
	 * @param withDomain domain which we want to search only.
	 * @param asyncData Instance to use for all data storage.
	 * @return true if results are already available (coming from cache).
	 */
	bool getContactListFromFilterStartAsync (const std::string &filter, const std::string &withDomain, SearchAsyncData * asyncData);
#ifdef LDAP_ENABLED
	/**
	 * @brief getAddressFromLDAPServerStartAsync Initialize SearchAsyncData to add LDAP providers for a request on addresses.
	 * It is called from Iterate to auto start the process when ready.
	 * @param filter word we search.
	 * @param withDomain domain which we want to search only.
	 * @param asyncData Instance to use for all data storage.
	 */
	void getAddressFromLDAPServerStartAsync (const std::string &filter,const std::string &withDomain, SearchAsyncData * asyncData)const;
#endif

	/**
	 * @brief getAddressIsEndAsync Check if all Async processes are done.
	 * Test if getting addresses is over. It will cancel all providers that reach their timeout (this mecanism is an addition to the provider cancellation to make sure to stop all processes).
	 * @param asyncData Instance to use for all data storage.
	 * @return true if all is over.
	 */
	bool getAddressIsEndAsync(SearchAsyncData* asyncData) const;

	/**
	 * @brief mergeResults Merge all the providers results and build the final search vector, accessible with mSearchResults.
	 * @param filter word we search.
	 * @param withDomain domain which we want to search only.
	 * @param asyncData Instance to use for all data storage.
	 */
	void mergeResults (const std::string& filter, const std::string withDomain, SearchAsyncData * asyncData);

	/**
	 * @brief processResults Clean for unique items and set the cache.
	 * @return the cleaned list.
	 */
	std::list<SearchResult> processResults(std::shared_ptr<std::list<SearchResult> >);
	
	/**
	 * @brief beginNewSearchAsync Same as beginNewSearch but on an asynchronous version : it will build the SearchAsyncData from async providers like LDAP.
	 * @param filter word we search.
	 * @param withDomain domain which we want to search only.
	 * @param asyncData Instance to use for all data storage.
	 */
	void beginNewSearchAsync (const std::string &filter, const std::string &withDomain, SearchAsyncData * asyncData) const;

	/**
	 * @brief addResultsToResultsList Same as addResultsToResultsList but apply an unicity filtering before splicing. It is usefull to prioritize results based to the order of providers.
	 * @param results List of #SearchResult to add.
	 * @param srL List of #SearchResult to modify.
	 * @param filter word we search.
	 * @param withDomain domain which we want to search only.
	 */
	void addResultsToResultsList (std::list<SearchResult> &results, std::list<SearchResult> &srL, const std::string filter, const std::string& withDomain) const;

	int mState;
	/**
	 * @brief iterate Iteration that is executed in the main loop.
	 * State follows this flow:
	 * STATE_START => (STATE_WAIT) => STATE_SEND [<=] => STATE_END
	 * @return 
	 */
	bool iterate(void);
	
	L_DECLARE_PRIVATE(MagicSearch);
};

LINPHONE_END_NAMESPACE

#endif //_L_MAGIC_SEARCH_H_
