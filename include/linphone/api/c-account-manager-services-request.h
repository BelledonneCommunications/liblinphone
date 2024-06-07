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

#ifndef LINPHONE_ACCOUNT_MANAGER_SERVICES_REQUEST_H
#define LINPHONE_ACCOUNT_MANAGER_SERVICES_REQUEST_H

#include "linphone/api/c-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup account_creator
 * @{
 */

/**
 * Takes a reference on a #LinphoneAccountManagerServicesRequest.
 * @param request The #LinphoneAccountManagerServicesRequest object. @notnil
 * @return the same #LinphoneAccountManagerServicesRequest object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccountManagerServicesRequest *
linphone_account_manager_services_request_ref(LinphoneAccountManagerServicesRequest *request);

/**
 * Releases a #LinphoneAccountManagerServicesRequest.
 * @param request The #LinphoneAccountManagerServicesRequest object. @notnil
 */
LINPHONE_PUBLIC void linphone_account_manager_services_request_unref(LinphoneAccountManagerServicesRequest *request);

// -----------------------------------------------------------------------------

/**
 * Executes the request represented by #LinphoneAccountManagerServicesRequest.
 * @param request The #LinphoneAccountManagerServicesRequest object to execute. @notnil
 */
LINPHONE_PUBLIC void linphone_account_manager_services_request_submit(LinphoneAccountManagerServicesRequest *request);

/**
 * Gets the #LinphoneAccountManagerServicesRequestType representing this request.
 * @param request The #LinphoneAccountManagerServicesRequest object. @notnil
 * @return the #LinphoneAccountManagerServicesRequestType.
 */
LINPHONE_PUBLIC LinphoneAccountManagerServicesRequestType
linphone_account_manager_services_request_get_type(const LinphoneAccountManagerServicesRequest *request);

// -----------------------------------------------------------------------------

/**
 * Adds a listener in order to be notified of #LinphoneAccountManagerServicesRequest events.
 * @param request The #LinphoneAccountManagerServicesRequest object to monitor. @notnil
 * @param cbs A #LinphoneAccountManagerServicesRequestCbs object holding the callbacks you need. @notnil
 */
LINPHONE_PUBLIC void
linphone_account_manager_services_request_add_callbacks(LinphoneAccountManagerServicesRequest *request,
                                                        LinphoneAccountManagerServicesRequestCbs *cbs);

/**
 * Removes a listener from a #LinphoneAccountManagerServicesRequest.
 * @param request The #LinphoneAccount object. @notnil
 * @param cbs #LinphoneAccountManagerServicesRequestCbs object to remove. @notnil
 */
LINPHONE_PUBLIC void
linphone_account_manager_services_request_remove_callbacks(LinphoneAccountManagerServicesRequest *request,
                                                           LinphoneAccountManagerServicesRequestCbs *cbs);

/**
 * Gets the current LinphoneAccountManagerServicesRequest.
 * This is meant only to be called from a callback to be able to get the user_data associated with the
 * #LinphoneAccountManagerServicesRequestCbs that is calling the callback.
 * @param request The #LinphoneAccountManagerServicesRequest object. @notnil
 * @return The #LinphoneAccountManagerServicesRequestCbs that has called the last callback. @maybenil
 */
LINPHONE_PUBLIC LinphoneAccountManagerServicesRequestCbs *
linphone_account_manager_services_request_get_current_callbacks(const LinphoneAccountManagerServicesRequest *request);

// -----------------------------------------------------------------------------

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_ACCOUNT_MANAGER_SERVICES_REQUEST_H */
