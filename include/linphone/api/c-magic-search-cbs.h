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

LinphoneMagicSearchCbs *linphone_magic_search_cbs_new (void);

/**
 * Acquire a reference to the chat message callbacks object.
 * @param cbs The #LinphoneMagicSearchCbs object @notnil
 * @return The same chat message callbacks object
**/
LINPHONE_PUBLIC LinphoneMagicSearchCbs * linphone_magic_search_cbs_ref (LinphoneMagicSearchCbs *cbs);

/**
 * Release reference to the chat message callbacks object.
 * @param cbs The #LinphoneMagicSearchCbs object @notnil
**/
LINPHONE_PUBLIC void linphone_magic_search_cbs_unref (LinphoneMagicSearchCbs *cbs);

/**
 * Retrieve the user pointer associated with the chat message callbacks object.
 * @param cbs The #LinphoneMagicSearchCbs object @notnil
 * @return The user pointer associated with the chat message callbacks object. @maybenil
**/
LINPHONE_PUBLIC void * linphone_magic_search_cbs_get_user_data (const LinphoneMagicSearchCbs *cbs);

/**
 * Assign a user pointer to the chat message callbacks object.
 * @param cbs The #LinphoneMagicSearchCbs object @notnil
 * @param user_data The user pointer to associate with the chat message callbacks object. @maybenil
**/
LINPHONE_PUBLIC void linphone_magic_search_cbs_set_user_data (LinphoneMagicSearchCbs *cbs, void *user_data);

/**
 * Get the message state changed callback.
 * @param cbs #LinphoneMagicSearchCbs object. @notnil
 * @return The current message state changed callback.
 */
LINPHONE_PUBLIC LinphoneMagicSearchCbsSearchResultsReceivedCb linphone_magic_search_cbs_get_search_results_received (const LinphoneMagicSearchCbs *cbs);

/**
 * Set the message state changed callback.
 * @param cbs #LinphoneMagicSearchCbs object. @notnil
 * @param cb The message state changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_magic_search_cbs_set_search_results_received (LinphoneMagicSearchCbs *cbs, LinphoneMagicSearchCbsSearchResultsReceivedCb cb);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_CHAT_MESSAGE_CBS_H_
