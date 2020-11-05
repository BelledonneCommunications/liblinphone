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

#ifndef _L_C_TYPES_H_
#define _L_C_TYPES_H_

// TODO: Remove me in the future.
#include "linphone/types.h"

#include "linphone/enums/call-enums.h"
#include "linphone/enums/chat-message-enums.h"
#include "linphone/enums/chat-room-enums.h"
#include "linphone/enums/conference-enums.h"
#include "linphone/enums/encryption-engine-enums.h"
#include "linphone/enums/event-log-enums.h"
#include "linphone/enums/security-event-enums.h"
#include "linphone/utils/enum-generator.h"

// =============================================================================

#ifdef __cplusplus
	extern "C" {
#endif // ifdef __cplusplus

// =============================================================================
// Misc.
// =============================================================================

#ifdef TRUE
	#undef TRUE
#endif

#ifdef FALSE
	#undef FALSE
#endif

#define TRUE 1
#define FALSE 0

// -----------------------------------------------------------------------------
// Authentication.
// -----------------------------------------------------------------------------

/**
 * Object holding authentication information.
 *
 * In most case, authentication information consists of a username and password.
 * If realm isn't set, it will be deduced automatically from the first authentication challenge as for the hash algorithm.
 * Sometimes, a userid is required by the proxy and then domain can be useful to discriminate different credentials.
 * You can also use this object if you need to use a client certificate.
 *
 * Once created and filled, a #LinphoneAuthInfo must be added to the #LinphoneCore in
 * order to become known and used automatically when needed.
 * Use linphone_core_add_auth_info() for that purpose.
 *
 * The #LinphoneCore object can take the initiative to request authentication information
 * when needed to the application through the authentication_requested() callback of it's #LinphoneCoreCbs.
 *
 * The application can respond to this information request later using
 * linphone_core_add_auth_info(). This will unblock all pending authentication
 * transactions and retry them with authentication headers.
 *
 * @ingroup authentication
**/
typedef struct _LinphoneAuthInfo LinphoneAuthInfo;

// -----------------------------------------------------------------------------
// Address.
// -----------------------------------------------------------------------------

/**
 * Object that represents a parsed SIP address.
 *
 * A SIP address is made of display name, username, domain name, port, and various
 * uri headers (such as tags). It looks like 'Alice <sip:alice@example.net>'.
 * 
 * You can create an address using linphone_factory_create_address() or linphone_core_interpret_url(),
 * and both will return a NULL object if it doesn't match the grammar defined by the standard.
 * 
 * This object is used in almost every other major objects to identity people (including yourself) & servers.
 * 
 * The #LinphoneAddress has methods to extract and manipulate all parts of the address.
 * @ingroup linphone_address
 */
typedef struct _LinphoneAddress LinphoneAddress;

// -----------------------------------------------------------------------------
// Conference.
// -----------------------------------------------------------------------------

/**
 * TODO
 * @ingroup conferencing
 */
typedef struct _LinphoneConference LinphoneConference;

/**
 * TODO
 * @ingroup conferencing
 */
typedef struct _LinphoneConferenceParams LinphoneConferenceParams;

/**
 * An object to handle the callbacks for the handling a #LinphoneConference objects.
 *
 * Use linphone_factory_create_conference_cbs() to create an instance. 
 * Then pass the object to a #LinphoneConference instance through linphone_conference_add_callbacks().
 * @ingroup conferencing
 */
typedef struct _LinphoneConferenceCbs LinphoneConferenceCbs;

// -----------------------------------------------------------------------------
// Participants.
// -----------------------------------------------------------------------------

/**
 * Identifies a member of a #LinphoneConference or #LinphoneChatRoom.
 * 
 * A participant is identified by it's SIP address.
 * It can have many devices, @see #LinphoneParticipantDevice.
 * @ingroup conference
 */
typedef struct _LinphoneParticipant LinphoneParticipant;

/**
 * This object represents a unique device for a member of a #LinphoneConference or #LinphoneChatRoom.
 * Devices are identified by the gruu parameter inside the #LinphoneAddress which can be obtained by linphone_participant_device_get_address().
 * It is specially usefull to know the security level of each device inside an end-to-end encrypted #LinphoneChatRoom.
 * 
 * You can get a list of all #LinphoneParticipantDevice using linphone_participant_get_devices().
 * 
 * @ingroup conference
 */
typedef struct _LinphoneParticipantDevice LinphoneParticipantDevice;

/**
 * This object represents the delivery/display state of a given chat message for a given participant.
 * It also contains a timestamp at which this participant state has changed.
 * 
 * Use linphone_chat_message_get_participants_by_imdn_state() to get all #LinphoneParticipantImdnState for a given state.
 * From there use linphone_participant_imdn_state_get_participant() to get the #LinphoneParticipant object if you need it.
 * @ingroup conference
 */
typedef struct _LinphoneParticipantImdnState LinphoneParticipantImdnState;

/**
 * This object is only used on server side for group chat rooms.
 * @see linphone_chat_room_set_participant_devices()
 * @ingroup conference
 */
typedef struct _LinphoneParticipantDeviceIdentity LinphoneParticipantDeviceIdentity;

// -----------------------------------------------------------------------------
// Call.
// -----------------------------------------------------------------------------

/**
 * TODO
 * The #LinphoneCall object represents a call issued or received by the #LinphoneCore
 * @ingroup call_control
 */
typedef struct _LinphoneCall LinphoneCall;

/** Callback prototype */
typedef void (*LinphoneCallCbFunc) (LinphoneCall *call, void *ud);

/**
 * That class holds all the callbacks which are called by #LinphoneCall objects.
 *
 * Use linphone_factory_create_call_cbs() to create an instance. Then, call the
 * callback setters on the events you need to monitor and pass the object to
 * a #LinphoneCall instance through linphone_call_add_callbacks().
 * @ingroup call_control
 */
typedef struct _LinphoneCallCbs LinphoneCallCbs;

/**
 * An object containing various parameters of a #LinphoneCall.
 * 
 * You can specify your params while answering an incoming call using linphone_call_accept_with_params() 
 * or while initiating an outgoing call with linphone_core_invite_address_with_params().
 * 
 * This object can be created using linphone_core_create_call_params(), using NULL for the call pointer if you plan to use it for an outgoing call.
 * 
 * For each call, three #LinphoneCallParams are available: yours, your correspondent's 
 * and the one that describe the current state of the call that is the result of the negociation between the previous two.
 * For example, you might enable a certain feature in your call param but this feature can be denied in the remote's configuration, hence the difference.
 * 
 * @see linphone_call_get_current_params(), linphone_call_get_remote_params() and linphone_call_get_params().
 * @ingroup call_control
**/
typedef struct _LinphoneCallParams LinphoneCallParams;

/**
 * Object used to keep track of all calls initiated, received or missed.
 * 
 * It contains the call ID, date & time at which the call took place and it's duration (0 if it wasn't answered).
 * You can also know if video was enabled or not or if it was a conference, as well as it's average quality.
 * 
 * If needed, you can also create a fake #LinphoneCallLog using linphone_core_create_call_log(), 
 * otherwise use linphone_core_get_call_logs() or even linphone_call_get_call_log() to get the log of an ongoing call.
 * 
 * @ingroup call_logs
**/
typedef struct _LinphoneCallLog LinphoneCallLog;

// -----------------------------------------------------------------------------
// Audio.
// -----------------------------------------------------------------------------

/**
 * Object holding audio device information.
 * 
 * It contains the name of the device, it's type if available (Earpiece, Speaker, Bluetooth, etc..)
 * and capabilities (input, output or both) the name of the driver that created it (filter in mediastreamer).
 * 
 * You can use the #LinphoneAudioDevice objects to configure default input/output devices or do it dynamically during a call.
 * 
 * To get the list of available devices, use linphone_core_get_audio_devices(). This list will be limited to one device of each type. 
 * Use linphone_core_get_extended_audio_devices() for a complete list. 
 * 
 * @ingroup audio
**/
typedef struct _LinphoneAudioDevice LinphoneAudioDevice;

// -----------------------------------------------------------------------------
// ChatRoom.
// -----------------------------------------------------------------------------

/**
 * TODO
 * An chat message is the object that is sent or received through a #LinphoneChatRoom.
 * @ingroup chatroom
 */
typedef struct _LinphoneChatMessage LinphoneChatMessage;

/**
 * An object to handle the callbacks for the handling a #LinphoneChatMessage objects.
 * @ingroup chatroom
 */
typedef struct _LinphoneChatMessageCbs LinphoneChatMessageCbs;

/**
 * TODO
 * A chat room is the place where text messages are exchanged.
 * Can be created by linphone_core_create_chat_room().
 * @ingroup chatroom
 */
typedef struct _LinphoneChatRoom LinphoneChatRoom;

/**
 * TODO
 * An object to handle a chat room parameters.
 * Can be created with linphone_core_get_default_chat_room_params() or linphone_chat_room_params_new().
 * @ingroup chatroom
 */
typedef struct _LinphoneChatRoomParams LinphoneChatRoomParams;

/**
 * A mask of #LinphoneChatRoomCapabilities
 * @ingroup chatroom
 */
typedef int LinphoneChatRoomCapabilitiesMask;

/**
 * An object to handle the callbacks for the handling a #LinphoneChatRoom objects.
 * @ingroup chatroom
 */
typedef struct _LinphoneChatRoomCbs LinphoneChatRoomCbs;

/**
 * TODO
 * Object holding chat message data received by a push notification
 * @ingroup chatroom
**/
typedef struct _LinphonePushNotificationMessage LinphonePushNotificationMessage;

// -----------------------------------------------------------------------------
// EventLog.
// -----------------------------------------------------------------------------

/**
 * Object that represents an event that must be stored in database.
 * 
 * For example, all chat related events are wrapped in an #LinphoneEventLog, 
 * and many callbacks use this kind of type as parameter.
 * 
 * Use linphone_event_log_get_type() to get the #LinphoneEventLogType it refers to, 
 * and then you can use one of the accessor methods to get the underlying object, 
 * for example linphone_event_log_get_chat_message() for a #LinphoneChatMessage.
 * @ingroup events
 */
typedef struct _LinphoneEventLog LinphoneEventLog;

// -----------------------------------------------------------------------------
// Misc.
// -----------------------------------------------------------------------------

/**
 * The LinphoneContent object holds data that can be embedded in a signaling message.
 * @ingroup misc
 */
typedef struct _LinphoneContent LinphoneContent;

/**
 * Represents a dial plan
 * @ingroup misc
 */
typedef struct _LinphoneDialPlan LinphoneDialPlan;

// -----------------------------------------------------------------------------
// Search.
// -----------------------------------------------------------------------------

/**
 * A #LinphoneMagicSearch is used to do specifics searchs
 * @ingroup misc
 */
typedef struct _LinphoneMagicSearch LinphoneMagicSearch;

/**
 * The LinphoneSearchResult object represents a result of a search
 * @ingroup misc
 */
typedef struct _LinphoneSearchResult LinphoneSearchResult;

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_TYPES_H_
