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

#include "linphone/api/c-chat-message-reaction.h"
#include "c-wrapper/c-wrapper.h"
#include "chat/chat-message/chat-message-reaction.h"

using namespace LinphonePrivate;

const char *linphone_chat_message_reaction_get_body(const LinphoneChatMessageReaction *reaction) {
	return L_STRING_TO_C(ChatMessageReaction::toCpp(reaction)->getBody());
}

const LinphoneAddress *linphone_chat_message_reaction_get_from_address(const LinphoneChatMessageReaction *reaction) {
	return ChatMessageReaction::toCpp(reaction)->getFromAddress()->toC();
}

LinphoneChatMessageReaction *linphone_chat_message_reaction_ref(LinphoneChatMessageReaction *reaction) {
	if (reaction) {
		ChatMessageReaction::toCpp(reaction)->ref();
		return reaction;
	}
	return NULL;
}

void linphone_chat_message_reaction_unref(LinphoneChatMessageReaction *reaction) {
	if (reaction) {
		ChatMessageReaction::toCpp(reaction)->unref();
	}
}

void linphone_chat_message_reaction_send(LinphoneChatMessageReaction *reaction) {
	ChatMessageReaction::toCpp(reaction)->send();
}

const char *linphone_chat_message_reaction_get_call_id(const LinphoneChatMessageReaction *reaction) {
	return L_STRING_TO_C(ChatMessageReaction::toCpp(reaction)->getCallId());
}