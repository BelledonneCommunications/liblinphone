/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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
#include "linphone/enums/participant-device-enums.h"
#include "linphone/enums/participant-enums.h"
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
 * @brief Object holding authentication information.
 *
 * In most case, authentication information consists of a username and password.
 * If realm isn't set, it will be deduced automatically from the first authentication challenge as for the hash
 *algorithm. Sometimes, a userid is required by the proxy and then domain can be useful to discriminate different
 *credentials. You can also use this object if you need to use a client certificate.
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
// Account.
// -----------------------------------------------------------------------------

/**
 * Object that represents a Linphone Account.
 * This object replaces the deprecated #LinphoneProxyConfig.
 * Use a #LinphoneAccountParams object to configure it.
 * @ingroup account
 */
typedef struct _LinphoneAccount LinphoneAccount;

/**
 * Object that is used to set the different parameters of a #LinphoneAccount.
 *
 * Note that authenticated accounts should have a corresponding #LinphoneAuthInfo added to the #LinphoneCore to register
 * properly.
 * @ingroup account
 */
typedef struct _LinphoneAccountParams LinphoneAccountParams;

/**
 * An object to handle the callbacks for the handling of #LinphoneAccount objects.
 * @ingroup account
 */
typedef struct _LinphoneAccountCbs LinphoneAccountCbs;

/**
 * Creates and manage SIP user accounts remotely, using the REST API of the Flexisip Account Manager.
 * @see https://gitlab.linphone.org/BC/public/flexisip-account-manager
 * @ingroup account_creator
 */
typedef struct _LinphoneAccountManagerServices LinphoneAccountManagerServices;

/**
 * Request object created by #LinphoneAccountManagerServices.
 * @ingroup account_creator
 */
typedef struct _LinphoneAccountManagerServicesRequest LinphoneAccountManagerServicesRequest;

/**
 * An object to handle the callbacks for #LinphoneAccountManagerServicesRequest object.
 * @ingroup account_creator
 */
typedef struct _LinphoneAccountManagerServicesRequestCbs LinphoneAccountManagerServicesRequestCbs;

/**
 * Object that represents a device that at least once was connected to a given account.
 * @ingroup account
 */
typedef struct _LinphoneAccountDevice LinphoneAccountDevice;

// -----------------------------------------------------------------------------
// Address.
// -----------------------------------------------------------------------------

/**
 * @brief Object that represents a parsed SIP address.
 *
 * A SIP address is made of display name, username, domain name, port, and various
 * uri headers (such as tags). It looks like 'Alice <sip:alice@example.net>'.
 *
 * You can create an address using linphone_factory_create_address() or linphone_core_interpret_url_2()
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
 * @brief A conference is the object that allow to make calls when there are 2 or more participants
 *
 * To create (or find) a #LinphoneConference, you first need a #LinphoneConferenceParams object.
 * linphone_core_create_conference_with_params() allows you to create a conference.
 * A conference is uniquely identified by a conference address, meaning you can
 * have more than one conference between two accounts. To find a conference among those a core is part of, you can
 * call linphone_core_search_conference().
 *
 * A #LinphoneConference may be created automatically and implicitely when an outgoing call is made
 * to a conference server. Thanks to the standard 'isfocus' contact parameter, the call is identified as
 * belonging to a conference. The conference object can then be retrieved with linphone_call_get_conference().
 * @ingroup conference
 */
typedef struct _LinphoneConference LinphoneConference;

/**
 * @brief Object defining parameters for a #LinphoneConference.
 *
 * Can be created by calling function linphone_core_create_conference_params_2().
 *
 * @ingroup conference
 */
typedef struct _LinphoneConferenceParams LinphoneConferenceParams;

/**
 * @brief An object to handle the callbacks for the handling a #LinphoneConference objects.
 *
 * Use linphone_factory_create_conference_cbs() to create an instance.
 * Then pass the object to a #LinphoneConference instance through linphone_conference_add_callbacks().
 * @ingroup conference
 */
