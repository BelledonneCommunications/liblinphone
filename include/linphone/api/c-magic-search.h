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

#ifndef _L_C_MAGIC_SEARCH_H_
#define _L_C_MAGIC_SEARCH_H_

#include "linphone/api/c-magic-search-cbs.h"
#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
	extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup misc
 * @{
 */

/**
 * Constructs a LinphoneMagicSearch object
 * @param core a #LinphoneCore object @notnil
 * @return a #LinphoneMagicSearch object @notnil
 **/
LINPHONE_PUBLIC LinphoneMagicSearch *linphone_magic_search_new (LinphoneCore *core);

/**
 * Increment reference count of LinphoneMagicSearch object.
 * @param magic_search a #LinphoneMagicSearch object @notnil
 * @return the same #LinphoneMagicSearch object @notnil
 **/
LINPHONE_PUBLIC LinphoneMagicSearch *linphone_magic_search_ref (LinphoneMagicSearch *magic_search);

/**
 * Decrement reference count of LinphoneMagicSearch object. When dropped to zero, memory is freed.
 * @param magic_search a #LinphoneMagicSearch object @notnil
 **/
LINPHONE_PUBLIC void linphone_magic_search_unref (LinphoneMagicSearch *magic_search);

/**
 * Add a listener in order to be notified of #LinphoneMagicSearch events.
 * @param magic_search #LinphoneMagicSearch object to monitor. @notnil
 * @param cbs A #LinphoneMagicSearchCbs object holding the callbacks you need. @notnil
 */
LINPHONE_PUBLIC void linphone_magic_search_add_callbacks(LinphoneMagicSearch *magic_search, LinphoneMagicSearchCbs *cbs);

/**
 * Remove a listener from a #LinphoneMagicSearch
 * @param magic_search #LinphoneMagicSearch object @notnil
 * @param cbs #LinphoneMagicSearchCbs object to remove. @notnil
 */
LINPHONE_PUBLIC void linphone_magic_search_remove_callbacks(LinphoneMagicSearch *magic_search, LinphoneMagicSearchCbs *cbs);

/**
 * Gets the current LinphoneMagicSearchCbs.
 * This is meant only to be called from a callback to be able to get the user_data associated with the #LinphoneMagicSearchCbs that is calling the callback.
 * @param magic_search #LinphoneMagicSearch object @notnil
 * @return The #LinphoneMagicSearchCbs that has called the last callback. @maybenil
 */
LINPHONE_PUBLIC LinphoneMagicSearchCbs *linphone_magic_search_get_current_callbacks(const LinphoneMagicSearch *magic_search);

/**
 * Set the minimum value used to calculate the weight in search
 * @param magic_search a #LinphoneMagicSearch object @notnil
 * @param weight minimum weight
 **/
LINPHONE_PUBLIC void linphone_magic_search_set_min_weight (LinphoneMagicSearch *magic_search, unsigned int weight);

/**
 * Get the minimum value used to calculate the weight in search
 * @param magic_search a #LinphoneMagicSearch object @notnil
 * @return the minimum value used to calculate the weight in search
 **/
LINPHONE_PUBLIC unsigned int linphone_magic_search_get_min_weight (const LinphoneMagicSearch *magic_search);

/**
 * Set the maximum value used to calculate the weight in search
 * @param magic_search a #LinphoneMagicSearch object @notnil
 * @param weight maximum weight
 **/
LINPHONE_PUBLIC void linphone_magic_search_set_max_weight (LinphoneMagicSearch *magic_search, unsigned int weight);

/**
 * Get the maximum value used to calculate the weight in search
 * @param magic_search a #LinphoneMagicSearch object @notnil
 * @return the maximum value used to calculate the weight in search
 **/
LINPHONE_PUBLIC unsigned int linphone_magic_search_get_max_weight (const LinphoneMagicSearch *magic_search);

/**
 * Get the delimiter used for the search.
 * @param magic_search a #LinphoneMagicSearch object @notnil
 * @return the delimiter used to find matched filter word @maybenil
 **/
LINPHONE_PUBLIC const char *linphone_magic_search_get_delimiter (const LinphoneMagicSearch *magic_search);

/**
 * Set the delimiter used to find matched filter word
 * @param magic_search a #LinphoneMagicSearch object @notnil
 * @param delimiter delimiter (example "-_.,") @maybenil
 **/
LINPHONE_PUBLIC void linphone_magic_search_set_delimiter (LinphoneMagicSearch *magic_search, const char *delimiter);

/**
 * Returns whether the delimiter is being used for the search
 * @param magic_search a #LinphoneMagicSearch object @notnil
 * @return if the delimiter search is used
 **/
LINPHONE_PUBLIC bool_t linphone_magic_search_get_use_delimiter (LinphoneMagicSearch *magic_search);

/**
 * Enable or disable the delimiter in search
 * @param magic_search a #LinphoneMagicSearch object @notnil
 * @param enable TRUE to use the delimiter, FALSE otherwise
 **/
LINPHONE_PUBLIC void linphone_magic_search_set_use_delimiter (LinphoneMagicSearch *magic_search, bool_t enable);

/**
 * Get the number of maximum search result the search will return
 * @param magic_search a #LinphoneMagicSearch object @notnil
 * @return the number of the maximum #LinphoneSearchResult which will be returned
 **/
LINPHONE_PUBLIC unsigned int linphone_magic_search_get_search_limit (const LinphoneMagicSearch *magic_search);

/**
 * Set the number of the maximum SearchResult which will be returned
 * @param magic_search a #LinphoneMagicSearch object @notnil
 * @param limit the maximum number of #LinphoneSearchResult the search will return
 **/
LINPHONE_PUBLIC void linphone_magic_search_set_search_limit (LinphoneMagicSearch *magic_search, unsigned int limit);

/**
 * Return whether or not the search is limited
 * @param magic_search a #LinphoneMagicSearch object @notnil
 * @return TRUE if the search is limited, FALSE otherwise
 **/
LINPHONE_PUBLIC bool_t linphone_magic_search_get_limited_search (const LinphoneMagicSearch *magic_search);

/**
 * Enable or disable the limited search
 * @param magic_search a #LinphoneMagicSearch object @notnil
 * @param limited TRUE to limit the search, FALSE otherwise
 **/
LINPHONE_PUBLIC void linphone_magic_search_set_limited_search (LinphoneMagicSearch *magic_search, bool_t limited);

/**
 * Reset the cache to begin a new search
 * @param magic_search a #LinphoneMagicSearch object @notnil
 **/
LINPHONE_PUBLIC void linphone_magic_search_reset_search_cache (LinphoneMagicSearch *magic_search);

/**
 * Create a sorted list of SearchResult from SipUri, Contact name,
 * Contact displayname, Contact phone number, which match with a filter word
 * The last item list will be an address formed with "filter" if a proxy config exist
 * During the first search, a cache is created and used for the next search
 * Use linphone_magic_search_reset_search_cache() to begin a new search
 * @param magic_search a #LinphoneMagicSearch object @notnil
 * @param filter word we search @maybenil
 * @param domain domain which we want to search only @maybenil
 * - NULL or "" for searching in all contact
 * - "*" for searching in contact with sip SipUri
 * - "yourdomain" for searching in contact from "yourdomain" domain
 * @return sorted list of \bctbx_list{LinphoneSearchResult} @tobefreed @notnil
 **/
LINPHONE_PUBLIC bctbx_list_t *linphone_magic_search_get_contact_list_from_filter (
	LinphoneMagicSearch *magic_search,
	const char *filter,
	const char *domain
);

/**
 * Create a sorted list of SearchResult asynchronous from SipUri, Contact name,
 * Contact displayname, Contact phone number, which match with a filter word
 * The last item list will be an address formed with "filter" if a proxy config exist
 * During the first search, a cache is created and used for the next search
 * Use linphone_magic_search_reset_search_cache() to begin a new search
 * @param magic_search a #LinphoneMagicSearch object @notnil
 * @param filter word we search @maybenil
 * @param domain domain which we want to search only @maybenil
 ** NULL or "" for searching in all contact
 ** "*" for searching in contact with sip SipUri
 ** "yourdomain" for searching in contact from "yourdomain" domain
 **/
//typedef void (MagicSearchCb)(const void *, void *);
LINPHONE_PUBLIC void linphone_magic_search_get_contact_list_from_filter_async (
	LinphoneMagicSearch *magic_search,
	const char *filter,
	const char *domain
);
/**
 * 
 * @param magic_search a #LinphoneMagicSearch object @notnil
 * @return sorted list of \bctbx_list{LinphoneSearchResult} @tobefreed @notnil
 * 
 **/
LINPHONE_PUBLIC bctbx_list_t* linphone_magic_search_get_last_search(const LinphoneMagicSearch *magic_search);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif // _L_C_MAGIC_SEARCH_H_
