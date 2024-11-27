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

#ifndef _L_CARDDAV_MAGIC_SEARCH_PLUGIN_H_
#define _L_CARDDAV_MAGIC_SEARCH_PLUGIN_H_

#include "core/core.h"
#include "friend/friend.h"
#include "search/magic-search-plugin.h"
#include "search/magic-search.h"
#include "search/search-request.h"
#include "vcard/carddav-context.h"
#include "vcard/carddav-params.h"
#include "vcard/carddav-query.h"

LINPHONE_BEGIN_NAMESPACE

class CardDavMagicSearchPlugin : public MagicSearchPlugin,
                                 public std::enable_shared_from_this<CardDavMagicSearchPlugin> {
public:
	CardDavMagicSearchPlugin(const std::shared_ptr<Core> &core,
	                         MagicSearch &magicSearch,
	                         std::shared_ptr<CardDavParams> params);

	void stop() override;

	std::list<std::shared_ptr<SearchResult>> startSearch(const std::string &filter, const std::string &domain) override;

	void startSearchAsync(const std::string &filter, const std::string &domain, SearchAsyncData *asyncData) override;

	void sendQueryAgainAfterDiscoveryProcess();

	void notifyError(const std::string &errorMessage);

	void processResults(const std::list<std::shared_ptr<Friend>> &friends);

	void setMoreResultsAvailable();

private:
	std::list<CardDavPropFilter> computePropFilters() const;

	std::shared_ptr<CardDAVContext> mCardDavContext;
	std::string mFilter;
	std::string mDomain;
	bool mMoreResultsAvailable = false;

	std::shared_ptr<CardDavParams> mParams;

	SearchAsyncData *mAsyncData;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CARDDAV_MAGIC_SEARCH_PLUGIN_H_
