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
#include "private_functions.h"
#include "search/magic-search.h"

// =============================================================================

using namespace std;
using namespace LinphonePrivate;

static void _linphone_magic_search_constructor(LinphoneMagicSearch *magic);
static void _linphone_magic_search_destructor(LinphoneMagicSearch *magic);

L_DECLARE_C_OBJECT_IMPL_WITH_XTORS(MagicSearch,
                                   _linphone_magic_search_constructor,
                                   _linphone_magic_search_destructor,

                                   bctbx_list_t *callbacks;
                                   LinphoneMagicSearchCbs * currentCbs;);

static void _linphone_magic_search_constructor(LinphoneMagicSearch *) {
}

static void _linphone_magic_search_destructor(LinphoneMagicSearch *magic) {
	bctbx_list_free_with_data(magic->callbacks, (bctbx_list_free_func)linphone_magic_search_cbs_unref);
	magic->callbacks = nullptr;
}

LinphoneMagicSearch *linphone_core_create_magic_search(LinphoneCore *lc) {
	shared_ptr<LinphonePrivate::MagicSearch> cppPtr =
	    make_shared<LinphonePrivate::MagicSearch>(L_GET_CPP_PTR_FROM_C_OBJECT(lc));

	LinphoneMagicSearch *object = L_INIT(MagicSearch);
	L_SET_CPP_PTR_FROM_C_OBJECT(object, cppPtr);
	return object;
}

LinphoneMagicSearch *linphone_magic_search_new(LinphoneCore *lc) {
	return linphone_core_create_magic_search(lc);
}

LinphoneMagicSearch *linphone_magic_search_ref(LinphoneMagicSearch *magic_search) {
	belle_sip_object_ref(magic_search);
	return magic_search;
}

void linphone_magic_search_unref(LinphoneMagicSearch *magic_search) {
	belle_sip_object_unref(magic_search);
}

void linphone_magic_search_add_callbacks(LinphoneMagicSearch *magic_search, LinphoneMagicSearchCbs *cbs) {
	magic_search->callbacks = bctbx_list_append(magic_search->callbacks, linphone_magic_search_cbs_ref(cbs));
}

void linphone_magic_search_remove_callbacks(LinphoneMagicSearch *magic_search, LinphoneMagicSearchCbs *cbs) {
	magic_search->callbacks = bctbx_list_remove(magic_search->callbacks, cbs);
	linphone_magic_search_cbs_unref(cbs);
}

LinphoneMagicSearchCbs *linphone_magic_search_get_current_callbacks(const LinphoneMagicSearch *msg) {
	return msg->currentCbs;
}

void linphone_magic_search_set_current_callbacks(LinphoneMagicSearch *msg, LinphoneMagicSearchCbs *cbs) {
	msg->currentCbs = cbs;
}

const bctbx_list_t *linphone_magic_search_get_callbacks_list(const LinphoneMagicSearch *magic_search) {
	return magic_search->callbacks;
}

#define NOTIFY_IF_EXIST(cbName, functionName, ...)                                                                     \
	bctbx_list_t *callbacksCopy = bctbx_list_copy(linphone_magic_search_get_callbacks_list(magic_search));             \
	for (bctbx_list_t *it = callbacksCopy; it; it = bctbx_list_next(it)) {                                             \
		linphone_magic_search_set_current_callbacks(magic_search,                                                      \
		                                            static_cast<LinphoneMagicSearchCbs *>(bctbx_list_get_data(it)));   \
		LinphoneMagicSearchCbs##cbName##Cb cb =                                                                        \
		    linphone_magic_search_cbs_get_##functionName(linphone_magic_search_get_current_callbacks(magic_search));   \
		if (cb) cb(__VA_ARGS__);                                                                                       \
	}                                                                                                                  \
	linphone_magic_search_set_current_callbacks(magic_search, nullptr);                                                \
	bctbx_list_free(callbacksCopy);

void _linphone_magic_search_notify_search_results_received(LinphoneMagicSearch *magic_search) {
	NOTIFY_IF_EXIST(SearchResultsReceived, search_results_received, magic_search)
}

void _linphone_magic_search_notify_ldap_have_more_results(LinphoneMagicSearch *magic_search, LinphoneLdap *ldap) {
	NOTIFY_IF_EXIST(LdapHaveMoreResults, ldap_have_more_results, magic_search, ldap)
}

// =============================================================================
// Getter and setters
// =============================================================================

void linphone_magic_search_set_min_weight(LinphoneMagicSearch *magic_search, unsigned int weight) {
	L_GET_CPP_PTR_FROM_C_OBJECT(magic_search)->setMinWeight(weight);
}

unsigned int linphone_magic_search_get_min_weight(const LinphoneMagicSearch *magic_search) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(magic_search)->getMinWeight();
}

void linphone_magic_search_set_max_weight(LinphoneMagicSearch *magic_search, unsigned int weight) {
	L_GET_CPP_PTR_FROM_C_OBJECT(magic_search)->setMaxWeight(weight);
}

unsigned int linphone_magic_search_get_max_weight(const LinphoneMagicSearch *magic_search) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(magic_search)->getMaxWeight();
}

