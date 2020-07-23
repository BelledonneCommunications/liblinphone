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

#ifndef _L_REMOTE_CONFERENCE_EVENT_HANDLER_BASE_H_
#define _L_REMOTE_CONFERENCE_EVENT_HANDLER_BASE_H_

#include "conference/conference-id.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ConferenceId;

class LINPHONE_PUBLIC RemoteConferenceEventHandlerBase {
	friend class ClientGroupChatRoom;

public:
	virtual ~RemoteConferenceEventHandlerBase() = default;

	virtual void subscribe () = 0;
	virtual void unsubscribe () = 0;
	virtual void invalidateSubscription () = 0;

};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_REMOTE_CONFERENCE_EVENT_HANDLER_BASE_H_
