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

#ifndef LINPHONE_CHAT_MESSAGE_REACTION_H
#define LINPHONE_CHAT_MESSAGE_REACTION_H

#include "linphone/api/c-types.h"

/**
 * @addtogroup chatroom
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns the emoji(s) used for the reaction.
 * @param reaction the #LinphoneChatMessageReaction. @notnil
 * @return the emoji(s) used as UTF-8 characters. @notnil
 */
LINPHONE_PUBLIC const char *linphone_chat_message_reaction_get_body(const LinphoneChatMessageReaction *reaction);

/**
 * Returns the #LinphoneAddress of the participant that sent this reaction.
 * @param reaction the #LinphoneChatMessageReaction. @notnil
 * @return the #LinphoneAddress that sent this reaction. @notnil
 */
LINPHONE_PUBLIC const LinphoneAddress *
linphone_chat_message_reaction_get_from_address(const LinphoneChatMessageReaction *reaction);

/**
 * Takes a reference on a #LinphoneChatMessageReaction.
 * @param reaction the #LinphoneChatMessageReaction. @notnil
 * @return the same #LinphoneChatMessageReaction object @notnil
 */
LINPHONE_PUBLIC LinphoneChatMessageReaction *linphone_chat_message_reaction_ref(LinphoneChatMessageReaction *reaction);

/**
 * Releases a #LinphoneChatMessageReaction reference.
 * @param reaction the #LinphoneChatMessageReaction @notnil
 */
LINPHONE_PUBLIC void linphone_chat_message_reaction_unref(LinphoneChatMessageReaction *reaction);

/**
 * Sends a #LinphoneChatMessageReaction.
 * @param reaction the #LinphoneChatMessageReaction to send. @notnil
 */
LINPHONE_PUBLIC void linphone_chat_message_reaction_send(LinphoneChatMessageReaction *reaction);

/**
 * Allows to get the Call ID associated with a #LinphoneChatMessageReaction.
 * @param reaction the #LinphoneChatMessageReaction. @notnil
 * @return the Call ID associated with this reaction.
 */
LINPHONE_PUBLIC const char *linphone_chat_message_reaction_get_call_id(const LinphoneChatMessageReaction *reaction);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif // LINPHONE_CHAT_MESSAGE_REACTION_H