typedef struct _LinphoneConferenceCbs LinphoneConferenceCbs;

/**
 * @brief Object defining all information related to a past or future conference.
 *
 * @ingroup conference
 */
typedef struct _LinphoneConferenceInfo LinphoneConferenceInfo;

/**
 * @brief Creates and manages conferences on a conferenceing service, and send conference invitations to notify
 * participants
 *
 * @ingroup conference
 */
typedef struct _LinphoneConferenceScheduler LinphoneConferenceScheduler;

/**
 * Callbacks of #LinphoneConferenceScheduler object.
 * @ingroup account
 */
typedef struct _LinphoneConferenceSchedulerCbs LinphoneConferenceSchedulerCbs;

// -----------------------------------------------------------------------------
// Participants.
// -----------------------------------------------------------------------------

/**
 * @brief Identifies a member of a #LinphoneConference or #LinphoneChatRoom.
 *
 * A participant is identified by it's SIP address.
 * It can have many #LinphoneParticipantDevice.
 * @ingroup conference
 */
typedef struct _LinphoneParticipant LinphoneParticipant;

/**
 * @brief Object defining all information related to a participant
 *
 * @ingroup conference
 */
typedef struct _LinphoneParticipantInfo LinphoneParticipantInfo;

/**
 * @brief This object represents a unique device for a member of a #LinphoneConference or #LinphoneChatRoom.
 *
 * Devices are identified by the gruu parameter inside the #LinphoneAddress which can be obtained by
 * linphone_participant_device_get_address(). It is specially usefull to know the security level of each device inside
 * an end-to-end encrypted #LinphoneChatRoom.
 *
 * You can get a list of all #LinphoneParticipantDevice using linphone_participant_get_devices().
 * @ingroup conference
 */
typedef struct _LinphoneParticipantDevice LinphoneParticipantDevice;

/**
 * @brief An object to handle the callbacks for the handling a #LinphoneParticipantDevice objects.
 *
 * Use linphone_factory_create_participant_device_cbs() to create an instance.
 * Then pass the object to a #LinphoneParticipantDevice instance through linphone_participant_device_add_callbacks().
 * @ingroup conference
 */
typedef struct _LinphoneParticipantDeviceCbs LinphoneParticipantDeviceCbs;

/**
 * @brief This object represents the delivery/display state of a given chat message for a given participant.
 *
 * It also contains a timestamp at which this participant state has changed.
 *
 * Use linphone_chat_message_get_participants_by_imdn_state() to get all #LinphoneParticipantImdnState for a given
 * state. From there use linphone_participant_imdn_state_get_participant() to get the #LinphoneParticipant object if you
 * need it.
 * @ingroup conference
 */
typedef struct _LinphoneParticipantImdnState LinphoneParticipantImdnState;

/**
 * @brief This object is only used on server side for #LinphoneChatRoom with #LinphoneChatRoomBackendFlexisipChat
 * backend.
 * @ingroup conference
 */
typedef struct _LinphoneParticipantDeviceIdentity LinphoneParticipantDeviceIdentity;

// -----------------------------------------------------------------------------
// Call.
// -----------------------------------------------------------------------------

/**
 * @brief This object represents a call issued or received by the #LinphoneCore.
 *
 * Linphone only allows at most one active call at any given time and it will be
 * in #LinphoneCallStateStreamsRunning. However, if the core is locally hosting a #LinphoneConference,
 * you may have some or all the calls in the conference in #LinphoneCallStateStreamsRunning
 * as well as an additional active call outside of the conference in #LinphoneCallStateStreamsRunning
 * if the local participant of the #LinphoneConference is not part of it.
 *
 * You can get the #LinphoneCallState of the call using linphone_call_get_state(),
 * it's current #LinphoneCallParams with linphone_call_get_current_params() and
 * the latest statistics by calling linphone_call_get_audio_stats() or linphone_call_get_video_stats().
 *
 * The application can receive the various kind of events occuring in a call through the
 * #LinphoneCallCbs interface, see also linphone_call_add_callbacks().
 *
 * @ingroup call_control
 */
