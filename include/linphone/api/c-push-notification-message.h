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

LINPHONE_PUBLIC LinphonePushNotificationMessage *linphone_push_notification_message_new(const char *call_id, bool_t is_text,
									   const char *text_content, const char *subject, const char *from_addr,
									   const char *local_addr, const char *peer_addr);

/**
 * Take a reference on a #LinphonePushNotificationMessage.
 * @param message the #LinphonePushNotificationMessage object @notnil
 * @return the same #LinphonePushNotificationMessage object @notnil
 */
LINPHONE_PUBLIC LinphonePushNotificationMessage *linphone_push_notification_message_ref(LinphonePushNotificationMessage *message);

/**
 * Release a #LinphonePushNotificationMessage.
 * @param message the #LinphonePushNotificationMessage object @notnil
 */
LINPHONE_PUBLIC void linphone_push_notification_message_unref(LinphonePushNotificationMessage *message);

/**
 * Gets the call id.
 * @param message The #LinphonePushNotificationMessage object @notnil
 * @return The call id. @notnil
 */
LINPHONE_PUBLIC const char *linphone_push_notification_message_get_call_id(const LinphonePushNotificationMessage *message);

/**
 * Returns wether it is a text message or not.
 * @param message The #LinphonePushNotificationMessage object @notnil
 * @return TRUE if it is a text message, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t linphone_push_notification_message_is_text(const LinphonePushNotificationMessage *message);

/**
 * Gets the text content.
 * @param message The #LinphonePushNotificationMessage object @notnil
 * @return The text content or NULL. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_push_notification_message_get_text_content(const LinphonePushNotificationMessage *message);

/**
 * Gets the subject.
 * @param message The #LinphonePushNotificationMessage object @notnil
 * @return The subject or NULL. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_push_notification_message_get_subject(const LinphonePushNotificationMessage *message);

/**
 * Gets the from address.
 * @param message The #LinphonePushNotificationMessage object @notnil
 * @return The from #LinphoneAddress. @notnil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_push_notification_message_get_from_addr(const LinphonePushNotificationMessage *message);

/**
 * Gets the local address.
 * @param message The #LinphonePushNotificationMessage object @notnil
 * @return The local #LinphoneAddress. @notnil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_push_notification_message_get_local_addr(const LinphonePushNotificationMessage *message);

/**
 * Gets the peer address.
 * @param message The #LinphonePushNotificationMessage object @notnil
 * @return The peer #LinphoneAddress. @notnil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_push_notification_message_get_peer_addr(const LinphonePushNotificationMessage *message);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_PUSH_NOTIFICATION_MESSAGE_H */
