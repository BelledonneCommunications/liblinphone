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

#ifndef LINPHONE_DAEMON_MESSAGE_H
#define LINPHONE_DAEMON_MESSAGE_H

#include "daemon.h"

class MessageCommand: public DaemonCommand {
public:
	MessageCommand();

	void exec(Daemon *app, const std::string &args) override;
private:
	static void sMsgStateChanged(LinphoneChatMessage *msg, LinphoneChatMessageState state);
};

class IncomingMessageEvent: public Event {
public:
	IncomingMessageEvent(LinphoneChatMessage *msg);
};

class OutgoingMessageEvent: public Event {
public:
	OutgoingMessageEvent(LinphoneChatMessage *msg);
};

#endif // LINPHONE_DAEMON_COMMAND_AUDIO_CODEC_SET_H_
