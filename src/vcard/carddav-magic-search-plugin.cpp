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

#include "carddav-magic-search-plugin.h"
#include "http/http-client.h"

LINPHONE_BEGIN_NAMESPACE

using namespace std;

CardDavMagicSearchPlugin::CardDavMagicSearchPlugin(const shared_ptr<Core> &core,
                                                   MagicSearch &magicSearch,
                                                   shared_ptr<CardDavParams> params)
    : MagicSearchPlugin(core, magicSearch, LinphoneMagicSearchSourceRemoteCardDAV, "CardDAV") {
	mParams = params;
	setTimeout(mParams->getTimeout());

	mCardDavContext = make_shared<CardDAVContext>(core);
}

void CardDavMagicSearchPlugin::stop() {
	setHasEnded(true);

	if (mCardDavContext) {
		mCardDavContext->setMagicSearchPlugin(nullptr);
		mCardDavContext = nullptr;
	}
}

list<shared_ptr<SearchResult>> CardDavMagicSearchPlugin::startSearch(BCTBX_UNUSED(const string &filter),
                                                                     BCTBX_UNUSED(const string &domain)) {
	lWarning() << "[Magic Search][CardDAV] Synchronous search isn't supported";
	setHasEnded(true);
	return list<shared_ptr<SearchResult>>();
}

void CardDavMagicSearchPlugin::startSearchAsync(const string &filter,
                                                const string &domain,
                                                SearchAsyncData *asyncData) {
	setHasEnded(false);
	mMoreResultsAvailable = false;

	if (filter.size() < mParams->getMinCharactersToStartQuery()) {
		lInfo() << "[Magic Search][CardDAV] Too few characters to start the query, doing nothing";
		setHasEnded(true);
		return;
	}

	if (!linphone_core_is_network_reachable(getCore()->getCCore())) {
		lWarning() << "[Magic Search][CardDAV] Network isn't reachable, can't send CardDAV query, doing nothing";
		setHasEnded(true);
		return;
	}

	lInfo() << "[Magic Search][CardDAV] Starting async search with filter [" << filter << "] and domain [" << domain
	        << "] on server URL [" << mParams->getServerUrl() << "]";
	mFilter = filter;
	mDomain = domain;
	mAsyncData = asyncData;

	mCardDavContext->setMagicSearchPlugin(shared_from_this());
	mCardDavContext->queryVcards(mParams->getServerUrl(), computePropFilters(), mParams->getLimit());
}

void CardDavMagicSearchPlugin::sendQueryAgainAfterDiscoveryProcess() {
	mCardDavContext->queryVcards(mParams->getServerUrl(), computePropFilters(), mParams->getLimit());
}

void CardDavMagicSearchPlugin::notifyError(const string &errorMessage) {
	lError() << "[Magic Search][CardDAV] Error: " << errorMessage;
	setHasEnded(true);
}

void CardDavMagicSearchPlugin::processResults(const list<shared_ptr<Friend>> &friends) {
	lInfo() << "[Magic Search][CardDAV] Processing [" << friends.size() << "] friends";

	list<shared_ptr<SearchResult>> resultList;
	bool filterPluginResults = getMagicSearch().filterPluginsResults() || (!mDomain.empty() && mDomain != "*");

	for (auto &lFriend : friends) {
		if (filterPluginResults) {
			auto found = getMagicSearch().searchInFriend(lFriend, mDomain, getSource());
			if (resultList.empty()) {
				resultList = found;
			} else if (!found.empty()) {
				resultList.splice(resultList.end(), found);
			}
		} else {
			auto result = getMagicSearch().createResultFromFriend(lFriend, mDomain, getSource());
			resultList.push_back(result);
		}
	}

	if (filterPluginResults) {
		lInfo() << "[Magic Search][CardDAV] Found " << resultList.size() << " results (after local filter is applied)";
	} else {
		lInfo() << "[Magic Search][CardDAV] Found " << resultList.size() << " results (no local filter was applied)";
	}
	setResults(resultList);

	if (mMoreResultsAvailable) {
		lInfo() << "[Magic Search][CardDAV] Server response says more results are available";
		_linphone_magic_search_notify_more_results_available(getMagicSearch().toC(), getSource());
	}

	setHasEnded(true);
}

list<CardDavPropFilter> CardDavMagicSearchPlugin::computePropFilters() const {
	list<CardDavPropFilter> filters;
	bool exactMatch = mParams->getUseExactMatchPolicy();

	auto fields = mParams->getFieldsToFilterUserInputOn();
	if (!fields.empty() && !mFilter.empty()) {
		for (auto propFilter : fields) {
			filters.push_back(CardDavPropFilter(mFilter, propFilter, exactMatch));
		}
	}

	fields = mParams->getFieldsToFilterDomainOn();
	if (!fields.empty() && !mDomain.empty()) {
		for (auto propFilter : fields) {
			filters.push_back(CardDavPropFilter(mDomain, propFilter, exactMatch));
		}
	}

	return filters;
}

void CardDavMagicSearchPlugin::setMoreResultsAvailable() {
	mMoreResultsAvailable = true;
}

LINPHONE_END_NAMESPACE
