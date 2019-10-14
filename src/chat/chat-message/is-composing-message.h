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

#ifndef _L_IS_COMPOSING_MESSAGE_H_
#define _L_IS_COMPOSING_MESSAGE_H_

#include "chat/chat-message/notification-message.h"
#include "chat/notification/is-composing.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC IsComposingMessage : public NotificationMessage {
public:
	friend class ChatRoomPrivate;

	L_OVERRIDE_SHARED_FROM_THIS(IsComposingMessage);

	virtual ~IsComposingMessage () = default;

private:
	IsComposingMessage (
		const std::shared_ptr<AbstractChatRoom> &chatRoom,
		IsComposing &isComposingHandler,
		bool isComposing
	);

	L_DECLARE_PRIVATE(NotificationMessage);
	L_DISABLE_COPY(IsComposingMessage);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_IS_COMPOSING_MESSAGE_H_
