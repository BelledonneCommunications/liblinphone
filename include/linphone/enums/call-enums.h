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

#ifndef _L_CALL_ENUMS_H_
#define _L_CALL_ENUMS_H_

// =============================================================================
/**
 * #LinphoneCallState enum represents the different states a call can reach into.
 * The application is notified of a state change through the LinphoneCoreVTable::call_state_changed callback.
 * @ingroup call_control
 */
typedef enum _LinphoneCallState{
	LinphoneCallStateIdle, /**< Initial state */
	LinphoneCallStateIncomingReceived, /**< Incoming call received */
	LinphoneCallStatePushIncomingReceived, /**< PushIncoming call received */
	LinphoneCallStateOutgoingInit, /**< Outgoing call initialized */
	LinphoneCallStateOutgoingProgress, /**< Outgoing call in progress */
	LinphoneCallStateOutgoingRinging, /**< Outgoing call ringing */
	LinphoneCallStateOutgoingEarlyMedia, /**< Outgoing call early media */
	LinphoneCallStateConnected, /**< Connected */
	LinphoneCallStateStreamsRunning, /**< Streams running */
	LinphoneCallStatePausing, /**< Pausing */
	LinphoneCallStatePaused, /**< Paused */
	LinphoneCallStateResuming, /**< Resuming */
	LinphoneCallStateReferred, /**< Referred */
	LinphoneCallStateError, /**< Error */
	LinphoneCallStateEnd, /**< Call end */
	LinphoneCallStatePausedByRemote, /**< Paused by remote */
	LinphoneCallStateUpdatedByRemote, /**< The call&apos;s parameters are updated for example when video is asked by remote */
	LinphoneCallStateIncomingEarlyMedia, /**< We are proposing early media to an incoming call */
	LinphoneCallStateUpdating, /**< We have initiated a call update */
	LinphoneCallStateReleased, /**< The call object is now released */
	LinphoneCallStateEarlyUpdatedByRemote, /**< The call is updated by remote while not yet answered (SIP UPDATE in early dialog received) */
	LinphoneCallStateEarlyUpdating, /**< We are updating the call while not yet answered (SIP UPDATE in early dialog sent) */
} LinphoneCallState;

/**
 * #LinphoneAudioDeviceType enum represents the different types of an audio device.
 * @ingroup audio
 */
typedef enum _LinphoneAudioDeviceType {
	LinphoneAudioDeviceTypeUnknown, /** Unknown */
	LinphoneAudioDeviceTypeMicrophone, /** Microphone */
	LinphoneAudioDeviceTypeEarpiece, /** Earpiece */
	LinphoneAudioDeviceTypeSpeaker, /** Speaker */
	LinphoneAudioDeviceTypeBluetooth, /** Bluetooth */
	LinphoneAudioDeviceTypeBluetoothA2DP, /** Bluetooth A2DP */
	LinphoneAudioDeviceTypeTelephony, /** Telephony */
	LinphoneAudioDeviceTypeAuxLine, /** AuxLine */
	LinphoneAudioDeviceTypeGenericUsb, /** GenericUsb */
	LinphoneAudioDeviceTypeHeadset, /** Headset */
	LinphoneAudioDeviceTypeHeadphones, /** Headphones */
} LinphoneAudioDeviceType;

/**
 * #LinphoneAudioDeviceCapabilities enum represents whether a device can record audio, play audio or both
 * @ingroup audio
 */
typedef enum _LinphoneAudioDeviceCapabilities {
	LinphoneAudioDeviceCapabilityRecord = 1 << 0, /** Can record audio */
	LinphoneAudioDeviceCapabilityPlay = 1 << 1, /** Can play audio */
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
