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

#ifndef _L_C_EVENT_CBS_H_
#define _L_C_EVENT_CBS_H_

#include "linphone/api/c-callbacks.h"
#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup call_control
 * @{
 */

LinphoneEventCbs *linphone_event_cbs_new(void);

/**
 * Acquire a reference to a LinphoneEventCbs object.
 * @return The same LinphoneEventCbs object. @notnil
 **/
LINPHONE_PUBLIC LinphoneEventCbs *linphone_event_cbs_ref(LinphoneEventCbs *cbs);

/**
 * Release a reference to a LinphoneEventCbs object.
 * @param cbs LinphoneEventCbs object. @notnil
 **/
LINPHONE_PUBLIC void linphone_event_cbs_unref(LinphoneEventCbs *cbs);

/**
 * Retrieve the user pointer associated with a LinphoneEventCbs object.
 * @param cbs LinphoneEventCbs object. @notnil
 * @return The user pointer associated with the LinphoneEventCbs object. @maybenil
 **/
LINPHONE_PUBLIC void *linphone_event_cbs_get_user_data(const LinphoneEventCbs *cbs);

/**
 * Assign a user pointer to a LinphoneEventCbs object.
 * @param cbs LinphoneEventCbs object. @notnil
 * @param user_data The user pointer to associate with the LinphoneEventCbs object. @maybenil
 **/
LINPHONE_PUBLIC void linphone_event_cbs_set_user_data(LinphoneEventCbs *cbs, void *user_data);

/**
 * Get the notify response callback.
 * @param cbs LinphoneEventCbs object. @notnil
 * @return The current notify response callback.
 **/
LINPHONE_PUBLIC LinphoneEventCbsNotifyResponseCb linphone_event_cbs_get_notify_response(const LinphoneEventCbs *cbs);

/**
 * Set the notify response callback.
 * @param cbs LinphoneEventCbs object. @notnil
 * @param cb The notify response callback to be used.
 **/
LINPHONE_PUBLIC void linphone_event_cbs_set_notify_response(LinphoneEventCbs *cbs, LinphoneEventCbsNotifyResponseCb cb);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_EVENT_CBS_H_