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
#include "friend/friend.h"
#include "ldap.h"

LINPHONE_BEGIN_NAMESPACE

using namespace std;

static void resultsCb(list<shared_ptr<Friend>> friends, void *data, bool_t haveMoreResults) {
	LdapMagicSearchPlugin *ldapPlugin = (LdapMagicSearchPlugin *)data;
	lInfo() << "[Magic Search][LDAP] Processing [" << friends.size() << "] friends";

	list<shared_ptr<SearchResult>> resultList;
	string domainFilter = ldapPlugin->getDomain();
	bool filterPluginResults =
	    ldapPlugin->getMagicSearch().filterPluginsResults() || (!domainFilter.empty() && domainFilter != "*");

	for (auto &lFriend : friends) {
		if (filterPluginResults) {
			auto found = ldapPlugin->getMagicSearch().searchInFriend(lFriend, domainFilter, ldapPlugin->getSource());
			if (resultList.empty()) {
				resultList = found;
			} else if (!found.empty()) {
				resultList.splice(resultList.end(), found);
			}
		} else {
			auto result =
			    ldapPlugin->getMagicSearch().createResultFromFriend(lFriend, domainFilter, ldapPlugin->getSource());
			resultList.push_back(result);
		}
	}

	if (filterPluginResults) {
		lInfo() << "[Magic Search][LDAP] Found " << resultList.size() << " results (after local filter is applied)";
	} else {
		lInfo() << "[Magic Search][LDAP] Found " << resultList.size() << " results (no local filter was applied)";
	}
	ldapPlugin->setResults(resultList);

	if (haveMoreResults) {
		lInfo() << "[Magic Search][LDAP] More results are available.";
		shared_ptr<LdapParams> ldapParams = ldapPlugin->getLdapProvider()->getLdapServer();
		shared_ptr<Ldap> ldap = Ldap::create(ldapPlugin->getCore(), ldapParams);
		// Legacy
		_linphone_magic_search_notify_ldap_have_more_results(ldapPlugin->getMagicSearch().toC(), ldap->toC());

		_linphone_magic_search_notify_more_results_available(ldapPlugin->getMagicSearch().toC(),
		                                                     ldapPlugin->getSource());
	}
	ldapPlugin->setHasEnded(true);
}

void LdapMagicSearchPlugin::stop() {
	setHasEnded(true);
	mProvider = nullptr;
}

list<shared_ptr<SearchResult>> LdapMagicSearchPlugin::startSearch(const string &filter, const string &domain) {
	if (linphone_core_is_network_reachable(getCore()->getCCore())) {
		setHasEnded(false);
		return getAddressFromLDAPServer(filter, domain);
	}
	setHasEnded(true);
	return list<shared_ptr<SearchResult>>();
}

void LdapMagicSearchPlugin::startSearchAsync(const string &filter, const string &domain, SearchAsyncData *asyncData) {
	if (linphone_core_is_network_reachable(getCore()->getCCore())) {
		setHasEnded(false);
		getAddressFromLDAPServerStartAsync(filter, domain, asyncData);
	} else {
		setHasEnded(true);
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
