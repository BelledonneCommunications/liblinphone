/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#ifndef _L_MAGIC_SEARCH_PLUGIN_H_
#define _L_MAGIC_SEARCH_PLUGIN_H_

#include "core/core-accessor.h"
#include "core/core.h"
#include "search-async-data.h"

LINPHONE_BEGIN_NAMESPACE

/** Interface to inherit from when adding a new source for MagicSearch results, used by LDAP & remote CardDAV queries */
class MagicSearchPlugin : public CoreAccessor {
public:
	MagicSearchPlugin(const std::shared_ptr<Core> &core,
	                  MagicSearch &magicSearch,
	                  LinphoneMagicSearchSource source,
	                  const std::string &displayName)
	    : CoreAccessor(core), mMagicSearch(magicSearch) {
		mSource = source;
		mDisplayName = displayName;
	}
	virtual ~MagicSearchPlugin() {
	}

	/** In case of asynchronous query, asks the plugin to stop the work */
	virtual void stop() = 0;

	/** Called when the MagicSearch instance is asked to perform a synchronous query */
	virtual std::list<std::shared_ptr<SearchResult>> startSearch(const std::string &filter,
	                                                             const std::string &domain) = 0;

	/** Called when the MagicSearch instance is asked to perform an asynchronous query */
	virtual void startSearchAsync(const std::string &filter, const std::string &domain, SearchAsyncData *asyncData) = 0;

	LinphoneMagicSearchSource getSource() const {
		return mSource;
	}

	/** Gets a reference to the MagicSearch object used to create this plugin */
	MagicSearch &getMagicSearch() const {
		return mMagicSearch;
	}

	/** Returns true if the plugin considers the query as completed, false otherwise */
	bool getHasEnded() const {
		return mEnded;
	}
	void setHasEnded(bool ended) {
		mEnded = ended;
	}

	/** When using asynchronous operations, allows to cancel a query if it takes too long */
	int64_t getTimeout() const {
		return mTimeout;
	}
	void setTimeout(int64_t timeout) {
		mTimeout = timeout;
	}

	/** Store plugin related SearchResults, will be processed and merged by MagicSearch when all queries are done */
	void setResults(std::list<std::shared_ptr<SearchResult>> &results) {
		mResults = results;
	}
	const std::list<std::shared_ptr<SearchResult>> &getResults() {
		return mResults;
	}
	void addToResults(std::shared_ptr<SearchResult> result) {
		mResults.push_back(result);
	}

	/** Text filter given to MagicSearch object to filter results on */
	const std::string &getFilter() const {
		return mFilter;
	}
	void setFilter(const std::string &filter) {
		mFilter = filter;
	}

	/** SIP domain (may be empty or wildcard) to use to filter results on  */
	const std::string &getDomain() const {
		return mWithDomain;
	}
	void setDomain(const std::string &domain) {
		mWithDomain = domain;
	}

	const std::string &getDisplayName() const {
		return mDisplayName;
	}

private:
	MagicSearch &mMagicSearch;
	LinphoneMagicSearchSource mSource;

	bool mEnded = true; // Consider the plugin request as ended by default in case plugin do not match the source flag

	int64_t mTimeout = 5; // 5 secs

	std::list<std::shared_ptr<SearchResult>> mResults;

	std::string mDisplayName;
	std::string mFilter;
	std::string mWithDomain;
};

LINPHONE_END_NAMESPACE

#endif //_L_MAGIC_SEARCH_PLUGIN_H_
