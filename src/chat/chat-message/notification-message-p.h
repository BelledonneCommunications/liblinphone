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

#ifndef _L_NOTIFICATION_MESSAGE_P_H_
#define _L_NOTIFICATION_MESSAGE_P_H_

#include <bctoolbox/defs.h>

#include "chat/chat-message/chat-message-p.h"
#include "chat/chat-message/notification-message.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class NotificationMessagePrivate : public ChatMessagePrivate {
	friend class ImdnMessage;
	friend class IsComposingMessage;

protected:
	NotificationMessagePrivate(const std::shared_ptr<AbstractChatRoom> &chatRoom, ChatMessage::Direction dir)
	    : ChatMessagePrivate(chatRoom, dir) {
	}

	void setState(BCTBX_UNUSED(ChatMessage::State state)) override{};

private:
	void setDisplayNotificationRequired(BCTBX_UNUSED(bool value)) override {
	}
	void setNegativeDeliveryNotificationRequired(BCTBX_UNUSED(bool value)) override {
	}
	void setPositiveDeliveryNotificationRequired(BCTBX_UNUSED(bool value)) override {
	}

	L_DECLARE_PUBLIC(NotificationMessage);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_NOTIFICATION_MESSAGE_P_H_
