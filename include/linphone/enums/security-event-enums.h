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

#ifndef _L_SECURITY_EVENT_ENUMS_H_
#define _L_SECURITY_EVENT_ENUMS_H_

// =============================================================================

/**
 * #LinphoneSecurityEventType is used to indicate the type of security event.
 * @ingroup events
 */

typedef enum _SecurityEventType {
	LinphoneSecurityEventTypeNone = 0,                    /**< Event is not a security event */
	LinphoneSecurityEventTypeSecurityLevelDowngraded = 1, /**< Chatroom security level downgraded event */
	LinphoneSecurityEventTypeParticipantMaxDeviceCountExceeded =
	    2, /**< Participant has exceeded the maximum number of device event */
	LinphoneSecurityEventTypeEncryptionIdentityKeyChanged =
	    3, /**< Peer device instant messaging encryption identity key has changed event */
	LinphoneSecurityEventTypeManInTheMiddleDetected = 4, /**< Man in the middle detected event */
} LinphoneSecurityEventType;

#endif // ifndef _L_SECURITY_EVENT_ENUMS_H_
