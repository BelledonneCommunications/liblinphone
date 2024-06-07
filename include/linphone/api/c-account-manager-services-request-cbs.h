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

#ifndef _L_C_ACCOUNT_MANAGER_SERVICES_REQUEST_CBS_H_
#define _L_C_ACCOUNT_MANAGER_SERVICES_REQUEST_CBS_H_

#include "linphone/api/c-callbacks.h"
#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup account_creator
 * @{
 */

/**
 * Acquire a reference to the account manager services callbacks object.
 * @param cbs The #LinphoneAccountManagerServicesRequestCbs object. @notnil
 * @return The same account manager services callbacks object. @notnil
 **/
LINPHONE_PUBLIC LinphoneAccountManagerServicesRequestCbs *
linphone_account_manager_services_request_cbs_ref(LinphoneAccountManagerServicesRequestCbs *cbs);

/**
 * Release reference to the account manager services callbacks object.
 * @param cbs The #LinphoneAccountManagerServicesRequestCbs object. @notnil
 **/
LINPHONE_PUBLIC void linphone_account_manager_services_request_cbs_unref(LinphoneAccountManagerServicesRequestCbs *cbs);

/**
 * Retrieve the user pointer associated with the account manager services callbacks object.
 * @param cbs The #LinphoneAccountManagerServicesRequestCbs object. @notnil
 * @return The user pointer associated with the account manager services callbacks object. @maybenil
 **/
LINPHONE_PUBLIC void *
linphone_account_manager_services_request_cbs_get_user_data(const LinphoneAccountManagerServicesRequestCbs *cbs);

/**
 * Assign a user pointer to the account manager services callbacks object.
 * @param cbs The #LinphoneAccountManagerServicesRequestCbs object. @notnil
 * @param user_data The user pointer to associate with the account manager services callbacks object. @maybenil
 **/
LINPHONE_PUBLIC void
linphone_account_manager_services_request_cbs_set_user_data(LinphoneAccountManagerServicesRequestCbs *cbs,
                                                            void *user_data);

// -----------------------------------------------------------------------------

/**
 * Gets the request successful callback.
 * @param cbs #LinphoneAccountManagerServicesRequestCbs object. @notnil
 * @return The current request successful callback.
 */
LINPHONE_PUBLIC LinphoneAccountManagerServicesRequestCbsOnSuccessfulRequestCb
linphone_account_manager_services_request_cbs_get_request_successful(
    const LinphoneAccountManagerServicesRequestCbs *cbs);

/**
 * Sets the request successful callback.
 * @param cbs #LinphoneAccountManagerServicesRequestCbs object. @notnil
 * @param cb The request successful callback to be used.
 */
LINPHONE_PUBLIC void linphone_account_manager_services_request_cbs_set_request_successful(
    LinphoneAccountManagerServicesRequestCbs *cbs, LinphoneAccountManagerServicesRequestCbsOnSuccessfulRequestCb cb);

/**
 * Gets the request error callback.
 * @param cbs #LinphoneAccountManagerServicesRequestCbs object. @notnil
 * @return The current request error callback.
 */
LINPHONE_PUBLIC LinphoneAccountManagerServicesRequestCbsOnRequestErrorCb
linphone_account_manager_services_request_cbs_get_request_error(const LinphoneAccountManagerServicesRequestCbs *cbs);

/**
 * Sets the request error callback.
 * @param cbs #LinphoneAccountManagerServicesRequestCbs object. @notnil
 * @param cb The message request error callback to be used.
 */
LINPHONE_PUBLIC void linphone_account_manager_services_request_cbs_set_request_error(
    LinphoneAccountManagerServicesRequestCbs *cbs, LinphoneAccountManagerServicesRequestCbsOnRequestErrorCb cb);

/**
 * Gets the devices list fetched callback.
 * @param cbs #LinphoneAccountManagerServicesRequestCbs object. @notnil
 * @return The current devices list fetched callback.
 */
LINPHONE_PUBLIC LinphoneAccountManagerServicesRequestCbsOnDevicesListFetchedCb
linphone_account_manager_services_request_cbs_get_devices_list_fetched(
    const LinphoneAccountManagerServicesRequestCbs *cbs);

/**
 * Sets the devices list fetched callback.
 * @param cbs #LinphoneAccountManagerServicesRequestCbs object. @notnil
 * @param cb The message devices list fetched callback to be used.
 */
LINPHONE_PUBLIC void linphone_account_manager_services_request_cbs_set_devices_list_fetched(
    LinphoneAccountManagerServicesRequestCbs *cbs, LinphoneAccountManagerServicesRequestCbsOnDevicesListFetchedCb cb);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_ACCOUNT_MANAGER_SERVICES_REQUEST_CBS_H_
