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

#include "ldap-magic-search-plugin.h"

LINPHONE_BEGIN_NAMESPACE

using namespace std;

static void resultsCb(std::list<std::shared_ptr<SearchResult>> searchResults, void *data, bool_t haveMoreResults) {
	LdapMagicSearchPlugin *ldapPlugin = (LdapMagicSearchPlugin *)data;
	lInfo() << "[Magic Search][LDAP] Processing results";

	for (auto searchResult : searchResults) {
		if (searchResult) {
			if (ldapPlugin->getFilter().empty() && ldapPlugin->getDomain().empty()) {
				searchResult->setWeight(0);
				ldapPlugin->addToResults(searchResult);
			} else { // We have constraints : add result with weight
				unsigned int weight = ldapPlugin->getMagicSearch().searchInAddress(
				    searchResult->getAddress(), ldapPlugin->getFilter(), ldapPlugin->getDomain());
				if (weight >= ldapPlugin->getMagicSearch().getMinWeight()) {
					searchResult->setWeight(weight);
					ldapPlugin->addToResults(searchResult);
				}
			}
		}
	}

	lInfo() << "[Magic Search][LDAP] Found " << ldapPlugin->getResults().size() << " results in LDAP."
	        << (haveMoreResults ? " More results are available." : "");
	if (haveMoreResults) {
		std::shared_ptr<Ldap> ldap = ldapPlugin->getLdapProvider()->getLdapServer();
		_linphone_magic_search_notify_ldap_have_more_results(ldapPlugin->getMagicSearch().toC(), ldap->toC());
	}
	ldapPlugin->setHasEnded(true);
}

void LdapMagicSearchPlugin::stop() {
	mProvider = nullptr;
}

list<shared_ptr<SearchResult>> LdapMagicSearchPlugin::startSearch(const string &filter, const string &domain) {
	if (linphone_core_is_network_reachable(getCore()->getCCore())) {
		return getAddressFromLDAPServer(filter, domain);
	}
	return list<shared_ptr<SearchResult>>();
}

void LdapMagicSearchPlugin::startSearchAsync(const string &filter, const string &domain, SearchAsyncData *asyncData) {
	if (linphone_core_is_network_reachable(getCore()->getCCore())) {
		getAddressFromLDAPServerStartAsync(filter, domain, asyncData);
	}
}

// Synchronous search by iterate directly on main loop if there are async processes
// Provided for convenience
list<shared_ptr<SearchResult>> LdapMagicSearchPlugin::getAddressFromLDAPServer(const string &filter,
                                                                               const string &withDomain) {
	SearchAsyncData asyncData;
	// Start async search
	SearchRequest request(filter, withDomain, LinphoneMagicSearchSourceAll, LinphoneMagicSearchAggregationNone);
	asyncData.pushRequest(request);
	asyncData.setSearchRequest(request);
	getAddressFromLDAPServerStartAsync(filter, withDomain, &asyncData);
	asyncData.initStartTime();

	// Loop in core iterate till having all (good/bad) results
	while (!getMagicSearch().arePluginsProcessingDone(&asyncData)) {
		linphone_core_iterate(this->getCore()->getCCore());
	}
	getMagicSearch().mergeResults(&asyncData);

	return asyncData.getSearchResults();
}

void LdapMagicSearchPlugin::getAddressFromLDAPServerStartAsync(const string &filter,
                                                               const string &withDomain,
                                                               SearchAsyncData *asyncData) {
	if (mProvider && mProvider->getCurrentAction() != LdapContactProvider::ACTION_ERROR) {
		setTimeout((int64_t)mProvider->configValueToInt("timeout"));
		setFilter(filter);
		setDomain(withDomain);
		setHasEnded(!mProvider->search(filter, resultsCb, this, asyncData->getRequestHistory()));
	} else {
		setHasEnded(true);
	}
}

LINPHONE_END_NAMESPACE
