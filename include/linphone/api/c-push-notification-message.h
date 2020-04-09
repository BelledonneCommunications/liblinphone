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

#ifndef LINPHONE_PUSH_NOTIFICATION_MESSAGE_H
#define LINPHONE_PUSH_NOTIFICATION_MESSAGE_H

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

LINPHONE_PUBLIC LinphonePushNotificationMessage *linphone_push_notification_message_new (
	bool_t is_using_user_defaults,
	const char *call_id,
	bool_t is_text,
	const char *text_content,
	const char *subject,
	const char *from_addr,
	const char *local_addr,
	const char *peer_addr);

/**
 * Take a reference on a #LinphonePushNotificationMessage.
 */
LINPHONE_PUBLIC LinphonePushNotificationMessage *linphone_push_notification_message_ref(LinphonePushNotificationMessage *msg);

/**
 * Release a #LinphonePushNotificationMessage.
 */
LINPHONE_PUBLIC void linphone_push_notification_message_unref(LinphonePushNotificationMessage *msg);

/**
 * is #LinphonePushNotificationMessage build from UserDefaults data or from a #LinphoneChatMessage
 * @param[in] info The #LinphonePushNotificationMessage object
 * @return The is_using_user_defaults.
 */
LINPHONE_PUBLIC bool_t linphone_push_notification_message_is_using_user_defaults(const LinphonePushNotificationMessage *msg);

/**
 * Gets the call_id.
 * @param[in] info The #LinphonePushNotificationMessage object
 * @return The call_id.
 */
LINPHONE_PUBLIC const char *linphone_push_notification_message_get_call_id(const LinphonePushNotificationMessage *msg);

/**
 * return true if it is a text message.
 * @param[in] info The #LinphonePushNotificationMessage object
 * @return The is_text.
 */
LINPHONE_PUBLIC bool_t linphone_push_notification_message_is_text(const LinphonePushNotificationMessage *msg);

/**
 * Gets the text content.
 * @param[in] info The #LinphonePushNotificationMessage object
 * @return The text_content.
 */
LINPHONE_PUBLIC const char *linphone_push_notification_message_get_text_content(const LinphonePushNotificationMessage *msg);

/**
 * Gets the subject.
 * @param[in] info The #LinphonePushNotificationMessage object
 * @return The subject.
 */
LINPHONE_PUBLIC const char *linphone_push_notification_message_get_subject(const LinphonePushNotificationMessage *msg);

/**
 * Gets the from_addr.
 * @param[in] info The #LinphonePushNotificationMessage object
 * @return The from_addr.
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_push_notification_message_get_from_addr(const LinphonePushNotificationMessage *msg);

/**
 * Gets the local_addr.
 * @param[in] info The #LinphonePushNotificationMessage object
 * @return The local_addr.
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_push_notification_message_get_local_addr(const LinphonePushNotificationMessage *msg);

/**
 * Gets the peer_addr.
 * @param[in] info The #LinphonePushNotificationMessage object
 * @return The peer_addr.
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_push_notification_message_get_peer_addr(const LinphonePushNotificationMessage *msg);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_PUSH_NOTIFICATION_MESSAGE_H */