typedef struct _LinphoneCall LinphoneCall;

/** Callback prototype */
typedef void (*LinphoneCallCbFunc)(LinphoneCall *call, void *ud);

/**
 * @brief That class holds all the callbacks which are called by #LinphoneCall objects.
 *
 * Use linphone_factory_create_call_cbs() to create an instance. Then, call the
 * callback setters on the events you need to monitor and pass the object to
 * a #LinphoneCall instance through linphone_call_add_callbacks().
 * @ingroup call_control
 */
typedef struct _LinphoneCallCbs LinphoneCallCbs;

/**
 * @brief An object containing various parameters of a #LinphoneCall.
 *
 * You can specify your params while answering an incoming call using linphone_call_accept_with_params()
 * or while initiating an outgoing call with linphone_core_invite_address_with_params().
 *
 * This object can be created using linphone_core_create_call_params(), using NULL for the call pointer if you plan to
 *use it for an outgoing call.
 *
 * For each call, three #LinphoneCallParams are available: yours, your correspondent's
 * and the one that describe the current state of the call that is the result of the negociation between the previous
 *two. For example, you might enable a certain feature in your call param but this feature can be denied in the remote's
 *configuration, hence the difference.
 *
 * @see linphone_call_get_current_params(), linphone_call_get_remote_params() and linphone_call_get_params().
 * @ingroup call_control
 **/
typedef struct _LinphoneCallParams LinphoneCallParams;

/**
 * @brief Interface used to record audio and video into files.
 * @see linphone_core_create_recorder()
 * @ingroup call_control
 **/
typedef struct _LinphoneRecorder LinphoneRecorder;

/**
 * @brief Object containing various parameters of a #LinphoneRecorder.
 * @see linphone_core_create_recorder()
 * @ingroup call_control
 **/
typedef struct _LinphoneRecorderParams LinphoneRecorderParams;

/**
 * @brief Object used to keep track of all calls initiated, received or missed.
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

/**
 * @brief This object carry various statistic informations regarding the quality of an audio or video stream for a given
 *#LinphoneCall.
 *
 * To receive these informations periodically and as soon as they are computed,
 * implement the call_stats_updated() callback inside a #LinphoneCoreCbs.
 *
 * At any time, the application can access latest computed statistics using linphone_call_get_audio_stats() and
 *linphone_call_get_video_stats().
 * @ingroup call_misc
 **/
typedef struct _LinphoneCallStats LinphoneCallStats;

/**
 * @brief Object representing an RTP payload type.
 * @ingroup media_parameters
 */
typedef struct _LinphonePayloadType LinphonePayloadType;

/**
 * Object that is used to describe a video source.
 *
 * @ingroup call_control
 */
typedef struct _LinphoneVideoSourceDescriptor LinphoneVideoSourceDescriptor;

// -----------------------------------------------------------------------------
// Audio.
// -----------------------------------------------------------------------------

/**
 * @brief Object holding audio device information.
 *
 * It contains the name of the device, it's type if available (Earpiece, Speaker, Bluetooth, etc..)
 * and capabilities (input, output or both) the name of the driver that created it (filter in mediastreamer).
 *
 * You can use the #LinphoneAudioDevice objects to configure default input/output devices or do it dynamically during a
 *call.
 *
 * To get the list of available devices, use linphone_core_get_audio_devices(). This list will be limited to one device
 *of each type. Use linphone_core_get_extended_audio_devices() for a complete list.
 *
 * @ingroup audio
 **/
typedef struct _LinphoneAudioDevice LinphoneAudioDevice;

// -----------------------------------------------------------------------------
// ChatRoom.
// -----------------------------------------------------------------------------

