/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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

#ifndef _L_C_ACCOUNT_CBS_H_
#define _L_C_ACCOUNT_CBS_H_

#include "linphone/api/c-callbacks.h"
#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
	extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup account
 * @{
 */

/**
 * Create a new account callbacks object.
 * @return The #LinphoneAccountCbs object. @notnil
**/
LinphoneAccountCbs *linphone_account_cbs_new (void);

/**
 * Acquire a reference to the account callbacks object.
 * @param cbs The #LinphoneAccountCbs object. @notnil
 * @return The same account callbacks object. @notnil
**/
LINPHONE_PUBLIC LinphoneAccountCbs * linphone_account_cbs_ref (LinphoneAccountCbs *cbs);

/**
 * Release reference to the account callbacks object.
 * @param cbs The #LinphoneAccountCbs object. @notnil
**/
LINPHONE_PUBLIC void linphone_account_cbs_unref (LinphoneAccountCbs *cbs);

/**
 * Retrieve the user pointer associated with the account callbacks object.
 * @param cbs The #LinphoneAccountCbs object. @notnil
 * @return The user pointer associated with the account callbacks object. @maybenil
**/
LINPHONE_PUBLIC void * linphone_account_cbs_get_user_data (const LinphoneAccountCbs *cbs);

/**
 * Assign a user pointer to the account callbacks object.
 * @param cbs The #LinphoneAccountCbs object. @notnil
 * @param user_data The user pointer to associate with the account callbacks object. @maybenil
**/
LINPHONE_PUBLIC void linphone_account_cbs_set_user_data (LinphoneAccountCbs *cbs, void *user_data);

/**
 * Get the registration state changed callback.
 * @param cbs #LinphoneAccountCbs object. @notnil
 * @return The current registration state changed callback.
 */
LINPHONE_PUBLIC LinphoneAccountCbsRegistrationStateChangedCb linphone_account_cbs_get_registration_state_changed (const LinphoneAccountCbs *cbs);

/**
 * Set the registration state changed callback.
 * @param cbs #LinphoneAccountCbs object. @notnil
 * @param cb The registration state changed callback to be used.
 */
LINPHONE_PUBLIC void linphone_account_cbs_set_registration_state_changed (LinphoneAccountCbs *cbs, LinphoneAccountCbsRegistrationStateChangedCb cb);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_ACCOUNT_CBS_H_
