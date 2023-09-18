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

#include "linphone/api/c-push-notification-message.h"
#include "c-wrapper/c-wrapper.h"
#include "push-notification-message/push-notification-message.h"

using namespace LinphonePrivate;

LinphonePushNotificationMessage *linphone_push_notification_message_new(const char *call_id,
                                                                        bool_t is_text,
                                                                        const char *text_content,
                                                                        const char *subject,
                                                                        const char *from_addr,
                                                                        const char *local_addr,
                                                                        const char *peer_addr,
                                                                        const char *reaction_content,
                                                                        bool_t is_icalendar,
                                                                        bool_t is_conference_invitation_new,
                                                                        bool_t is_conference_invitation_update,
                                                                        bool_t is_conference_invitation_cancellation) {
	return PushNotificationMessage::createCObject(
	    L_C_TO_STRING(call_id), is_text, L_C_TO_STRING(text_content), L_C_TO_STRING(subject), L_C_TO_STRING(from_addr),
	    L_C_TO_STRING(local_addr), L_C_TO_STRING(peer_addr), L_C_TO_STRING(reaction_content), is_icalendar,
	    is_conference_invitation_new, is_conference_invitation_update, is_conference_invitation_cancellation);
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

const char *linphone_push_notification_message_get_call_id(const LinphonePushNotificationMessage *msg) {
	return L_STRING_TO_C(PushNotificationMessage::toCpp(msg)->getCallId());
}

bool_t linphone_push_notification_message_is_text(const LinphonePushNotificationMessage *msg) {
	return PushNotificationMessage::toCpp(msg)->isText();
}

const char *linphone_push_notification_message_get_text_content(const LinphonePushNotificationMessage *msg) {
	return L_STRING_TO_C(PushNotificationMessage::toCpp(msg)->getTextContent());
}

const char *linphone_push_notification_message_get_subject(const LinphonePushNotificationMessage *msg) {

	return L_STRING_TO_C(PushNotificationMessage::toCpp(msg)->getSubject());
}

const LinphoneAddress *linphone_push_notification_message_get_from_addr(const LinphonePushNotificationMessage *msg) {
	return PushNotificationMessage::toCpp(msg)->getFromAddr()->toC();
}

const LinphoneAddress *linphone_push_notification_message_get_local_addr(const LinphonePushNotificationMessage *msg) {
	return PushNotificationMessage::toCpp(msg)->getLocalAddr()->toC();
}

const LinphoneAddress *linphone_push_notification_message_get_peer_addr(const LinphonePushNotificationMessage *msg) {
	return PushNotificationMessage::toCpp(msg)->getPeerAddr()->toC();
}

const char *linphone_push_notification_message_get_reaction_content(const LinphonePushNotificationMessage *msg) {
	return L_STRING_TO_C(PushNotificationMessage::toCpp(msg)->getReactionContent());
}

bool_t linphone_push_notification_message_is_icalendar(const LinphonePushNotificationMessage *msg) {
	return PushNotificationMessage::toCpp(msg)->isIcalendar();
}

bool_t linphone_push_notification_message_is_conference_invitation_new(const LinphonePushNotificationMessage *msg) {
	return PushNotificationMessage::toCpp(msg)->isConferenceInvitationNew();
}

bool_t linphone_push_notification_message_is_conference_invitation_update(const LinphonePushNotificationMessage *msg) {
	return PushNotificationMessage::toCpp(msg)->isConferenceInvitationUpdate();
}

bool_t
linphone_push_notification_message_is_conference_invitation_cancellation(const LinphonePushNotificationMessage *msg) {
	return PushNotificationMessage::toCpp(msg)->isConferenceInvitationCancellation();
}