/**
 * @brief A #LinphoneChatMessage represents an instant message that can be send or received through a #LinphoneChatRoom.
 *
 * To create a #LinphoneChatMessage, use linphone_chat_room_create_empty_message(),
 * then either add text using linphone_chat_message_add_utf8_text_content() or a
 * #LinphoneContent with file informations using linphone_chat_message_add_file_content().
 * A valid #LinphoneContent for file transfer must contain a type and subtype, the name of the file and it's size.
 * Finally call linphone_chat_message_send() to send it.
 *
 * To send files through a #LinphoneChatMessage, you need to have configured a file transfer server URL with
 * linphone_core_set_file_transfer_server(). On the receiving side, either use linphone_chat_message_download_content()
 * to download received files or enable auto-download in the #LinphoneCore using
 * linphone_core_set_max_size_for_auto_download_incoming_files(), -1 disabling the feature and 0 always downloading
 * files no matter it's size.
 *
 * Keep in mind a #LinphoneChatMessage created by a #LinphoneChatRoomBackendBasic #LinphoneChatRoom can only contain one
 * #LinphoneContent, either text or file.
 * @ingroup chatroom
 */
typedef struct _LinphoneChatMessage LinphoneChatMessage;

/**
 * @brief A chat message reaction is an emoji sent by someone in the same chat room to react to a specific
 * #LinphoneChatMessage.
 *
 * To create a #LinphoneChatMessageReaction, use linphone_chat_message_create_reaction().
 * Once you are ready, send the reaction using linphone_chat_message_reaction_send().
 *
 * Reactions are available using linphone_chat_message_get_reactions() and will be notified using dedicated callbacks
 * either in #LinphoneCoreCbs or #LinphoneChatMessageCbs.
 * @ingroup chatroom
 */
typedef struct _LinphoneChatMessageReaction LinphoneChatMessageReaction;

/**
 * @brief An object to handle the callbacks for the handling a #LinphoneChatMessage objects.
 * @ingroup chatroom
 */
typedef struct _LinphoneChatMessageCbs LinphoneChatMessageCbs;

/**
 * @brief A chat room is the place where #LinphoneChatMessage are exchanged.
 *
 * To create (or find) a #LinphoneChatRoom, you first need a #LinphoneChatRoomParams object.
 * A chat room is uniquely identified by it's local and remote SIP addresses, meaning you can
 * only have one chat room between two accounts (unless the backend is #LinphoneChatRoomBackendFlexisipChat).
 * Then you can call linphone_core_search_chat_room() or linphone_core_create_chat_room_6().
 *
 * Be careful as a #LinphoneChatRoomBackendFlexisipChat backend #LinphoneChatRoom will be created asynchronously, so
 * make sure you add a #LinphoneChatRoomCbs to the returned object to be notified
 * when it will be in state #LinphoneChatRoomStateCreated.
 *
 * All chat rooms are loaded from database when the #LinphoneCore starts, and you can get them using
 * linphone_core_get_chat_rooms(). This method doesn't return empty chat rooms nor ones for which the local address
 * doesn't match an existing #LinphoneAccount identity, unless you specify otherwise in the [misc] section
 * of your configuration file by setting hide_empty_chat_rooms=0 and/or hide_chat_rooms_from_removed_proxies=0.
 * @ingroup chatroom
 */
typedef struct _LinphoneChatRoom LinphoneChatRoom;

/**
 * @brief An object to handle the callbacks for the handling a #LinphoneChatRoom objects.
 *
 * Use linphone_factory_create_chat_room_cbs() to create an instance.
 * Then pass the object to a #LinphoneChatRoom instance through linphone_chat_room_add_callbacks().
 * @ingroup chatroom
 */
typedef struct _LinphoneChatRoomCbs LinphoneChatRoomCbs;

/**
 * @brief Object defining settings strictly associated with #LinphoneChatRoom objects such as ephemeral settings and
 * backends.
 *
 * It is automatically created when you enable chat capabilities in the #LinphoneConferenceParams.
 *
 * If the #LinphoneChatRoom backend is #LinphoneChatRoomBackendBasic, then no other parameter is required,
 * but #LinphoneChatMessage sent and received won't benefit from all features a #LinphoneChatRoomBackendFlexisipChat can
 * offer like conversation with multiple participants and a subject, end-to-end encryption, ephemeral messages, etc...
 * but this type is the only one that can interoperate with other SIP clients or with non-flexisip SIP proxies.
 * @ingroup chatroom
 */
