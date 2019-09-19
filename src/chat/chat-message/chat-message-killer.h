/*
 * chat-message-killer.h
 * Copyright (C) 2010-2019 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _L_CHAT_MESSAGE_KILLER_H_
#define _L_CHAT_MESSAGE_KILLER_H_

#include <belle-sip/types.h>
#include <map>

#include "utils/background-task.h"
#include "core/core-accessor.h"

#include "db/main-db-event-key.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ChatMessageKiller {
	friend class MainDb;
	friend class MainDbEventKey;
public:
	explicit ChatMessageKiller (MainDbEventKey dbKey);
	explicit ChatMessageKiller (double duration, MainDbEventKey dbKey);
	
	static int timerExpired (void *data, unsigned int revents);
	
	void startTimer ();
	void setDuration (double time);

private:
	belle_sip_source_t *timer;
	BackgroundTask bgTask;
	double duration;
	MainDbEventKey dbKey;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CHAT_MESSAGE_KILLER_H_

