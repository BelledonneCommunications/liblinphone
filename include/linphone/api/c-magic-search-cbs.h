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

#ifndef _L_C_MAGIC_SEARCH_CBS_H_
#define _L_C_MAGIC_SEARCH_CBS_H_

#include "linphone/api/c-callbacks.h"
#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup misc
 * @{
 */

LinphoneMagicSearchCbs *linphone_magic_search_cbs_new(void);

/**
 * Acquire a reference to the chat message callbacks object.
 * @param cbs The #LinphoneMagicSearchCbs object @notnil
 * @return The same chat message callbacks object
 **/
LINPHONE_PUBLIC LinphoneMagicSearchCbs *linphone_magic_search_cbs_ref(LinphoneMagicSearchCbs *cbs);

/**
 * Release reference to the chat message callbacks object.
 * @param cbs The #LinphoneMagicSearchCbs object @notnil
 **/
LINPHONE_PUBLIC void linphone_magic_search_cbs_unref(LinphoneMagicSearchCbs *cbs);

/**
 * Retrieve the user pointer associated with the chat message callbacks object.
 * @param cbs The #LinphoneMagicSearchCbs object @notnil
 * @return The user pointer associated with the chat message callbacks object. @maybenil
 **/
LINPHONE_PUBLIC void *linphone_magic_search_cbs_get_user_data(const LinphoneMagicSearchCbs *cbs);

/**
 * Assign a user pointer to the chat message callbacks object.
 * @param cbs The #LinphoneMagicSearchCbs object @notnil
 * @param user_data The user pointer to associate with the chat message callbacks object. @maybenil
 **/
LINPHONE_PUBLIC void linphone_magic_search_cbs_set_user_data(LinphoneMagicSearchCbs *cbs, void *user_data);

/**
 * Get the received results callback.
 * @param cbs #LinphoneMagicSearchCbs object. @notnil
 * @return The current result received callback.
 */
LINPHONE_PUBLIC LinphoneMagicSearchCbsSearchResultsReceivedCb
linphone_magic_search_cbs_get_search_results_received(const LinphoneMagicSearchCbs *cbs);

/**
 * Set the received results  callback.
 * @param cbs #LinphoneMagicSearchCbs object. @notnil
 * @param cb The received results callback to be used.
 */
LINPHONE_PUBLIC void
linphone_magic_search_cbs_set_search_results_received(LinphoneMagicSearchCbs *cbs,
                                                      LinphoneMagicSearchCbsSearchResultsReceivedCb cb);

/**
 * Get the ldap callback on having more results.
 * @param cbs #LinphoneMagicSearchCbs object. @notnil
 * @return The ldap callback on having more results.
 * @deprecated 18/11/2024 use linphone_magic_search_cbs_get_more_results_available() instead.
 */
LINPHONE_PUBLIC LinphoneMagicSearchCbsLdapHaveMoreResultsCb
linphone_magic_search_cbs_get_ldap_have_more_results(const LinphoneMagicSearchCbs *cbs);

/**
 * Set the ldap callback on having more results.
 * @param cbs #LinphoneMagicSearchCbs object. @notnil
 * @param cb The ldap callback on having more results.
 * @deprecated 18/11/2024 use linphone_magic_search_cbs_set_more_results_available() instead.
 */
LINPHONE_PUBLIC void
linphone_magic_search_cbs_set_ldap_have_more_results(LinphoneMagicSearchCbs *cbs,
                                                     LinphoneMagicSearchCbsLdapHaveMoreResultsCb cb);

/**
 * Gets the callback notifying more results for a source flag are available.
 * @param cbs #LinphoneMagicSearchCbs object. @notnil
 * @return The more results available callback.
 */
LINPHONE_PUBLIC LinphoneMagicSearchCbsMoreResultsAvailableCb
linphone_magic_search_cbs_get_more_results_available(const LinphoneMagicSearchCbs *cbs);

/**
 * Sets the callback notifying that more results for a source flag are available.
 * @param cbs #LinphoneMagicSearchCbs object. @notnil
 * @return The more results available callback.
 */
LINPHONE_PUBLIC void
linphone_magic_search_cbs_set_more_results_available(LinphoneMagicSearchCbs *cbs,
                                                     LinphoneMagicSearchCbsMoreResultsAvailableCb cb);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_CHAT_MESSAGE_CBS_H_