typedef struct _LinphoneChatParams LinphoneChatParams;

/**
 * @brief Object defining parameters for a #LinphoneChatRoom.
 *
 * Can be created with linphone_core_create_default_chat_room_params().
 * You can use linphone_chat_room_params_is_valid() to check if your configuration is valid or not.
 *
 * If the #LinphoneChatRoom backend is #LinphoneChatRoomBackendBasic, then no other parameter is required,
 * but #LinphoneChatMessage sent and received won't benefit from all features a #LinphoneChatRoomBackendFlexisipChat can
 * offer like conversation with multiple participants and a subject, end-to-end encryption, ephemeral messages, etc...
 * but this type is the only one that can interoperate with other SIP clients or with non-flexisip SIP proxies.
 * @ingroup chatroom
 */
typedef struct _LinphoneConferenceParams LinphoneChatRoomParams;

/**
 * @brief A mask of #LinphoneChatRoomCapabilities
 * @ingroup chatroom
 */
typedef int LinphoneChatRoomCapabilitiesMask;

/**
 * @brief A mask of #LinphoneChatRoomHistoryFilter
 * @ingroup chatroom
 */
typedef unsigned int LinphoneChatRoomHistoryFilterMask;

/**
 * @brief Enum for search direction.
 * @ingroup chatroom
 **/
typedef enum _LinphoneSearchDirection {
	LinphoneSearchDirectionUp = 0,
	LinphoneSearchDirectionDown = 1,
} LinphoneSearchDirection;

// -----------------------------------------------------------------------------
// Ekt.
// -----------------------------------------------------------------------------
/**
 * @brief Object representing all informations present in an Encrypted Key Transport event.
 * @ingroup ekt_api
 */
typedef struct _LinphoneEktInfo LinphoneEktInfo;

// -----------------------------------------------------------------------------
// Event.
// -----------------------------------------------------------------------------

/**
 * @brief Object representing an event state, which is subcribed or published.
 *
 * @see linphone_core_publish()
 * @see linphone_core_subscribe()
 * @ingroup events
 **/
typedef struct _LinphoneEvent LinphoneEvent;

/**
 * @brief An object to handle the callbacks for handling the LinphoneEvent operations.
 * @ingroup events
 **/
typedef struct _LinphoneEventCbs LinphoneEventCbs;

/**
 * @brief Object that represents an event that must be stored in database.
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
// LDAP.
// -----------------------------------------------------------------------------

/**
 * Object that represents a LDAP connection.
 * Use a #LinphoneLdapParams object to configure it.
 * @ingroup ldap
 * @deprecated 18/11/2024 #LinphoneLdap object is no longer used, use #LinphoneRemoteContactDirectory instead.
 */
typedef struct _LinphoneLdap LinphoneLdap;

/**
 * Object that is used to set the different parameters of a #LinphoneLdap.
 * @ingroup ldap
 */
typedef struct _LinphoneLdapParams LinphoneLdapParams;

/**
 * @brief Enum Debug verbosity for OpenLdap
 * @ingroup ldap
 **/
typedef enum _LinphoneLdapDebugLevel {
	LinphoneLdapDebugLevelOff = 0,     /**< Set OpenLdap verbosity to none */
	LinphoneLdapDebugLevelVerbose = 1, /**< Set OpenLdap verbosity to debug level */
} LinphoneLdapDebugLevel;

/**
 * @brief Enum describing how the authentification will be made.
 * @ingroup ldap
 **/
typedef enum _LinphoneLdapAuthMethod {
	LinphoneLdapAuthMethodAnonymous = 0, /**< Connection without passwords */
	LinphoneLdapAuthMethodSimple = 1,    /**< Connection with username/password */
} LinphoneLdapAuthMethod;

