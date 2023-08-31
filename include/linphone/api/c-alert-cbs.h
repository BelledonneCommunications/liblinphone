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

#ifndef _L_C_ALERT_CBS_H_
#define _L_C_ALERT_CBS_H_

#include "linphone/api/c-callbacks.h"
#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup alert
 * @{
 */

/**
 * Acquire a reference to the alert callbacks object.
 * @param cbs The #LinphoneAlertCbs object. @notnil
 * @return The same alert callbacks object. @notnil
 **/
LINPHONE_PUBLIC LinphoneAlertCbs *linphone_alert_cbs_ref(LinphoneAlertCbs *cbs);

/**
 * Release reference to the alert callbacks object.
 * @param cbs The #LinphoneAlertCbs object. @notnil
 **/
LINPHONE_PUBLIC void linphone_alert_cbs_unref(LinphoneAlertCbs *cbs);

/**
 * Retrieve the user pointer associated with the alert callbacks object.
 * @param cbs The #LinphoneAlertCbs object. @notnil
 * @return The user pointer associated with the alert callbacks object. @maybenil
 **/
LINPHONE_PUBLIC void *linphone_alert_cbs_get_user_data(const LinphoneAlertCbs *cbs);

/**
 * Assign a user pointer to the alert callbacks object.
 * @param cbs The #LinphoneAlertCbs object. @notnil
 * @param user_data The user pointer to associate with the alert callbacks object. @maybenil
 **/
LINPHONE_PUBLIC void linphone_alert_cbs_set_user_data(LinphoneAlertCbs *cbs, void *user_data);
/**
 * Assign a callback for when the alert is terminated
 * @param cbs The #LinphoneAlertCbs object. @notnil
 * @param on_terminated The #LinphoneAlertCbsOnTerminatedCb callback to execute. @notnil
 */
LINPHONE_PUBLIC void linphone_alert_cbs_set_terminated(LinphoneAlertCbs *cbs, LinphoneAlertCbsTerminatedCb terminated);
/**
 * Get the callback for when the alert is terminated
 * @param cbs The #LinphoneAlertCbs object. @notnil
 * @return The #LinphoneAlertCbsOnTerminatedCb callback to execute. @maybenil
 */
LINPHONE_PUBLIC LinphoneAlertCbsTerminatedCb linphone_alert_cbs_get_terminated(LinphoneAlertCbs *cbs);

/**
 * @}
 */
#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_ALERT_CBS_H_
