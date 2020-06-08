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

#ifndef _L_EVENT_LOG_P_H_
#define _L_EVENT_LOG_P_H_

#include "db/main-db-event-key.h"
#include "object/base-object-p.h"

#include "event-log.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class EventLogPrivate : public BaseObjectPrivate {
public:
	mutable MainDbEventKey dbKey;

private:
	EventLog::Type type = EventLog::Type::None;
	time_t creationTime = -1;

	L_DECLARE_PUBLIC(EventLog);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_EVENT_LOG_P_H_
