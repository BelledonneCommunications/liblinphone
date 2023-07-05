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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _L_SIP_HEADERS_H_
#define _L_SIP_HEADERS_H_

#include "linphone/utils/general.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

namespace PriorityHeader {
constexpr char HeaderName[] = "Priority";

// Values
constexpr char NonUrgent[] = "non-urgent";
constexpr char Urgent[] = "urgent";
constexpr char Emergency[] = "emergency";
constexpr char Normal[] = "normal";
} // namespace PriorityHeader

namespace XFsMessageTypeHeader {
constexpr char HeaderName[] = "X-fs-message-type";

// Values
constexpr char ChatService[] = "chat-service";
} // namespace XFsMessageTypeHeader

namespace XFsEventIdHeader {
constexpr char HeaderName[] = "X-fs-event-id";
}

LINPHONE_END_NAMESPACE

#endif // _L_SIP_HEADERS_H_
