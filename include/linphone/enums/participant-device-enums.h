/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#ifndef _L_PARTICIPANT_DEVICE_ENUMS_H_
#define _L_PARTICIPANT_DEVICE_ENUMS_H_
// =============================================================================

/**
 * #LinphoneParticipantDeviceState is used to list all states a participant device can be in
 * @ingroup conference
 */
typedef enum _LinphoneParticipantDeviceState {
	LinphoneParticipantDeviceStateJoining = 0,  /**< an INVITE has been sent */
	LinphoneParticipantDeviceStatePresent = 1,  /**< the SIP session has been concluded, participant is part of the conference */
	LinphoneParticipantDeviceStateLeaving = 2,  /**< A BYE is pending */
	LinphoneParticipantDeviceStateLeft = 3,  /**< The Session is terminated */
	LinphoneParticipantDeviceStateScheduledForJoining = 4,  /**< Initial state for the server group chatroom, when the participant has not yet been INVITEd. */
	LinphoneParticipantDeviceStateScheduledForLeaving = 5,  /**< Transitional state for a participant that will receive a BYE shortly. */
} LinphoneParticipantDeviceState;

#endif // ifndef _L_PARTICIPANT_DEVICE_ENUMS_H_
