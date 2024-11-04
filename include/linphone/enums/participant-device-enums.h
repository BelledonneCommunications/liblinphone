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

#ifndef _L_PARTICIPANT_DEVICE_ENUMS_H_
#define _L_PARTICIPANT_DEVICE_ENUMS_H_
// =============================================================================

// WARNING: Participant device state may be stored in the database (as it is done by the conference server), therefore
// do not modify the integer value of the enumeration items
/**
 * #LinphoneParticipantDeviceState is used to list all states a participant device can be in
 * @ingroup conference
 */
typedef enum _LinphoneParticipantDeviceState {
	LinphoneParticipantDeviceStateJoining = 0, /**< an INVITE has been sent */
	LinphoneParticipantDeviceStatePresent =
	    1, /**< the SIP session has been concluded, participant is part of the conference */
	LinphoneParticipantDeviceStateLeaving = 2, /**< A BYE is pending */
	LinphoneParticipantDeviceStateLeft = 3,    /**< The Session is terminated */
	LinphoneParticipantDeviceStateScheduledForJoining =
	    4, /**< Initial state for the server group chatroom, when the participant has not yet been INVITEd. */
	LinphoneParticipantDeviceStateScheduledForLeaving =
	    5, /**< Transitional state for a participant that will receive a BYE shortly. */
	LinphoneParticipantDeviceStateOnHold = 6, /**< the SIP session has been concluded, participant is not media mixed */
	LinphoneParticipantDeviceStateAlerting = 7,     /**< 180 Ringing */
	LinphoneParticipantDeviceStateMutedByFocus = 8, /**< Some medias have been muted by the focus */
	LinphoneParticipantDeviceStateRequestingToJoin =
	    9 /**< the participant has sent a request to join the conference as he/she didn't receive any invitation for */
} LinphoneParticipantDeviceState;

/**
 * #LinphoneParticipantDeviceJoiningMethod is used to indicate how a participant joined a conference or if it is the
 * focus.
 * @ingroup conference
 */
typedef enum _LinphoneParticipantDeviceJoiningMethod {
	LinphoneParticipantDeviceJoiningMethodDialedIn = 0,  /**< device called the conference */
	LinphoneParticipantDeviceJoiningMethodDialedOut = 1, /**< device is called the conference */
	LinphoneParticipantDeviceJoiningMethodFocusOwner = 2 /**< device is the focus */
} LinphoneParticipantDeviceJoiningMethod;

/**
 * #LinphoneParticipantDeviceDisconnectionMethod is used to indicate how a participant left a conference.
 * @ingroup conference
 */
typedef enum _LinphoneParticipantDeviceDisconnectionMethod {
	LinphoneParticipantDeviceDisconnectionMethodBooted = 0,   /**< an admin removes the device from a conference */
	LinphoneParticipantDeviceDisconnectionMethodDeparted = 1, /**< the device disconnects from the conference */
	LinphoneParticipantDeviceDisconnectionMethodBusy = 2,     /**< device is busy */
	LinphoneParticipantDeviceDisconnectionMethodFailed =
	    3 /**< an error occurred while the device is leaving the conference or he declined a call from the server */
} LinphoneParticipantDeviceDisconnectionMethod;

#endif // ifndef _L_PARTICIPANT_DEVICE_ENUMS_H_
