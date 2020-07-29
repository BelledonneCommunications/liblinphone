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
#ifndef LINPHONE_PUSH_NOTIFICATION_CONFIG_H
#define LINPHONE_PUSH_NOTIFICATION_CONFIG_H

#include "linphone/api/c-types.h"

/**
 * @addtogroup misc
 * @{
 */

/**
 * Safely cast a belle_sip_object_t into #LinphonePushNotificationMessage
 */

#ifdef __cplusplus
extern "C" {
#endif

LINPHONE_PUBLIC LinphonePushNotificationConfig *linphone_push_notification_config_new(void);

/**
 * Instantiate a new push notification parameters with values from source.
 * @param push_cfg The #LinphonePushNotificationConfig object to be cloned. @notnil
 * @return The newly created #LinphonePushNotificationConfig object. @notnil
 */
LINPHONE_PUBLIC LinphonePushNotificationConfig* linphone_push_notification_config_clone(const LinphonePushNotificationConfig *push_cfg);

/**
* Take a reference on a #LinphonePushNotificationConfig.
* @param push_cfg the #LinphonePushNotificationConfig object @notnil
* @return the same #LinphonePushNotificationConfig object @notnil
*/
LINPHONE_PUBLIC LinphonePushNotificationConfig *linphone_push_notification_config_ref(LinphonePushNotificationConfig *push_cfg);

/**
* Release a #LinphonePushNotificationConfig.
* @param push_cfg the #LinphonePushNotificationConfig object @notnil
*/
LINPHONE_PUBLIC void linphone_push_notification_config_unref(LinphonePushNotificationConfig *push_cfg);

/**
* Gets the provider for "contact uri parameter".
* @param push_cfg The #LinphonePushNotificationConfig object @notnil
* @return The provider, default value "". @notnil
*/
LINPHONE_PUBLIC const char *linphone_push_notification_config_get_provider (const LinphonePushNotificationConfig *push_cfg);

/**
 * Sets the provider for "contact uri parameter". If not set, the default value will be used for "contact uri parameter", "firebase" for android or "apns" for ios.
 * @param push_cfg The #LinphonePushNotificationConfig object @notnil
 * @param provider The new provider set for push notification config. @notnil
**/
LINPHONE_PUBLIC void linphone_push_notification_config_set_provider (LinphonePushNotificationConfig *push_cfg, const char *provider);

/**
* Gets the team id for "contact uri parameter".
* @param push_cfg The #LinphonePushNotificationConfig object @notnil
* @return The team id, default value "ABCD1234". @notnil
*/
LINPHONE_PUBLIC const char *linphone_push_notification_config_get_team_id (const LinphonePushNotificationConfig *push_cfg);

/**
 * Sets the team id for "contact uri parameter". It's not necessary if param is set. See #linphone_push_notification_config_set_param().
 * @param push_cfg The #LinphonePushNotificationConfig object @notnil
 * @param team_id The new team id set for push notification config. @notnil
**/
LINPHONE_PUBLIC void linphone_push_notification_config_set_team_id (LinphonePushNotificationConfig *push_cfg, const char *team_id);

/**
* Gets the msg_str for "contact uri parameter".
* @param push_cfg The #LinphonePushNotificationConfig object @notnil
* @return The msg_str, default value "IM_MSG". @notnil
*/
LINPHONE_PUBLIC const char *linphone_push_notification_config_get_msg_str (const LinphonePushNotificationConfig *push_cfg);

/**
 * Sets the msg_str for "contact uri parameter", specific for remote push notification.
 * @param push_cfg The #LinphonePushNotificationConfig object @notnil
 * @param msg_str The new msg_str set for push notification config. @notnil
**/
LINPHONE_PUBLIC void linphone_push_notification_config_set_msg_str (LinphonePushNotificationConfig *push_cfg, const char *msg_str);

/**
* Gets the call_str for "contact uri parameter".
* @param push_cfg The #LinphonePushNotificationConfig object @notnil
* @return The call_str, default value "IC_MSG". @notnil
*/
LINPHONE_PUBLIC const char *linphone_push_notification_config_get_call_str (const LinphonePushNotificationConfig *push_cfg);

/**
 * Sets the call_str for "contact uri parameter", specific for remote push notification.
 * @param push_cfg The #LinphonePushNotificationConfig object @notnil
 * @param call_str The new call_str set for push notification config. @notnil
**/
LINPHONE_PUBLIC void linphone_push_notification_config_set_call_str (LinphonePushNotificationConfig *push_cfg, const char *call_str);

/**
* Gets the groupchat_str for "contact uri parameter".
* @param push_cfg The #LinphonePushNotificationConfig object @notnil
* @return The groupchat_str, default value "GC_MSG". @notnil
*/
LINPHONE_PUBLIC const char *linphone_push_notification_config_get_group_chat_str (const LinphonePushNotificationConfig *push_cfg);

/**
 * Sets the group_chat_str for "contact uri parameter", specific for remote push notification.
 * @param push_cfg The #LinphonePushNotificationConfig object @notnil
 * @param group_chat_str The new group_chat_str set for push notification config. @notnil
**/
LINPHONE_PUBLIC void linphone_push_notification_config_set_group_chat_str (LinphonePushNotificationConfig *push_cfg, const char *group_chat_str);

/**
* Gets the call_snd for "contact uri parameter".
* @param push_cfg The #LinphonePushNotificationConfig object @notnil
* @return The call_snd, default value "notes_of_the_optimistic.caf". @notnil
*/
LINPHONE_PUBLIC const char *linphone_push_notification_config_get_call_snd (const LinphonePushNotificationConfig *push_cfg);

/**
 * Sets the call_snd for "contact uri parameter", specific for remote push notification.
 * @param push_cfg The #LinphonePushNotificationConfig object @notnil
 * @param call_snd The new call_snd set for push notification config. @notnil
**/
LINPHONE_PUBLIC void linphone_push_notification_config_set_call_snd (LinphonePushNotificationConfig *push_cfg, const char *call_snd);

/**
* Gets the msg_snd for "contact uri parameter".
* @param push_cfg The #LinphonePushNotificationConfig object @notnil
* @return The msg_snd, default value "msg.caf". @notnil
*/
LINPHONE_PUBLIC const char *linphone_push_notification_config_get_msg_snd (const LinphonePushNotificationConfig *push_cfg);

/**
 * Sets the msg_snd for "contact uri parameter", specific for remote push notification.
 * @param push_cfg The #LinphonePushNotificationConfig object @notnil
 * @param msg_snd The new msg_snd set for push notification config. @notnil
**/
LINPHONE_PUBLIC void linphone_push_notification_config_set_msg_snd (LinphonePushNotificationConfig *push_cfg, const char *msg_snd);

/**
* Gets the app's bundle identifier for "contact uri parameter".
* @param push_cfg The #LinphonePushNotificationConfig object @notnil
* @return The app's bundle identifier, default value "". @notnil
*/
LINPHONE_PUBLIC const char *linphone_push_notification_config_get_bundle_identifier (const LinphonePushNotificationConfig *push_cfg);

/**
 * Sets the bundle_identifier for "contact uri parameter". It's not necessary if param is set. See #linphone_push_notification_config_set_param().
 * @param push_cfg The #LinphonePushNotificationConfig object @notnil
 * @param bundle_identifier The new bundle_identifier set for push notification config. @notnil
**/
LINPHONE_PUBLIC void linphone_push_notification_config_set_bundle_identifier (LinphonePushNotificationConfig *push_cfg, const char *bundle_identifier);

/**
* Gets the voip token for "contact uri parameter".
* @param push_cfg The #LinphonePushNotificationConfig object @notnil
* @return The voip token, default value "". @notnil
*/
LINPHONE_PUBLIC const char *linphone_push_notification_config_get_voip_token (const LinphonePushNotificationConfig *push_cfg);

/**
 * Sets the voip_token for "contact uri parameter", specific for voip push notification. It's not necessary if prid is set. See #linphone_push_notification_config_set_prid().
 * @param push_cfg The #LinphonePushNotificationConfig object @notnil
 * @param voip_token The new voip_token set for push notification config. @notnil
**/
LINPHONE_PUBLIC void linphone_push_notification_config_set_voip_token (LinphonePushNotificationConfig *push_cfg, const char *voip_token);

/**
* Gets the remote token for "contact uri parameter".
* @param push_cfg The #LinphonePushNotificationConfig object @notnil
* @return The remote token, default value "". @notnil
*/
LINPHONE_PUBLIC const char *linphone_push_notification_config_get_remote_token (const LinphonePushNotificationConfig *push_cfg);

/**
 * Sets the remote_token for "contact uri parameter", specific for remote push notification. It's not necessary if prid is set. See linphone_push_notification_config_set_prid().
 * @param push_cfg The #LinphonePushNotificationConfig object @notnil
 * @param remote_token The new remote_token set for push notification config. @notnil
**/
LINPHONE_PUBLIC void linphone_push_notification_config_set_remote_token (LinphonePushNotificationConfig *push_cfg, const char *remote_token);

/**
* Gets the param for "contact uri parameter".
* @param push_cfg The #LinphonePushNotificationConfig object @notnil
* @return The param, default value "". @notnil
*/
LINPHONE_PUBLIC const char *linphone_push_notification_config_get_param (const LinphonePushNotificationConfig *push_cfg);

/**
 * Sets the param for "contact uri parameter". If it's not set, "team_id.bundle_identifier.services" will be used.
 * @param push_cfg The #LinphonePushNotificationConfig object @notnil
 * @param param The new param set for push notification config. @notnil
**/
LINPHONE_PUBLIC void linphone_push_notification_config_set_param (LinphonePushNotificationConfig *push_cfg, const char *param);

/**
* Gets the prid for "contact uri parameter".
* @param push_cfg The #LinphonePushNotificationConfig object @notnil
* @return The prid, default value "". @notnil
*/
LINPHONE_PUBLIC const char *linphone_push_notification_config_get_prid (const LinphonePushNotificationConfig *push_cfg);

/**
 * Sets the prid for "contact uri parameter". If it's not set, "voip_token&remote_token" will be used.
 * @param push_cfg The #LinphonePushNotificationConfig object @notnil
 * @param prid The new prid set for push notification config. @notnil
**/
LINPHONE_PUBLIC void linphone_push_notification_config_set_prid (LinphonePushNotificationConfig *push_cfg, const char *prid);

/**
* @}
*/

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_PUSH_NOTIFICATION_CONFIG_H */
