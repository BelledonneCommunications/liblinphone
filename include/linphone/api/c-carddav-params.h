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

#ifndef LINPHONE_CARDDAV_PARAMS_H
#define LINPHONE_CARDDAV_PARAMS_H

#include "linphone/api/c-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup contacts
 * @{
 */

/**
 * Instantiates a new #LinphoneCardDavParams with values from source.
 * @param params The #LinphoneCardDavParams object to be cloned. @notnil
 * @return The newly created #LinphoneCardDavParams object. @notnil
 */
LINPHONE_PUBLIC LinphoneCardDavParams *linphone_card_dav_params_clone(const LinphoneCardDavParams *params);

/**
 * Takes a reference on a #LinphoneCardDavParams.
 * @param params The #LinphoneCardDavParams object. @notnil
 * @return the same #LinphoneCardDavParams object. @notnil
 */
LINPHONE_PUBLIC LinphoneCardDavParams *linphone_card_dav_params_ref(LinphoneCardDavParams *params);

/**
 * Releases a #LinphoneCardDavParams.
 * @param params The #LinphoneCardDavParams object. @notnil
 */
LINPHONE_PUBLIC void linphone_card_dav_params_unref(LinphoneCardDavParams *params);

/*****************************************************************************************************************/

/**
 * Gets the list of vCard RFC fields to use to match user input text on.
 * For example you can use "FN", "N", "IMPP", "ORG", etc...
 * @param The #LinphoneCardDavParams object. @notnil
 * @return The list of vCard fields to make the query on using user input. \bctbx_list{char *} @maybenil @tobefreed
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_card_dav_params_get_user_input_fields(const LinphoneCardDavParams *params);

/**
 * Sets the list of vCard RFC fields to use to match user input text on.
 * For example you can use "FN", "N", "IMPP", "ORG", etc...
 * @param The #LinphoneCardDavParams object. @notnil
 * @param list the list of vCard RFC fields to use to match user input text on. \bctbx_list{char *} @maybenil
 */
LINPHONE_PUBLIC void linphone_card_dav_params_set_user_input_fields(LinphoneCardDavParams *params, bctbx_list_t *list);

/**
 * Gets the list of vCard RFC fields to use to match the domain filter on.
 * For example you can use "IMPP".
 * @param The #LinphoneCardDavParams object. @notnil
 * @return The list of vCard fields to make the query on using domain filter. \bctbx_list{char *} @maybenil @tobefreed
 */
LINPHONE_PUBLIC bctbx_list_t *linphone_card_dav_params_get_domain_fields(const LinphoneCardDavParams *params);

/**
 * Sets the list of vCard RFC fields to use to match the domain filter on.
 * For example you can use "IMPP".
 * @param The #LinphoneCardDavParams object. @notnil
 * @param list the list of vCard RFC fields to use to match the domain filter on. \bctbx_list{const char *}  @maybenil
 */
LINPHONE_PUBLIC void linphone_card_dav_params_set_domain_fields(LinphoneCardDavParams *params,
                                                                const bctbx_list_t *list);

/**
 * Gets the matching policy: TRUE if the remote vCard value must match the filter exactly, FALSE if it only needs to
 * contain the filter.
 * @param The #LinphoneCardDavParams object. @notnil
 * @return The matching policy: TRUE if the remote vCard value must match the filter exactly, FALSE if it only needs to
 * contain the filter.
 */
LINPHONE_PUBLIC bool_t linphone_card_dav_params_get_use_exact_match_policy(const LinphoneCardDavParams *params);

/**
 * Sets the matching policy: TRUE if the remote vCard value must match the filter exactly, FALSE if it only needs to
 * contain the filter.
 * @param The #LinphoneCardDavParams object. @notnil
 * @param exact_match the policy to use: TRUE if the remote vCard value must match the filter exactly, FALSE if it only
 * needs to contain the filter.
 */
LINPHONE_PUBLIC void linphone_card_dav_params_set_use_exact_match_policy(LinphoneCardDavParams *params,
                                                                         bool_t exact_match);

/*****************************************************************************************************************/

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_CARDDAV_PARAMS_H */