/**
 * @brief Enum describing server certificates verification modes.
 * @ingroup ldap
 **/
typedef enum _LinphoneLdapCertVerificationMode {
	LinphoneLdapCertVerificationDefault = -1, /**< Use default value defined on core */
	LinphoneLdapCertVerificationDisabled = 0, /**< Verification is disabled*/
	LinphoneLdapCertVerificationEnabled = 1   /**< Verification is enabled*/
} LinphoneLdapCertVerificationMode;

/**
 * @brief Enum describing errors in LDAP parameters.
 * @ingroup ldap
 **/
typedef enum _LinphoneLdapCheck {
	LinphoneLdapCheckOk = 0, /**< No error */

	LinphoneLdapCheckServerEmpty = 1,    /**< Server field is empty */
	LinphoneLdapCheckServerNotUrl = 2,   /**< The server is not an url*/
	LinphoneLdapCheckServerNoScheme = 4, /**< The server doesn't contain a scheme*/
	LinphoneLdapCheckServerNotLdap = 8,  /**< The server is not a LDAP scheme */
	LinphoneLdapCheckServerLdaps =
	    16, /**< LDAP over SSL is non-standardized and deprecated: ldaps has been specified */

	LinphoneLdapCheckBaseObjectEmpty = 32, /**< Base Object has been specified */

	LinphoneLdapCheckMissingFields = 64, /**< Some required fields are missing*/

} LinphoneLdapCheck;

// -----------------------------------------------------------------------------
// Misc.
// -----------------------------------------------------------------------------

/**
 * @brief The object representing a data buffer.
 * @ingroup misc
 */
typedef struct _LinphoneBuffer LinphoneBuffer;

/**
 * @brief This object holds data that can be embedded in a signaling message or IM message.
 *
 * Use linphone_core_create_content() to create it, and then you should set at least it's
 * type and subtype and fill the buffer with your data.
 *
 * A #LinphoneContent can be multipart (contain other contents), have file information (name, path, size),
 * be encrypted, have custom headers, etc...
 *
 * @ingroup misc
 */
typedef struct _LinphoneContent LinphoneContent;

/**
 * @brief Represents a dial plan
 * @ingroup misc
 */
typedef struct _LinphoneDialPlan LinphoneDialPlan;

/**
 * @brief Object holding chat message data received by a push notification on iOS platform only.
 *
 * This object is a subset of #LinphoneChatMessage, so only a few methods of it's parent are available,
 * like linphone_push_notification_message_get_text_content() and linphone_push_notification_message_get_subject(),
 * just enough to be able to build a notification to show the user.
 * @ingroup chatroom
 **/
typedef struct _LinphonePushNotificationMessage LinphonePushNotificationMessage;

/**
 * @brief Object holding push notification configuration that will be set in the contact URI parameters of the Contact
 *header in the REGISTER, if the #LinphoneAccountParams is configured to allow push notifications, see
 *linphone_account_params_set_push_notification_allowed().
 *
 * This object can be accessed through the #LinphoneAccountParams object, which can be obtained from your
 *#LinphoneAccount object.
 * @ingroup account
 **/
typedef struct _LinphonePushNotificationConfig LinphonePushNotificationConfig;

// -----------------------------------------------------------------------------
// Search.
// -----------------------------------------------------------------------------

/**
 * A #LinphoneMagicSearch is used to search for contacts from various sources:
 * - #LinphoneFriendList
 * - Ldap connection (see #LinphoneLdap)
 * - Remote CardDAV server (see #LinphoneCardDavParams)
 * - Call logs, conferences and existing chat rooms.
 * @see linphone_magic_search_get_contacts_list_async()
 * @ingroup contacts
 */
typedef struct _LinphoneMagicSearch LinphoneMagicSearch;

/**
 * #LinphoneMagicSearchCbs is an interface to be notified of results
 * of contact searches initiated from the #LinphoneMagicSearch.
 * @see linphone_magic_search_add_callbacks()
 * @ingroup contacts
 */
