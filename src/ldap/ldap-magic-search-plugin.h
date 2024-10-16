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

#ifndef _L_LDAP_MAGIC_SEARCH_PLUGIN_H_
#define _L_LDAP_MAGIC_SEARCH_PLUGIN_H_

#include "core/core.h"
#include "ldap-contact-provider.h"
#include "search/magic-search-plugin.h"
#include "search/magic-search.h"
#include "search/search-request.h"

LINPHONE_BEGIN_NAMESPACE

class LdapMagicSearchPlugin : public MagicSearchPlugin {
public:
	LdapMagicSearchPlugin(const std::shared_ptr<Core> &core,
	                      MagicSearch &magicSearch,
	                      std::shared_ptr<LdapContactProvider> provider)
	    : MagicSearchPlugin(core, magicSearch, LinphoneMagicSearchSourceLdapServers) {
		mProvider = provider;
	}

	void stop() override;

	std::list<std::shared_ptr<SearchResult>> startSearch(const std::string &filter, const std::string &domain) override;

	void startSearchAsync(const std::string &filter, const std::string &domain, SearchAsyncData *asyncData) override;

	std::list<std::shared_ptr<SearchResult>> getAddressFromLDAPServer(const std::string &filter,
	                                                                  const std::string &withDomain);

	void getAddressFromLDAPServerStartAsync(const std::string &filter,
	                                        const std::string &withDomain,
	                                        SearchAsyncData *asyncData);

	const std::shared_ptr<LdapContactProvider> &getLdapProvider() const {
		return mProvider;
	}

private:
	std::shared_ptr<LdapContactProvider> mProvider;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_LDAP_MAGIC_SEARCH_PLUGIN_H_
