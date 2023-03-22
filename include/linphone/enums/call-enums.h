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

#ifndef _L_CALL_ENUMS_H_
#define _L_CALL_ENUMS_H_

// =============================================================================
/**
 * #LinphoneCallState enum represents the different states a call can reach into.
 * The application is notified of a state change through the LinphoneCoreVTable::call_state_changed callback.
 * @ingroup call_control
 */
typedef enum _LinphoneCallState{
	LinphoneCallStateIdle = 0, /**< Initial state */
	LinphoneCallStateIncomingReceived = 1, /**< Incoming call received */
	LinphoneCallStatePushIncomingReceived = 2, /**< PushIncoming call received */
	LinphoneCallStateOutgoingInit = 3, /**< Outgoing call initialized */
	LinphoneCallStateOutgoingProgress = 4, /**< Outgoing call in progress */
	LinphoneCallStateOutgoingRinging = 5, /**< Outgoing call ringing */
	LinphoneCallStateOutgoingEarlyMedia = 6, /**< Outgoing call early media */
	LinphoneCallStateConnected = 7, /**< Connected */
	LinphoneCallStateStreamsRunning = 8, /**< Streams running */
	LinphoneCallStatePausing = 9, /**< Pausing */
	LinphoneCallStatePaused = 10, /**< Paused */
	LinphoneCallStateResuming = 11, /**< Resuming */
	LinphoneCallStateReferred = 12, /**< Referred */
	LinphoneCallStateError = 13, /**< Error */
	LinphoneCallStateEnd = 14, /**< Call end */
	LinphoneCallStatePausedByRemote = 15, /**< Paused by remote */
	LinphoneCallStateUpdatedByRemote = 16, /**< The call&apos;s parameters are updated for example when video is asked by remote */
	LinphoneCallStateIncomingEarlyMedia = 17, /**< We are proposing early media to an incoming call */
	LinphoneCallStateUpdating = 18, /**< We have initiated a call update */
	LinphoneCallStateReleased = 19, /**< The call object is now released */
	LinphoneCallStateEarlyUpdatedByRemote = 20, /**< The call is updated by remote while not yet answered (SIP UPDATE in early dialog received) */
	LinphoneCallStateEarlyUpdating = 21, /**< We are updating the call while not yet answered (SIP UPDATE in early dialog sent) */
} LinphoneCallState;

/**
 * #LinphoneAudioDeviceType enum represents the different types of an audio device.
 * @ingroup audio
 */
typedef enum _LinphoneAudioDeviceType {
	LinphoneAudioDeviceTypeUnknown = 0,		  /**< Unknown */
	LinphoneAudioDeviceTypeMicrophone = 1,	  /**< Microphone */
	LinphoneAudioDeviceTypeEarpiece = 2,	  /**< Earpiece */
	LinphoneAudioDeviceTypeSpeaker = 3,		  /**< Speaker */
	LinphoneAudioDeviceTypeBluetooth = 4,	  /**< Bluetooth */
	LinphoneAudioDeviceTypeBluetoothA2DP = 5, /**< Bluetooth A2DP */
	LinphoneAudioDeviceTypeTelephony = 6,	  /**< Telephony */
	LinphoneAudioDeviceTypeAuxLine = 7,		  /**< AuxLine */
	LinphoneAudioDeviceTypeGenericUsb = 8,	  /**< GenericUsb */
	LinphoneAudioDeviceTypeHeadset = 9,		  /**< Headset */
	LinphoneAudioDeviceTypeHeadphones = 10,	  /**< Headphones */
	LinphoneAudioDeviceTypeHearingAid = 11,	  /**< Hearing Aid */
} LinphoneAudioDeviceType;

/**
 * #LinphoneAudioDeviceCapabilities enum represents whether a device can record audio, play audio or both
 * @ingroup audio
 */
typedef enum _LinphoneAudioDeviceCapabilities {
	LinphoneAudioDeviceCapabilityRecord = 1 << 0, /**< Can record audio */
	LinphoneAudioDeviceCapabilityPlay = 1 << 1,	  /**< Can play audio */
	LinphoneAudioDeviceCapabilityAll = 3,		  /**< Can play and record audio */
} LinphoneAudioDeviceCapabilities;

// =============================================================================
// DEPRECATED
// =============================================================================

#define LinphoneCallIdle LinphoneCallStateIdle
#define LinphoneCallIncomingReceived LinphoneCallStateIncomingReceived
#define LinphoneCallPushIncomingReceived LinphoneCallStatePushIncomingReceived
#define LinphoneCallOutgoingInit LinphoneCallStateOutgoingInit
#define LinphoneCallOutgoingProgress LinphoneCallStateOutgoingProgress
#define LinphoneCallOutgoingRinging LinphoneCallStateOutgoingRinging
#define LinphoneCallOutgoingEarlyMedia LinphoneCallStateOutgoingEarlyMedia
#define LinphoneCallConnected LinphoneCallStateConnected
#define LinphoneCallStreamsRunning LinphoneCallStateStreamsRunning
#define LinphoneCallPausing LinphoneCallStatePausing
#define LinphoneCallPaused LinphoneCallStatePaused
#define LinphoneCallResuming LinphoneCallStateResuming
#define LinphoneCallRefered LinphoneCallStateReferred
#define LinphoneCallError LinphoneCallStateError
#define LinphoneCallEnd LinphoneCallStateEnd
#define LinphoneCallPausedByRemote LinphoneCallStatePausedByRemote
#define LinphoneCallUpdatedByRemote LinphoneCallStateUpdatedByRemote
#define LinphoneCallIncomingEarlyMedia LinphoneCallStateIncomingEarlyMedia
#define LinphoneCallUpdating LinphoneCallStateUpdating
#define LinphoneCallEarlyUpdating LinphoneCallStateEarlyUpdating
#define LinphoneCallReleased LinphoneCallStateReleased
#define LinphoneCallEarlyUpdatedByRemote LinphoneCallStateEarlyUpdatedByRemote

#endif // ifndef _L_CALL_ENUMS_H_
