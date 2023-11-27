/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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
#ifndef _L_C_ALERT_H_
#define _L_C_ALERT_H_

#include "linphone/api/c-types.h"
#include "linphone/enums/c-enums.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @addtogroup alert
 * @{
 */

/**
 * Provide a string describing the alert type.
 * @param type the #LinphoneAlertType
 * @return a string
 */
LINPHONE_PUBLIC const char *linphone_alert_type_to_string(LinphoneAlertType type);

/**
 * Clone the given alert
 * @param alert The given alert. @notnil
 * @return A new alert with exactly same informations that param. @notnil
 */
LINPHONE_PUBLIC LinphoneAlert *linphone_alert_clone(const LinphoneAlert *alert);
/**
 * Take a reference on a #LinphoneAlert.
 * @param alert The #LinphoneAlert object. @notnil
 * @return the same #LinphoneAlert object. @notnil
 */
LINPHONE_PUBLIC LinphoneAlert *linphone_alert_ref(LinphoneAlert *alert);

/**
 * Release a #LinphoneAlert.
 * @param alert The #LinphoneAlert object. @notnil
 */
LINPHONE_PUBLIC void linphone_alert_unref(LinphoneAlert *alert);
/**
 * Return the start time of the alert
 * @param alert The given alert. @notnil
 * @return the start time of the alert.
 */
LINPHONE_PUBLIC time_t linphone_alert_get_start_time(const LinphoneAlert *alert);
/**
 * Return the end time of the alert
 * @param alert The given alert. @notnil
 * @return the end time of the alert.
 */
LINPHONE_PUBLIC time_t linphone_alert_get_end_time(const LinphoneAlert *alert);
/**
 * Return the type of the alert
 * @param alert The given alert. @notnil
 * @return A #LinphoneAlertType corresponding to the current alert.
 */
LINPHONE_PUBLIC LinphoneAlertType linphone_alert_get_type(const LinphoneAlert *alert);
/**
 * Return more informations about the alerts
 * @param alert The given alert. @notnil
 * @return A #LinphoneDictionary containing informations about the current alert. @maybenil
 */
LINPHONE_PUBLIC const LinphoneDictionary *linphone_alert_get_informations(const LinphoneAlert *alert);
/**
 * Return the call from the alert.
 * @param alert The given alert. @notnil
 * @return A #LinphoneCall from the alert. @notnil
 */
LINPHONE_PUBLIC LinphoneCall *linphone_alert_get_call(const LinphoneAlert *alert);
/**
 * Return the state of the alert.
 * @param alert The given alert. @notnil
 * @return true if and only if the alert is active.
 */
LINPHONE_PUBLIC bool_t linphone_alert_get_state(const LinphoneAlert *alert);
/**
 * Add a listener in order to be notified of #LinphoneAlert events.
 * @param alert The #LinphoneAlert object to monitor. @notnil
 * @param cbs A #LinphoneAlertCbs object holding the callbacks you need. @notnil
 */
LINPHONE_PUBLIC void linphone_alert_add_callbacks(LinphoneAlert *alert, LinphoneAlertCbs *cbs);
/**
 * Gets the current LinphoneAlertCbs.
 * This is meant only to be called from a callback to be able to get the user_data associated with the
 * #LinphoneAlertCbs that is calling the callback.
 * @param alert The #LinphoneAlert object. @notnil
 * @return The #LinphoneAlertCbs that has called the last callback. @maybenil
 */
LINPHONE_PUBLIC LinphoneAlertCbs *linphone_alert_get_current_callbacks(const LinphoneAlert *alert);
/**
 * @brief Gets the list of listener in the alert.
 * @param alert #LinphoneAlert object. @notnil
 * @return The list of #LinphoneAlertCbs. @maybenil
 * @donotwrap
 */
LINPHONE_PUBLIC const bctbx_list_t *linphone_alert_get_callbacks_list(const LinphoneAlert *alert);
/**
 * Removes a #LinphoneAlertCbs object associated to the LinphoneAlert.
 * @param alert #LinphoneAlert object @notnil
 * @param cbs The #LinphoneAlertCbs object to be removed from the LinphoneAlert. @notnil
 * @donotwrap
 */
LINPHONE_PUBLIC void linphone_alert_remove_callbacks(LinphoneAlert *alert, LinphoneAlertCbs *cbs);
/**
 * Notify the alert if it is terminated.
 * @param alert The given alert. @notnil
 */
LINPHONE_PUBLIC void linphone_alert_notify_on_terminated(LinphoneAlert *alert);
/**
 * @}
 */
#ifdef __cplusplus
}
#endif

#endif //_L_C_ALERT_H_
