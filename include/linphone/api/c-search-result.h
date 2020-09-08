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

#ifndef _L_C_SEARCH_RESULT_H_
#define _L_C_SEARCH_RESULT_H_

#include "linphone/api/c-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup misc
 * @{
 */

/**
 * Increment reference count of LinphoneSearchResult object.
 * @param search_result the #LinphoneSearchResult object @notnil
 * @return the same #LinphoneSearchResult object @notnil
 **/
LINPHONE_PUBLIC LinphoneSearchResult *linphone_search_result_ref(LinphoneSearchResult *search_result);

/**
 * Decrement reference count of LinphoneSearchResult object. When dropped to zero, memory is freed.
 * @param search_result the #LinphoneSearchResult object @notnil
 **/
LINPHONE_PUBLIC void linphone_search_result_unref(LinphoneSearchResult *search_result);

/**
 * Gets the friend of the search result if any.
 * @param search_result the #LinphoneSearchResult object @notnil
 * @return The associated #LinphoneFriend or NULL. @maybenil
 **/
LINPHONE_PUBLIC const LinphoneFriend* linphone_search_result_get_friend(const LinphoneSearchResult *search_result);

/**
 * Gets the address of the search result if any.
 * @param search_result the #LinphoneSearchResult object @notnil
 * @return The associed #LinphoneAddress or NULL. @maybenil
 **/
LINPHONE_PUBLIC const LinphoneAddress* linphone_search_result_get_address(const LinphoneSearchResult *search_result);

/**
 * Gets the phone number of the search result if any.
 * @param search_result the #LinphoneSearchResult object @notnil
 * @return The associed phone number or NULL. @maybenil
 **/
LINPHONE_PUBLIC const char* linphone_search_result_get_phone_number(const LinphoneSearchResult *search_result);

/**
 * Returns the capabilities mask of the search result.
 * @param search_result the #LinphoneSearchResult object @notnil
 * @return the capabilities mask associated to the search result
 **/
LINPHONE_PUBLIC int linphone_search_result_get_capabilities(const LinphoneSearchResult *search_result);

/**
 * Returns whether or not the search result has the given capability
 * @param search_result the #LinphoneSearchResult object @notnil
 * @param capability the #LinphoneFriendCapability to check
 * @return TRUE if it has the capability, FALSE otherwise.
 **/
LINPHONE_PUBLIC bool_t linphone_search_result_has_capability(const LinphoneSearchResult *search_result, const LinphoneFriendCapability capability);

/**
 * Gets the weight of the search result.
 * @param search_result the #LinphoneSearchResult object @notnil
 * @return the result weight
 **/
LINPHONE_PUBLIC unsigned int linphone_search_result_get_weight(const LinphoneSearchResult *search_result);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif // _L_C_SEARCH_RESULT_H_
