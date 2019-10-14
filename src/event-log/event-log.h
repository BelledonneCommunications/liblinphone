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

#ifndef _L_EVENT_LOG_H_
#define _L_EVENT_LOG_H_

#include <ctime>

#include "linphone/enums/event-log-enums.h"
#include "linphone/utils/enum-generator.h"

#include "object/base-object.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class EventLogPrivate;

class LINPHONE_PUBLIC EventLog : public BaseObject {
	friend class MainDb;
	friend class MainDbPrivate;

public:
	L_DECLARE_ENUM(Type, L_ENUM_VALUES_EVENT_LOG_TYPE);

	EventLog ();

	Type getType () const;
	time_t getCreationTime () const;

	static void deleteFromDatabase (const std::shared_ptr<const EventLog> &eventLog);

protected:
	EventLog (EventLogPrivate &p, Type type, time_t creationTime);

private:
	L_DECLARE_PRIVATE(EventLog);
	L_DISABLE_COPY(EventLog);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_EVENT_LOG_H_
