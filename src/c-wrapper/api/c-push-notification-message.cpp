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

#include "linphone/api/c-push-notification-message.h"
#include "c-wrapper/c-wrapper.h"
#include "push-notification-message/push-notification-message.h"

using namespace LinphonePrivate;

LinphonePushNotificationMessage *linphone_push_notification_message_new(bool_t is_using_user_defaults,
																		const char *call_id, bool_t is_text,
																		const char *text_content, const char *subject,
																		const char *from_addr, const char *local_addr,
																		const char *peer_addr, const char *from_display_name) {
	return PushNotificationMessage::createCObject(
		is_using_user_defaults, call_id ? call_id : "", is_text, text_content ? text_content : "",
		subject ? subject : "", from_addr ? from_addr : "", local_addr ? local_addr : "", peer_addr ? peer_addr : "",
		from_display_name ? from_display_name : "");
}

LinphonePushNotificationMessage *linphone_push_notification_message_ref(LinphonePushNotificationMessage *msg) {
	if (msg) {
		PushNotificationMessage::toCpp(msg)->ref();
		return msg;
	}
	return NULL;
}

void linphone_push_notification_message_unref(LinphonePushNotificationMessage *msg) {
	if (msg) {
		PushNotificationMessage::toCpp(msg)->unref();
	}
}

bool_t linphone_push_notification_message_is_using_user_defaults(const LinphonePushNotificationMessage *msg) {
	return PushNotificationMessage::toCpp(msg)->isUsingUserDefaults();
}

const char *linphone_push_notification_message_get_call_id(const LinphonePushNotificationMessage *msg) {
	const char *call_id = PushNotificationMessage::toCpp(msg)->getCallId().c_str();
	return strlen(call_id) != 0 ? call_id : NULL;
}

bool_t linphone_push_notification_message_is_text(const LinphonePushNotificationMessage *msg) {
	return PushNotificationMessage::toCpp(msg)->isText();
}

const char *linphone_push_notification_message_get_text_content(const LinphonePushNotificationMessage *msg) {
	const char *text = PushNotificationMessage::toCpp(msg)->getTextContent().c_str();
	return strlen(text) != 0 ? text : NULL;
}

const char *linphone_push_notification_message_get_subject(const LinphonePushNotificationMessage *msg) {
	const char *subject = PushNotificationMessage::toCpp(msg)->getSubject().c_str();
	return strlen(subject) != 0 ? subject : NULL;
}

const LinphoneAddress *linphone_push_notification_message_get_from_addr(const LinphonePushNotificationMessage *msg) {
	return linphone_address_new(PushNotificationMessage::toCpp(msg)->getFromAddr()->asString().c_str());
}

const LinphoneAddress *linphone_push_notification_message_get_local_addr(const LinphonePushNotificationMessage *msg) {
	return linphone_address_new(PushNotificationMessage::toCpp(msg)->getLocalAddr()->asString().c_str());
}

const LinphoneAddress *linphone_push_notification_message_get_peer_addr(const LinphonePushNotificationMessage *msg) {
	return linphone_address_new(PushNotificationMessage::toCpp(msg)->getPeerAddr()->asString().c_str());
}

const char *linphone_push_notification_message_get_from_display_name(const LinphonePushNotificationMessage *msg) {
	const char *displayName = PushNotificationMessage::toCpp(msg)->getFromDisplayName().c_str();
	return strlen(displayName) != 0 ? displayName : NULL;
}
