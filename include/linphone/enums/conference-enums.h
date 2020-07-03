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

#ifndef _L_CONFERENCE_ENUMS_H_
#define _L_CONFERENCE_ENUMS_H_

// =============================================================================
/**
 * #LinphoneConferenceState is used to indicate the current state of a conference.
 * @ingroup conferencing
 */
typedef enum _LinphoneConferenceState{
	LinphoneConferenceStateNone, /**< Initial state */
	LinphoneConferenceStateInstantiated, /**< Chat room is now instantiated on local */
	LinphoneConferenceStateCreationPending, /**< One creation request was sent to the server */
	LinphoneConferenceStateCreated, /**< Chat room was created on the server */
	LinphoneConferenceStateCreationFailed, /**< Chat room creation failed */
	LinphoneConferenceStateTerminationPending, /**< Wait for chat room termination */
	LinphoneConferenceStateTerminated, /**< Chat room exists on server but not in local */
	LinphoneConferenceStateTerminationFailed, /**< The chat room termination failed */
	LinphoneConferenceStateDeleted /**< Chat room was deleted on the server */
} LinphoneConferenceState;

#endif // ifndef _L_CONFERENCE_ENUMS_H_