const char *linphone_magic_search_get_delimiter(const LinphoneMagicSearch *magic_search) {
	return L_STRING_TO_C(L_GET_CPP_PTR_FROM_C_OBJECT(magic_search)->getDelimiter());
}

void linphone_magic_search_set_delimiter(LinphoneMagicSearch *magic_search, const char *delimiter) {
	L_GET_CPP_PTR_FROM_C_OBJECT(magic_search)->setDelimiter(L_C_TO_STRING(delimiter));
}

bool_t linphone_magic_search_get_use_delimiter(LinphoneMagicSearch *magic_search) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(magic_search)->getUseDelimiter();
}

void linphone_magic_search_set_use_delimiter(LinphoneMagicSearch *magic_search, bool_t enable) {
	L_GET_CPP_PTR_FROM_C_OBJECT(magic_search)->setUseDelimiter(!!enable);
}

unsigned int linphone_magic_search_get_search_limit(const LinphoneMagicSearch *magic_search) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(magic_search)->getSearchLimit();
}

void linphone_magic_search_set_search_limit(LinphoneMagicSearch *magic_search, unsigned int limit) {
	L_GET_CPP_PTR_FROM_C_OBJECT(magic_search)->setSearchLimit(limit);
}

bool_t linphone_magic_search_get_limited_search(const LinphoneMagicSearch *magic_search) {
	return L_GET_CPP_PTR_FROM_C_OBJECT(magic_search)->getLimitedSearch();
}

void linphone_magic_search_set_limited_search(LinphoneMagicSearch *magic_search, bool_t limited) {
	L_GET_CPP_PTR_FROM_C_OBJECT(magic_search)->setLimitedSearch(!!limited);
}

void linphone_magic_search_reset_search_cache(LinphoneMagicSearch *magic_search) {
	L_GET_CPP_PTR_FROM_C_OBJECT(magic_search)->resetSearchCache();
}

bctbx_list_t *linphone_magic_search_get_contacts_list(LinphoneMagicSearch *magic_search,
                                                      const char *filter,
                                                      const char *domain,
                                                      int sourceFlags,
                                                      LinphoneMagicSearchAggregation aggregation) {
	return SearchResult::getCListFromCppList(
	    L_GET_CPP_PTR_FROM_C_OBJECT(magic_search)
	        ->getContactListFromFilter(L_C_TO_STRING(filter), L_C_TO_STRING(domain), sourceFlags, aggregation));
}

bctbx_list_t *linphone_magic_search_get_contacts(LinphoneMagicSearch *magic_search,
                                                 const char *filter,
                                                 const char *domain,
                                                 int sourceFlags) {
	return SearchResult::getCListFromCppList(L_GET_CPP_PTR_FROM_C_OBJECT(magic_search)
	                                             ->getContactListFromFilter(L_C_TO_STRING(filter),
	                                                                        L_C_TO_STRING(domain), sourceFlags,
	                                                                        LinphoneMagicSearchAggregationNone));
}

bctbx_list_t *linphone_magic_search_get_contact_list_from_filter(LinphoneMagicSearch *magic_search,
                                                                 const char *filter,
                                                                 const char *domain) {
	return SearchResult::getCListFromCppList(
	    L_GET_CPP_PTR_FROM_C_OBJECT(magic_search)
	        ->getContactListFromFilter(L_C_TO_STRING(filter), L_C_TO_STRING(domain), LinphoneMagicSearchSourceAll,
	                                   LinphoneMagicSearchAggregationNone));
}

typedef void (*MagicSearchCb)(void *, void *);

void linphone_magic_search_get_contacts_list_async(LinphoneMagicSearch *magic_search,
                                                   const char *filter,
                                                   const char *domain,
                                                   int sourceFlags,
                                                   LinphoneMagicSearchAggregation aggregation) {
	L_GET_CPP_PTR_FROM_C_OBJECT(magic_search)
	    ->getContactListFromFilterAsync(L_C_TO_STRING(filter), L_C_TO_STRING(domain), sourceFlags, aggregation);
}

void linphone_magic_search_get_contacts_async(LinphoneMagicSearch *magic_search,
                                              const char *filter,
                                              const char *domain,
                                              int sourceFlags) {
	L_GET_CPP_PTR_FROM_C_OBJECT(magic_search)
	    ->getContactListFromFilterAsync(L_C_TO_STRING(filter), L_C_TO_STRING(domain), sourceFlags,
	                                    LinphoneMagicSearchAggregationNone);
}

void linphone_magic_search_get_contact_list_from_filter_async(LinphoneMagicSearch *magic_search,
                                                              const char *filter,
                                                              const char *domain) {
	L_GET_CPP_PTR_FROM_C_OBJECT(magic_search)
	    ->getContactListFromFilterAsync(L_C_TO_STRING(filter), L_C_TO_STRING(domain), LinphoneMagicSearchSourceAll,
	                                    LinphoneMagicSearchAggregationNone);
}

LINPHONE_PUBLIC bctbx_list_t *linphone_magic_search_get_last_search(const LinphoneMagicSearch *magic_search) {
	return SearchResult::getCListFromCppList(L_GET_CPP_PTR_FROM_C_OBJECT(magic_search)->getLastSearch());
}
