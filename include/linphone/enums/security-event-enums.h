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

#ifndef _L_SECURITY_EVENT_ENUMS_H_
#define _L_SECURITY_EVENT_ENUMS_H_

// =============================================================================

/**
 * #LinphoneSecurityEventType is used to indicate the type of security event.
 * @ingroup events
 */

typedef enum _SecurityEventType{
	LinphoneSecurityEventTypeNone, /**< Event is not a security event */
	LinphoneSecurityEventTypeSecurityLevelDowngraded, /**< Chatroom security level downgraded event */
	LinphoneSecurityEventTypeParticipantMaxDeviceCountExceeded, /**< Participant has exceeded the maximum number of device event */
	LinphoneSecurityEventTypeEncryptionIdentityKeyChanged, /**< Peer device instant messaging encryption identity key has changed event */
	LinphoneSecurityEventTypeManInTheMiddleDetected, /**< Man in the middle detected event */
} LinphoneSecurityEventType;


#endif // ifndef _L_SECURITY_EVENT_ENUMS_H_
