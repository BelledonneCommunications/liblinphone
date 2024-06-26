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

#ifndef _L_IMDN_MESSAGE_P_H_
#define _L_IMDN_MESSAGE_P_H_

#include "chat/chat-message/imdn-message.h"
#include "chat/chat-message/notification-message-p.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ImdnMessagePrivate : public NotificationMessagePrivate {
public:
	const ImdnMessage::Context &getContext() {
		return context;
	}

private:
	ImdnMessagePrivate(const ImdnMessage::Context &context)
	    : NotificationMessagePrivate(context.mChatRoom, ChatMessage::Direction::Outgoing), context(context) {
	}

	void setState(ChatMessage::State newState) override;

	ImdnMessage::Context context;

	L_DECLARE_PUBLIC(ImdnMessage);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_IMDN_MESSAGE_P_H_
