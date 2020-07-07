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

#ifndef LINPHONE_ACCOUNT_CREATOR_SERVICE_H_
#define LINPHONE_ACCOUNT_CREATOR_SERVICE_H_

#include "linphone/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Function to set custom server request.
 * @param creator #LinphoneAccountCreator object
 */
typedef LinphoneAccountCreatorStatus (*LinphoneAccountCreatorRequestFunc)(LinphoneAccountCreator *creator);

/**
 * @addtogroup account_creator_request
 * @{
 */

/************************** Start Account Creator Requests **************************/

/**
 * Create a new #LinphoneAccountCreatorService object.
 * @return a new #LinphoneAccountCreatorService object.
 * @donotwrap
**/
LINPHONE_PUBLIC LinphoneAccountCreatorService * linphone_account_creator_service_new(void);

/**
 * Acquire a reference to a #LinphoneAccountCreatorService object.
 * @param service #LinphoneAccountCreatorService object.
 * @return The same #LinphoneAccountCreatorService object.
 * @donotwrap
**/
LinphoneAccountCreatorService * linphone_account_creator_service_ref(LinphoneAccountCreatorService *service);

/**
 * Release a reference to a #LinphoneAccountCreatorService object.
 * @param service #LinphoneAccountCreatorService object.
 * @donotwrap
**/
void linphone_account_creator_service_unref(LinphoneAccountCreatorService *service);

/**
 * Retrieve the user pointer associated with a #LinphoneAccountCreatorService object.
 * @param service #LinphoneAccountCreatorService object.
 * @return The user pointer associated with the #LinphoneAccountCreatorService object.
 * @donotwrap
**/
LINPHONE_PUBLIC void *linphone_account_creator_service_get_user_data(const LinphoneAccountCreatorService *service);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorService object.
 * @param service #LinphoneAccountCreatorService object.
 * @param ud The user pointer to associate with the #LinphoneAccountCreatorService object.
 * @donotwrap
**/
LINPHONE_PUBLIC void linphone_account_creator_service_set_user_data(LinphoneAccountCreatorService *service, void *ud);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorService object.
 * @param service #LinphoneAccountCreatorService object.
 * @param cb The constructor of account creator requests.
 * @donotwrap
**/
LINPHONE_PUBLIC void linphone_account_creator_service_set_constructor_cb(LinphoneAccountCreatorService *service, LinphoneAccountCreatorRequestFunc cb);

/**
 * Get the constructor of account creator requests.
 * @param service #LinphoneAccountCreatorService object.
 * @return The current constructor of create account request.
 * @donotwrap
**/
LINPHONE_PUBLIC LinphoneAccountCreatorRequestFunc linphone_account_creator_service_get_constructor_cb(const LinphoneAccountCreatorService *service);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorService object.
 * @param service #LinphoneAccountCreatorService object.
 * @param cb The destructor.
 * @donotwrap
**/
LINPHONE_PUBLIC void linphone_account_creator_service_set_destructor_cb(LinphoneAccountCreatorService *service, LinphoneAccountCreatorRequestFunc cb);

/**
 * Get the destructor of create account request.
 * @param service #LinphoneAccountCreatorService object.
 * @return The current destructor of create account request.
 * @donotwrap
**/
LINPHONE_PUBLIC LinphoneAccountCreatorRequestFunc linphone_account_creator_service_get_destructor_cb(const LinphoneAccountCreatorService *service);

/**
 * Get the create account request.
 * @param service #LinphoneAccountCreatorService object.
 * @return The current create account request.
 * @donotwrap
**/
LINPHONE_PUBLIC LinphoneAccountCreatorRequestFunc linphone_account_creator_service_get_create_account_cb(const LinphoneAccountCreatorService *service);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorService object.
 * @param service #LinphoneAccountCreatorService object.
 * @param cb The create account request to be used.
 * @donotwrap
**/
LINPHONE_PUBLIC void linphone_account_creator_service_set_create_account_cb(LinphoneAccountCreatorService *service, LinphoneAccountCreatorRequestFunc cb);

/**
 * Get the is account exist request.
 * @param service #LinphoneAccountCreatorService object.
 * @return The current is account exist request.
 * @donotwrap
**/
LINPHONE_PUBLIC LinphoneAccountCreatorRequestFunc linphone_account_creator_service_get_is_account_exist_cb(const LinphoneAccountCreatorService *service);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorService object.
 * @param service #LinphoneAccountCreatorService object.
 * @param cb The is account exist request to be used.
 * @donotwrap
**/
LINPHONE_PUBLIC void linphone_account_creator_service_set_is_account_exist_cb(LinphoneAccountCreatorService *service, LinphoneAccountCreatorRequestFunc cb);

/**
 * Get the activate account request.
 * @param service #LinphoneAccountCreatorService object.
 * @return The current activate account request.
 * @donotwrap
**/
LINPHONE_PUBLIC LinphoneAccountCreatorRequestFunc linphone_account_creator_service_get_activate_account_cb(const LinphoneAccountCreatorService *service);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorService object.
 * @param service #LinphoneAccountCreatorService object.
 * @param cb The activate account request to be used.
 * @donotwrap
**/
LINPHONE_PUBLIC void linphone_account_creator_service_set_activate_account_cb(LinphoneAccountCreatorService *service, LinphoneAccountCreatorRequestFunc cb);

