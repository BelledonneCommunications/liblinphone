/*
 * chat-message-killer.cpp
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

#include "linphone/utils/general.h"
#include "chat-message-killer.h"
#include "event-log/event-log.h"
#include "db/main-db.h"
#include "core/core-p.h"
#include "core/core.h"
#include "db/main-db-key-p.h"

#include "private.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// =============================================================================
ChatMessageKiller::ChatMessageKiller (double duration, MainDbEventKey dbKey): duration(duration), dbKey(dbKey) {
	timer = nullptr;
	bgTask.setName("ephemeral message timeout");
}

ChatMessageKiller::ChatMessageKiller (MainDbEventKey dbKey) : ChatMessageKiller(86400, dbKey) {
}
// -----------------------------------------------------------------------------
int ChatMessageKiller::timerExpired (void *data, unsigned int revents) {
	ChatMessageKiller *d = static_cast<ChatMessageKiller *>(data);
	
	// stop timer
	if (d->timer) {
		auto core = d->dbKey.getPrivate()->core.lock()->getCCore();
		if (core && core->sal)
			core->sal->cancelTimer(d->timer);
		belle_sip_object_unref(d->timer);
		d->timer = nullptr;
	}
	d->bgTask.stop();
	
	// delete message in database for ephemral messag
	shared_ptr<LinphonePrivate::EventLog> event = LinphonePrivate::MainDb::getEventFromKey(
																						   d->dbKey
																						   );
	if (event)
		LinphonePrivate::EventLog::deleteFromDatabase(event);

	
	return BELLE_SIP_STOP;
}

void ChatMessageKiller::startTimer () {
	auto core = dbKey.getPrivate()->core.lock();
	if (!timer)
		timer = core->getCCore()->sal->createTimer(timerExpired, this, (unsigned int)duration*1000, "ephemeral message timeout");
	else
		belle_sip_source_set_timeout(timer, (unsigned int)duration*1000);
	bgTask.start(core, 1);
}

void ChatMessageKiller::setDuration(double time) {
	duration = time;
}

LINPHONE_END_NAMESPACE

