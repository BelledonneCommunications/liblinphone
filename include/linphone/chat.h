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

#ifndef LINPHONE_CHAT_H_
#define LINPHONE_CHAT_H_


#include "linphone/callbacks.h"
#include "linphone/types.h"
#include "linphone/api/c-types.h"
#include "linphone/api/c-chat-message.h"
#include "linphone/api/c-chat-message-cbs.h"
#include "linphone/api/c-chat-room-params.h"
#include "linphone/api/c-chat-room.h"
#include "linphone/api/c-chat-room-cbs.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup chatroom
 * @{
 */

/**
 * Returns a list of chat rooms
 * @param core #LinphoneCore object @notnil
 * @return List of chat rooms. \bctbx_list{LinphoneChatRoom} @maybenil
**/
LINPHONE_PUBLIC const bctbx_list_t* linphone_core_get_chat_rooms(LinphoneCore *core);

/**
 * Creates and returns the default chat room parameters.
 * @param core #LinphoneCore object @notnil
 * @return A #LinphoneChatRoomParams object @notnil
**/
LINPHONE_PUBLIC LinphoneChatRoomParams *linphone_core_create_default_chat_room_params(LinphoneCore *core);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif /* LINPHONE_CHAT_H_ */