/**
 * Get the is account activated request.
 * @param service #LinphoneAccountCreatorService object.
 * @return The current is account activated request.
 * @donotwrap
**/
LINPHONE_PUBLIC LinphoneAccountCreatorRequestFunc linphone_account_creator_service_get_is_account_activated_cb(const LinphoneAccountCreatorService *service);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorService object.
 * @param service #LinphoneAccountCreatorService object.
 * @param cb The is account activated request to be used.
 * @donotwrap
**/
LINPHONE_PUBLIC void linphone_account_creator_service_set_is_account_activated_cb(LinphoneAccountCreatorService *service, LinphoneAccountCreatorRequestFunc cb);

/**
 * Get the link account request.
 * @param service #LinphoneAccountCreatorService object.
 * @return The current link account request.
 * @donotwrap
**/
LINPHONE_PUBLIC LinphoneAccountCreatorRequestFunc linphone_account_creator_service_get_link_account_cb(const LinphoneAccountCreatorService *service);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorService object.
 * @param service #LinphoneAccountCreatorService object.
 * @param cb The link account request to be used.
 * @donotwrap
**/
LINPHONE_PUBLIC void linphone_account_creator_service_set_link_account_cb(LinphoneAccountCreatorService *service, LinphoneAccountCreatorRequestFunc cb);

/**
 * Get the activate alias request.
 * @param service #LinphoneAccountCreatorService object.
 * @return The current link account request.
 * @donotwrap
**/
LINPHONE_PUBLIC LinphoneAccountCreatorRequestFunc linphone_account_creator_service_get_activate_alias_cb(const LinphoneAccountCreatorService *service);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorService object.
 * @param service #LinphoneAccountCreatorService object.
 * @param cb The activate alias request to be used.
 * @donotwrap
**/
LINPHONE_PUBLIC void linphone_account_creator_service_set_activate_alias_cb(LinphoneAccountCreatorService *service, LinphoneAccountCreatorRequestFunc cb);

/**
 * Get the is alias used request.
 * @param service #LinphoneAccountCreatorService object.
 * @return The current is alias used request.
 * @donotwrap
**/
LINPHONE_PUBLIC LinphoneAccountCreatorRequestFunc linphone_account_creator_service_get_is_alias_used_cb(const LinphoneAccountCreatorService *service);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorService object.
 * @param service #LinphoneAccountCreatorService object.
 * @param cb The is alias used request to be used.
 * @donotwrap
**/
LINPHONE_PUBLIC void linphone_account_creator_service_set_is_alias_used_cb(LinphoneAccountCreatorService *service, LinphoneAccountCreatorRequestFunc cb);

/**
 * Get the is account linked request.
 * @param service #LinphoneAccountCreatorService object.
 * @return The current is account linked request.
 * @donotwrap
**/
LINPHONE_PUBLIC LinphoneAccountCreatorRequestFunc linphone_account_creator_service_get_is_account_linked_cb(const LinphoneAccountCreatorService *service);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorService object.
 * @param service #LinphoneAccountCreatorService object.
 * @param cb The is account linked request to be used.
 * @donotwrap
**/
LINPHONE_PUBLIC void linphone_account_creator_service_set_is_account_linked_cb(LinphoneAccountCreatorService *service, LinphoneAccountCreatorRequestFunc cb);

/**
 * Get the recover account request.
 * @param service #LinphoneAccountCreatorService object.
 * @return The current recover account request.
 * @donotwrap
**/
LINPHONE_PUBLIC LinphoneAccountCreatorRequestFunc linphone_account_creator_service_get_recover_account_cb(const LinphoneAccountCreatorService *service);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorService object.
 * @param service #LinphoneAccountCreatorService object.
 * @param cb The recover account request to be used.
 * @donotwrap
**/
LINPHONE_PUBLIC void linphone_account_creator_service_set_recover_account_cb(LinphoneAccountCreatorService *service, LinphoneAccountCreatorRequestFunc cb);

/**
 * Get the update account request.
 * @param service #LinphoneAccountCreatorService object.
 * @return The current update account request.
 * @donotwrap
**/
LINPHONE_PUBLIC LinphoneAccountCreatorRequestFunc linphone_account_creator_service_get_update_account_cb(const LinphoneAccountCreatorService *service);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorService object.
 * @param service #LinphoneAccountCreatorService object.
 * @param cb The update account request to be used.
 * @donotwrap
**/
LINPHONE_PUBLIC void linphone_account_creator_service_set_update_account_cb(LinphoneAccountCreatorService *service, LinphoneAccountCreatorRequestFunc cb);

/**
 * Get the login linphone account request.
 * @param service #LinphoneAccountCreatorService object.
 * @return The current login linphone account request.
 * @donotwrap
**/
LINPHONE_PUBLIC LinphoneAccountCreatorRequestFunc linphone_account_creator_service_get_login_linphone_account_cb(const LinphoneAccountCreatorService *service);

/**
 * Assign a user pointer to a #LinphoneAccountCreatorService object.
 * @param service #LinphoneAccountCreatorService object.
 * @param cb The login linphone account request to be used.
 * @donotwrap
**/
LINPHONE_PUBLIC void linphone_account_creator_service_set_login_linphone_account_cb(LinphoneAccountCreatorService *service, LinphoneAccountCreatorRequestFunc cb);

/************************** End Account Creator Requests **************************/

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* LINPHONE_ACCOUNT_CREATOR_SERVICE_H_ */