typedef struct _LinphoneMagicSearchCbs LinphoneMagicSearchCbs;

/**
 * The #LinphoneSearchResult object represents a result of a search initiated from a #LinphoneMagicSearch.
 * @ingroup contacts
 */
typedef struct _LinphoneSearchResult LinphoneSearchResult;

/**
 * The #LinphoneCardDavParams object represents a remote CardDAV server used by #LinphoneMagicSearch as a plugin source.
 * @ingroup contacts
 */
typedef struct _LinphoneCardDavParams LinphoneCardDavParams;

/**
 * Object that represents a remote contact directory such as a LDAP or CardDAV server; used as a #LinphoneMagicSearch
 * source.
 * @ingroup contacts
 */
typedef struct _LinphoneRemoteContactDirectory LinphoneRemoteContactDirectory;

// -----------------------------------------------------------------------------
// Digest authentication policy.
// -----------------------------------------------------------------------------

/**
 * @brief The LinphoneDigestAuthenticationPolicy holds parameters relative to digest authentication procedures.
 * @ingroup misc
 */
typedef struct _LinphoneDigestAuthenticationPolicy LinphoneDigestAuthenticationPolicy;

// -----------------------------------------------------------------------------
// Friend.
// -----------------------------------------------------------------------------

/**
 * Object that represents a #LinphoneFriend's phone number.
 * @ingroup contacts
 */
typedef struct _LinphoneFriendPhoneNumber LinphoneFriendPhoneNumber;

/**
 * Object that represents a #LinphoneFriend's device (name, trust level) for a given SIP address.
 * @ingroup contacts
 */
typedef struct _LinphoneFriendDevice LinphoneFriendDevice;

// -----------------------------------------------------------------------------
// Dictionary
// -----------------------------------------------------------------------------

/**
 *  Object that represents key-value pair container.
 * @ingroup dictionary
 */
typedef struct _LinphoneDictionary LinphoneDictionary;
// -----------------------------------------------------------------------------
// Alert
// -----------------------------------------------------------------------------

/**
 * @brief Object that represents an alert.
 * Alerts are raised at run-time when particular conditions are met, for example bad network quality.
 * The full list of available alert types is described by the #LinphoneAlertType enum.
 * An application is notified of new alerts through the #LinphoneCoreCbs interface.
 * Once raised, the application may use the #LinphoneAlertCbs interface to get notified
 * when the alert stops.
 * For each kind of alert, a #LinphoneDictionary is filled with relevant informations, returned by
 * linphone_alert_get_informations(). The keys available are documented per-type in #LinphoneAlertType enum.
 * @ingroup alert
 */
typedef struct _LinphoneAlert LinphoneAlert;
/**
 * @brief Object that represents a callback attached to an alert
 * @ingroup alert
 */
typedef struct _LinphoneAlertCbs LinphoneAlertCbs;

// -----------------------------------------------------------------------------
// SignalInformation
// -----------------------------------------------------------------------------
/**
 * @brief Object to get signal (wifi/4G etc...) informations.
 * @ingroup alert
 */
typedef struct _LinphoneSignalInformation LinphoneSignalInformation;

/**
 * @brief Object that represents a bearer token (eg OAUTH).
 * SIP servers may support "bearer" kind of authentication, in which case an authentication token
 * needs to be supplied in order to authenticate to the SIP service.
 * Applications are responsible to obtain the token from an authentication server.
 * In order to pass it to liblinphone for usage, the token needs to be
 * encapsulated into a #LinphoneBearerToken, together with its expiration time
 * and target server name for which it is intended to use, then passed into a #LinphoneAuthInfo object.
 * Both access and refresh tokens may be represented.
 * If both are provided to the #LinphoneAuthInfo, then liblinphone automatically uses
 * the refresh token to obtain a new access token when the latter is expired.
 * @ingroup authentication
 */
typedef struct _LinphoneBearerToken LinphoneBearerToken;

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_TYPES_H_
