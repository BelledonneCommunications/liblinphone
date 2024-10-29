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

#include "linphone/api/c-magic-search.h"
#include "c-wrapper/c-wrapper.h"
#include "linphone/api/c-magic-search-cbs.h"
#include "linphone/wrapper_utils.h"
#include "search/magic-search.h"

// =============================================================================

using namespace std;
using namespace LinphonePrivate;

void _linphone_magic_search_notify_search_results_received(LinphoneMagicSearch *magic_search) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS_NO_ARG(MagicSearch, MagicSearch::toCpp(magic_search),
	                                         linphone_magic_search_cbs_get_search_results_received);
}

void _linphone_magic_search_notify_ldap_have_more_results(LinphoneMagicSearch *magic_search, LinphoneLdap *ldap) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(MagicSearch, MagicSearch::toCpp(magic_search),
	                                  linphone_magic_search_cbs_get_ldap_have_more_results, ldap);
}

void _linphone_magic_search_notify_more_results_available(LinphoneMagicSearch *magic_search,
                                                          LinphoneMagicSearchSource source) {
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(MagicSearch, MagicSearch::toCpp(magic_search),
	                                  linphone_magic_search_cbs_get_more_results_available, source);
}

LinphoneMagicSearch *linphone_core_create_magic_search(LinphoneCore *lc) {
	return MagicSearch::createCObject(lc ? L_GET_CPP_PTR_FROM_C_OBJECT(lc) : nullptr);
}

LinphoneMagicSearch *linphone_magic_search_new(LinphoneCore *lc) {
	return linphone_core_create_magic_search(lc);
}

LinphoneMagicSearch *linphone_magic_search_ref(LinphoneMagicSearch *magic_search) {
	MagicSearch::toCpp(magic_search)->ref();
	return magic_search;
}

void linphone_magic_search_unref(LinphoneMagicSearch *magic_search) {
	MagicSearch::toCpp(magic_search)->unref();
}

void linphone_magic_search_add_callbacks(LinphoneMagicSearch *magic_search, LinphoneMagicSearchCbs *cbs) {
	MagicSearch::toCpp(magic_search)->addCallbacks(MagicSearchCbs::toCpp(cbs)->getSharedFromThis());
}

void linphone_magic_search_remove_callbacks(LinphoneMagicSearch *magic_search, LinphoneMagicSearchCbs *cbs) {
	MagicSearch::toCpp(magic_search)->removeCallbacks(MagicSearchCbs::toCpp(cbs)->getSharedFromThis());
}

LinphoneMagicSearchCbs *linphone_magic_search_get_current_callbacks(const LinphoneMagicSearch *magic_search) {
	return MagicSearch::toCpp(magic_search)->getCurrentCallbacks()->toC();
}

void linphone_magic_search_set_current_callbacks(LinphoneMagicSearch *magic_search, LinphoneMagicSearchCbs *cbs) {
	MagicSearch::toCpp(magic_search)->setCurrentCallbacks(MagicSearchCbs::toCpp(cbs)->getSharedFromThis());
}

const bctbx_list_t *linphone_magic_search_get_callbacks_list(const LinphoneMagicSearch *magic_search) {
	return MagicSearch::toCpp(magic_search)->getCCallbacksList();
}

// =============================================================================
// Getter and setters
// =============================================================================

void linphone_magic_search_set_min_weight(LinphoneMagicSearch *magic_search, unsigned int weight) {
	MagicSearch::toCpp(magic_search)->setMinWeight(weight);
}

unsigned int linphone_magic_search_get_min_weight(const LinphoneMagicSearch *magic_search) {
	return MagicSearch::toCpp(magic_search)->getMinWeight();
}

void linphone_magic_search_set_max_weight(LinphoneMagicSearch *magic_search, unsigned int weight) {
	MagicSearch::toCpp(magic_search)->setMaxWeight(weight);
}

unsigned int linphone_magic_search_get_max_weight(const LinphoneMagicSearch *magic_search) {
	return MagicSearch::toCpp(magic_search)->getMaxWeight();
}

const char *linphone_magic_search_get_delimiter(const LinphoneMagicSearch *magic_search) {
	return L_STRING_TO_C(MagicSearch::toCpp(magic_search)->getDelimiter());
}

void linphone_magic_search_set_delimiter(LinphoneMagicSearch *magic_search, const char *delimiter) {
	MagicSearch::toCpp(magic_search)->setDelimiter(L_C_TO_STRING(delimiter));
}

bool_t linphone_magic_search_get_use_delimiter(LinphoneMagicSearch *magic_search) {
	return MagicSearch::toCpp(magic_search)->getUseDelimiter();
}

void linphone_magic_search_set_use_delimiter(LinphoneMagicSearch *magic_search, bool_t enable) {
	MagicSearch::toCpp(magic_search)->setUseDelimiter(!!enable);
}

unsigned int linphone_magic_search_get_search_limit(const LinphoneMagicSearch *magic_search) {
	return MagicSearch::toCpp(magic_search)->getSearchLimit();
}

void linphone_magic_search_set_search_limit(LinphoneMagicSearch *magic_search, unsigned int limit) {
	MagicSearch::toCpp(magic_search)->setSearchLimit(limit);
}

bool_t linphone_magic_search_get_limited_search(const LinphoneMagicSearch *magic_search) {
	return MagicSearch::toCpp(magic_search)->getLimitedSearch();
}

void linphone_magic_search_set_limited_search(LinphoneMagicSearch *magic_search, bool_t limited) {
	MagicSearch::toCpp(magic_search)->setLimitedSearch(!!limited);
}

void linphone_magic_search_reset_search_cache(LinphoneMagicSearch *magic_search) {
	MagicSearch::toCpp(magic_search)->resetSearchCache();
}

bctbx_list_t *linphone_magic_search_get_contacts_list(LinphoneMagicSearch *magic_search,
                                                      const char *filter,
                                                      const char *domain,
                                                      int sourceFlags,
                                                      LinphoneMagicSearchAggregation aggregation) {
	return SearchResult::getCListFromCppList(
	    MagicSearch::toCpp(magic_search)
	        ->getContactListFromFilter(L_C_TO_STRING(filter), L_C_TO_STRING(domain), sourceFlags, aggregation));
}

void linphone_magic_search_get_contacts_list_async(LinphoneMagicSearch *magic_search,
                                                   const char *filter,
                                                   const char *domain,
                                                   int sourceFlags,
                                                   LinphoneMagicSearchAggregation aggregation) {
	MagicSearch::toCpp(magic_search)
	    ->getContactListFromFilterAsync(L_C_TO_STRING(filter), L_C_TO_STRING(domain), sourceFlags, aggregation);
}

LINPHONE_PUBLIC bctbx_list_t *linphone_magic_search_get_last_search(const LinphoneMagicSearch *magic_search) {
	return SearchResult::getCListFromCppList(MagicSearch::toCpp(magic_search)->getLastSearch());
}
