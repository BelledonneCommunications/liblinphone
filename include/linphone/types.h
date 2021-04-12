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

#ifndef LINPHONE_TYPES_H_
#define LINPHONE_TYPES_H_


#include "ortp/payloadtype.h"
#include "mediastreamer2/msinterfaces.h"
#include "mediastreamer2/msvideo.h"
#include "linphone/defs.h"

// For migration purpose.
#include "linphone/api/c-types.h"

// -----------------------------------------------------------------------------
// Account creator.
// -----------------------------------------------------------------------------

/**
 * @brief Represents an account configuration to be used by #LinphoneCore.
 * 
 * In addition to the #LinphoneAuthInfo that stores the credentials, 
 * you need to configure a #LinphoneProxyConfig as well to be able to connect to a proxy server.
 * 
 * A minimal proxy config consists of an identity address (sip:username@domain.tld) 
 * and the proxy server address, @see linphone_proxy_config_set_server_addr().
 * 
 * If any, it will be stored inside the default configuration file, so it will survive the destruction
 * of the #LinphoneCore and be available at the next start.
 *
 * The account set with linphone_core_set_default_proxy_config() will be used as default 
 * for outgoing calls & chat messages unless specified otherwise.
 * @ingroup proxies
**/
typedef struct _LinphoneProxyConfig LinphoneProxyConfig;

/**
 * @brief The object used to configure an account on a server via XML-RPC, 
 * see @link https://wiki.linphone.org/xwiki/wiki/public/view/Lib/Features/Override%20account%20creator%20request/
 * @ingroup account_creator
**/
typedef struct _LinphoneAccountCreator LinphoneAccountCreator;

/**
 * @brief An object to define a LinphoneAccountCreator service.
 * @ingroup account_creator
 * @donotwrap
**/
typedef struct _LinphoneAccountCreatorService LinphoneAccountCreatorService;

/**
 * @brief An object to handle the responses callbacks for handling the #LinphoneAccountCreator operations.
 * @ingroup account_creator
**/
typedef struct _LinphoneAccountCreatorCbs LinphoneAccountCreatorCbs;

/**
 * @brief Enum describing phone number checking, used by the #LinphoneAccountCreator.
 * @ingroup account_creator
**/
typedef enum _LinphoneAccountCreatorPhoneNumberStatus {
	LinphoneAccountCreatorPhoneNumberStatusOk = 0x1, /**< Phone number ok */
	LinphoneAccountCreatorPhoneNumberStatusTooShort = 0x2, /**< Phone number too short */
	LinphoneAccountCreatorPhoneNumberStatusTooLong = 0x4, /**< Phone number too long */
	LinphoneAccountCreatorPhoneNumberStatusInvalidCountryCode = 0x8, /**< Country code invalid */
	LinphoneAccountCreatorPhoneNumberStatusInvalid = 0x10 /**< Phone number invalid */
} LinphoneAccountCreatorPhoneNumberStatus;

/**
 * @brief A mask of #LinphoneAccountCreatorPhoneNumberStatus values.
 * @ingroup account_creator
 */
typedef unsigned int LinphoneAccountCreatorPhoneNumberStatusMask;

/**
 * @brief Enum describing username checking, used by the #LinphoneAccountCreator.
 * @ingroup account_creator
**/
typedef enum _LinphoneAccountCreatorUsernameStatus {
	LinphoneAccountCreatorUsernameStatusOk, /**< Username ok */
	LinphoneAccountCreatorUsernameStatusTooShort, /**< Username too short */
	LinphoneAccountCreatorUsernameStatusTooLong,  /**< Username too long */
	LinphoneAccountCreatorUsernameStatusInvalidCharacters, /**< Contain invalid characters */
	LinphoneAccountCreatorUsernameStatusInvalid /**< Invalid username */
} LinphoneAccountCreatorUsernameStatus;

/**
 * @brief Enum describing email checking, used by the #LinphoneAccountCreator.
 * @ingroup account_creator
**/
typedef enum _LinphoneAccountCreatorEmailStatus {
	LinphoneAccountCreatorEmailStatusOk, /**< Email ok */
	LinphoneAccountCreatorEmailStatusMalformed, /**< Email malformed */
	LinphoneAccountCreatorEmailStatusInvalidCharacters /**< Contain invalid characters */
} LinphoneAccountCreatorEmailStatus;

/**
 * @brief Enum describing password checking, used by the #LinphoneAccountCreator.
 * @ingroup account_creator
**/
typedef enum _LinphoneAccountCreatorPasswordStatus {
	LinphoneAccountCreatorPasswordStatusOk, /**< Password ok */
	LinphoneAccountCreatorPasswordStatusTooShort, /**< Password too short */
	LinphoneAccountCreatorPasswordStatusTooLong,  /**< Password too long */
	LinphoneAccountCreatorPasswordStatusInvalidCharacters, /**< Contain invalid characters */
	LinphoneAccountCreatorPasswordStatusMissingCharacters /**< Missing specific characters */
} LinphoneAccountCreatorPasswordStatus;

/**
 * @brief Enum describing language checking, used by the #LinphoneAccountCreator.
 * @ingroup account_creator
**/
typedef enum _LinphoneAccountCreatorLanguageStatus {
	LinphoneAccountCreatorLanguageStatusOk /**< Language ok */
} LinphoneAccountCreatorLanguageStatus;

/**
 * @brief Enum describing algorithm checking, used by the #LinphoneAccountCreator.
 * @ingroup account_creator
 **/
typedef enum _LinphoneAccountCreatorAlgoStatus {
	LinphoneAccountCreatorAlgoStatusOk, /**< Algorithm ok */
	LinphoneAccountCreatorAlgoStatusNotSupported /**< Algorithm not supported */
} LinphoneAccountCreatorAlgoStatus;

/**
 * @brief Enum describing activation code checking, used by the #LinphoneAccountCreator.
 * @ingroup account_creator
**/
typedef enum _LinphoneAccountCreatorActivationCodeStatus {
	LinphoneAccountCreatorActivationCodeStatusOk, /**< Activation code ok */
	LinphoneAccountCreatorActivationCodeStatusTooShort, /**< Activation code too short */
	LinphoneAccountCreatorActivationCodeStatusTooLong, /**< Activation code too long */
	LinphoneAccountCreatorActivationCodeStatusInvalidCharacters /**< Contain invalid characters */
} LinphoneAccountCreatorActivationCodeStatus;

/**
 * @brief Enum describing domain checking, used by the #LinphoneAccountCreator.
 * @ingroup account_creator
**/
typedef enum _LinphoneAccountCreatorDomainStatus {
	LinphoneAccountCreatorDomainOk, /**< Domain ok */
	LinphoneAccountCreatorDomainInvalid /**< Domain invalid */
} LinphoneAccountCreatorDomainStatus;

/**
 * @brief Enum describing transport checking, used by the #LinphoneAccountCreator.
 * @ingroup account_creator
**/
typedef enum _LinphoneAccountCreatorTransportStatus {
	LinphoneAccountCreatorTransportOk, /**< Transport ok */
	LinphoneAccountCreatorTransportUnsupported /**< Transport invalid */
} LinphoneAccountCreatorTransportStatus;

/**
 * @brief Enum describing the status of server request, used by the #LinphoneAccountCreator.
 * @ingroup account_creator_request
**/
typedef enum _LinphoneAccountCreatorStatus {
	/** Request status **/
	LinphoneAccountCreatorStatusRequestOk, /**< Request passed */
	LinphoneAccountCreatorStatusRequestFailed, /**< Request failed */
	LinphoneAccountCreatorStatusMissingArguments, /**< Request failed due to missing argument(s) */
	LinphoneAccountCreatorStatusMissingCallbacks, /**< Request failed due to missing callback(s) */

	/** Account status **/
	/* Creation */
	LinphoneAccountCreatorStatusAccountCreated, /**< Account created */
	LinphoneAccountCreatorStatusAccountNotCreated, /**< Account not created */
	/* Existence */
	LinphoneAccountCreatorStatusAccountExist, /**< Account exist */
	LinphoneAccountCreatorStatusAccountExistWithAlias, /**< Account exist with alias */
	LinphoneAccountCreatorStatusAccountNotExist, /**< Account not exist */
	LinphoneAccountCreatorStatusAliasIsAccount, /**< Account was created with Alias */
	LinphoneAccountCreatorStatusAliasExist, /**< Alias exist */
	LinphoneAccountCreatorStatusAliasNotExist, /**< Alias not exist */
	/* Activation */
	LinphoneAccountCreatorStatusAccountActivated, /**< Account activated */
	LinphoneAccountCreatorStatusAccountAlreadyActivated, /**< Account already activated */
	LinphoneAccountCreatorStatusAccountNotActivated, /**< Account not activated */
	/* Linking */
	LinphoneAccountCreatorStatusAccountLinked, /**< Account linked */
	LinphoneAccountCreatorStatusAccountNotLinked, /**< Account not linked */

	/** Server **/
	LinphoneAccountCreatorStatusServerError, /**< Error server */

	LinphoneAccountCreatorStatusPhoneNumberInvalid, /**< Error cannot send SMS */
	LinphoneAccountCreatorStatusWrongActivationCode, /**< Error key doesn't match */
	LinphoneAccountCreatorStatusPhoneNumberOverused, /**< Error too many SMS sent */
	LinphoneAccountCreatorStatusAlgoNotSupported, /**< Error algo isn't MD5 or SHA-256 */
	LinphoneAccountCreatorStatusUnexpectedError, /**< Generic error */
} LinphoneAccountCreatorStatus;

// -----------------------------------------------------------------------------
// Call.
// -----------------------------------------------------------------------------

/**
 * @brief Enum representing the direction of a call.
 * @ingroup call_logs
**/
typedef enum _LinphoneCallDir {
	LinphoneCallOutgoing, /**< outgoing calls*/
	LinphoneCallIncoming  /**< incoming calls*/
} LinphoneCallDir;

/**
 * @brief This object carry various statistic informations regarding the quality of an audio or video stream for a given #LinphoneCall.
 *
 * To receive these informations periodically and as soon as they are computed, 
 * implement the call_stats_updated() callback inside a #LinphoneCoreCbs.
 *
 * At any time, the application can access latest computed statistics using linphone_call_get_audio_stats() and linphone_call_get_video_stats().
 * @ingroup call_misc
**/
typedef struct _LinphoneCallStats LinphoneCallStats;

/**
 * @brief Enum representing the status of a call.
 * @ingroup call_logs
**/
typedef enum _LinphoneCallStatus {
	LinphoneCallSuccess, /**< The call was sucessful */
	LinphoneCallAborted, /**< The call was aborted (caller hanged up) */
	LinphoneCallMissed, /**< The call was missed (incoming call timed out without being answered or hanged up) */
	LinphoneCallDeclined, /**< The call was declined, either locally or by remote end */
	LinphoneCallEarlyAborted, /**<The call was aborted before being advertised to the application - for protocol reasons*/
	LinphoneCallAcceptedElsewhere, /**<The call was answered on another device*/
	LinphoneCallDeclinedElsewhere /**<The call was declined on another device*/
} LinphoneCallStatus;

// -----------------------------------------------------------------------------
// Friends.
// -----------------------------------------------------------------------------

/**
 * @brief This object is used to store a SIP address.
 * 
 * #LinphoneFriend is mainly used to implement an adressbook feature, and are used as data for the #LinphoneMagicSearch object.
 * If your proxy supports it, you can also use it to subscribe to presence information.
 * 
 * The objects are stored in a #LinphoneFriendList which are in turn stored inside the #LinphoneCore.
 * They can be stored inside a database if the path to it is configured, otherwise they will be lost after the #LinphoneCore is destroyed.
 * 
 * Thanks to the vCard plugin, you can also store more information like phone numbers, organization, etc...
 * @ingroup buddy_list
 */
typedef struct _LinphoneFriend LinphoneFriend;

/**
* @brief Enum describing the capabilities of a #LinphoneFriend, populated through presence subscribe/notify process.
* @ingroup buddy_list
**/
typedef enum _LinphoneFriendCapability {
	LinphoneFriendCapabilityNone = 0, /**< No capabilities populated */
	LinphoneFriendCapabilityGroupChat = 1 << 0, /**< This friend can be invited in a Flexisip backend #LinphoneChatRoom */
	LinphoneFriendCapabilityLimeX3dh = 1 << 1, /**< This friend can be invited in a Flexisip backend end-to-end encrypted #LinphoneChatRoom */
	LinphoneFriendCapabilityEphemeralMessages = 1 << 2 /**< This friend is able to delete ephemeral messages once they have expired */
} LinphoneFriendCapability;

/**
 * @brief This object representing a list of #LinphoneFriend.
 * 
 * You can use it to store contacts locally or synchronize them through CardDAV protocol.
 * @ingroup buddy_list
**/
typedef struct _LinphoneFriendList LinphoneFriendList;

/**
 * @brief An object to handle the callbacks for #LinphoneFriend synchronization.
 * @ingroup buddy_list
**/
typedef struct _LinphoneFriendListCbs LinphoneFriendListCbs;

/**
* @brief Enum describing the status of a LinphoneFriendList operation.
* @ingroup buddy_list
**/
typedef enum _LinphoneFriendListStatus {
	LinphoneFriendListOK, /**< Operation went fine */
	LinphoneFriendListNonExistentFriend, /**< #LinphoneFriend wasn't found in the #LinphoneFriendList */
	LinphoneFriendListInvalidFriend /**< #LinphoneFriend is already present in a #LinphoneFriendList */
} LinphoneFriendListStatus;

/**
 * @brief Enum describing the status of a CardDAV synchronization.
 * @ingroup buddy_list
 */
typedef enum _LinphoneFriendListSyncStatus {
	LinphoneFriendListSyncStarted, /**< Synchronization started */
	LinphoneFriendListSyncSuccessful, /**< Synchronization finished successfuly */
	LinphoneFriendListSyncFailure /**< Synchronization failed */
} LinphoneFriendListSyncStatus;

// -----------------------------------------------------------------------------
// Core.
// -----------------------------------------------------------------------------

/**
 * @brief This object is used to manipulate a configuration file.
 *
 * The format of the configuration file is a .ini like format:
 * - sections are defined in []
 * - each section contains a sequence of key=value pairs
 * - each line starting by a # is a comment
 * 
 * Various types can be used: strings and lists of strings, integers, floats, booleans (written as 0 or 1) and range of integers.
 * 
 * Usually a #LinphoneCore is initialized using two #LinphoneConfig, one default (where configuration changes through API calls will be saved)
 * and one named 'factory' which is read-only and overwrites any setting that may exists in the default one.
 * 
 * It is also possible to use only one (either default or factory) or even none.
 * @ingroup initializing
**/
typedef struct _LpConfig LinphoneConfig;

/**
 * Define old struct name for backward compatibility
 */
#define LpConfig LinphoneConfig

/**
 * @brief Describes the state of the remote configuring process of the #LinphoneCore object, 'Skipped' when the feature is disabled.
 * 
 * It is notified via the configuring_status() callback in #LinphoneCoreCbs.
 * @ingroup initializing
**/
typedef enum _LinphoneConfiguringState {
	LinphoneConfiguringSuccessful,
	LinphoneConfiguringFailed,
	LinphoneConfiguringSkipped
} LinphoneConfiguringState;

/**
 * @brief Describes the global state of the #LinphoneCore object.
 * 
 * It is notified via the global_state_changed() callback in #LinphoneCoreCbs.
 * @ingroup initializing
**/
typedef enum _LinphoneGlobalState {
	/** State in which we're in after linphone_core_stop(). Do not call any method while in this state except for linphone_core_start() */
	LinphoneGlobalOff,
	/** Transient state for when we call linphone_core_start() */
	LinphoneGlobalStartup,
	/** Indicates #LinphoneCore has been started and is up and running */
	LinphoneGlobalOn,
	/** Transient state for when we call linphone_core_stop() */
	LinphoneGlobalShutdown,
	/** Transient state between Startup and On if there is a remote provisionning URI configured */
	LinphoneGlobalConfiguring,
	/** #LinphoneCore state after being created by linphone_factory_create_core(), generally followed by a call to linphone_core_start() */
	LinphoneGlobalReady
} LinphoneGlobalState;

/**
 * @brief Describes proxy registration states.
 * 
 * It is notified via the registration_state_changed() callback in #LinphoneCoreCbs.
 * @ingroup proxies
**/
typedef enum _LinphoneRegistrationState {
	LinphoneRegistrationNone, /**< Initial state for registrations */
	LinphoneRegistrationProgress, /**< Registration is in progress */
	LinphoneRegistrationOk,	/**< Registration is successful */
	LinphoneRegistrationCleared, /**< Unregistration succeeded */
	LinphoneRegistrationFailed	/**< Registration failed */
} LinphoneRegistrationState;

/**
 * @brief Main object to instanciate and on which to keep a reference.
 * 
 * This object is the first object to instanciante, and will allow you to perform all kind of tasks.
 * To create it, use either linphone_factory_create_core_3() or linphone_factory_create_core_with_config_3(),
 * see #LinphoneConfig for more information about factory and default config.
 * On some platforms like Android or iOS you will need to give it the Context of your application.
 * 
 * Once the #LinphoneCore is in state #LinphoneGlobalReady, use linphone_core_start(). 
 * It will then go to state #LinphoneGlobalOn and from that you can start using it for calls and chat messages.
 * It is recommended to add a #LinphoneCoreCbs listener using linphone_core_add_listener() to it to monitor different events.
 * 
 * To be able to receive events from the network, you must schedule a call linphone_core_iterate() often, like every 20ms.
 * On Android & iOS linphone_core_is_auto_iterate_enabled() is enabled by default so you don't have to worry about that unless you disable it
 * using linphone_core_set_auto_iterate_enabled() or by setting in the [misc] section of your configuration auto_iterate=0.
 * @warning Our API isn't thread-safe but also isn't blocking, so it is strongly recommend to always call our methods from the main thread.
 * 
 * Once you don't need it anymore, call linphone_core_stop() and release the reference on it so it can gracefully shutdown.
 * @ingroup initializing
 */
typedef struct _LinphoneCore LinphoneCore;

/**
 * @brief The factory is a singleton object devoted to the creation of all the objects
 * of Liblinphone that cannot be created by #LinphoneCore itself.
 * 
 * It is also used to configure a few behaviors before creating the #LinphoneCore, like the logs verbosity or collection.
 * @ingroup initializing
 */
typedef struct _LinphoneFactory LinphoneFactory;

/**
 * @brief That class holds all the callbacks which are called by #LinphoneCore.
 *
 * Once created, add your #LinphoneCoreCbs using linphone_core_add_listener().
 * Keep a reference on it as long as you need it. 
 * You can use linphone_core_remove_listener() to remove it but that isn't mandatory.
 * 
 * The same applies to all listeners in our API.
 * @ingroup initializing
 */
typedef struct _LinphoneCoreCbs LinphoneCoreCbs;

// -----------------------------------------------------------------------------
// Misc.
// -----------------------------------------------------------------------------

/**
 * @brief SIP transports & ports configuration object.
 * 
 * Indicates which transport among UDP, TCP, TLS and DTLS should be enabled and if so on which port to listen.
 * You can use special values like #LC_SIP_TRANSPORT_DISABLED (0), #LC_SIP_TRANSPORT_RANDOM (-1) and #LC_SIP_TRANSPORT_DONTBIND (-2).
 * 
 * Once configuration is complete, use linphone_core_set_transports() to apply it.
 * This will be saved in configuration file so you don't have to do it each time the #LinphoneCore starts.
 * @ingroup initializing
 */
typedef struct _LinphoneTransports LinphoneTransports;

/**
 * @brief Object describing policy regarding video streams establishments.
 * 
 * Use linphone_video_activation_policy_set_automatically_accept() and linphone_video_activation_policy_set_automatically_initiate()
 * to tell the Core to automatically accept or initiate video during calls.
 * 
 * Even if disabled, you'll still be able to add it later while the call is running.
 * @ingroup media_parameters
**/
typedef struct _LinphoneVideoActivationPolicy LinphoneVideoActivationPolicy;

/**
 * @brief This object represents a video definition, eg. it's width, it's height and possibly it's name.
 * 
 * It is mostly used to configure the default video size sent by your camera during a video call with
 * linphone_core_set_preferred_video_definition() method.
 * @ingroup media_parameters
 */
typedef struct _LinphoneVideoDefinition LinphoneVideoDefinition;

// -----------------------------------------------------------------------------
// Other.
// -----------------------------------------------------------------------------

/**
 * @brief Enum describing Ip family.
 * @ingroup initializing
**/
typedef enum _LinphoneAddressFamily {
	LinphoneAddressFamilyInet, /**< IpV4 */
	LinphoneAddressFamilyInet6, /**< IpV6 */
	LinphoneAddressFamilyUnspec, /**< Unknown */
} LinphoneAddressFamily;

/**
 * @brief Enum describing the authentication methods.
 * @ingroup network_parameters
**/
typedef enum _LinphoneAuthMethod {
	LinphoneAuthHttpDigest, /**< Digest authentication requested */
	LinphoneAuthTls, /**< Client certificate requested */
} LinphoneAuthMethod;

/**
 * @brief Enum describing RTP AVPF activation modes.
 * @ingroup media_parameters
**/
typedef enum _LinphoneAVPFMode {
	LinphoneAVPFDefault = -1, /**< Use default value defined at upper level */
	LinphoneAVPFDisabled, /**< AVPF is disabled */
	LinphoneAVPFEnabled /**< AVPF is enabled */
} LinphoneAVPFMode;

/**
 * @brief The object representing a data buffer.
 * @ingroup misc
**/
typedef struct _LinphoneBuffer LinphoneBuffer;

/**
 * Consolidated presence information: 'online' means the user is open for communication,
 * 'busy' means the user is open for communication but involved in an other activity,
 * 'do not disturb' means the user is not open for communication, and 'offline' means
 * that no presence information is available.
 * @ingroup buddy_list
 */
typedef enum _LinphoneConsolidatedPresence {
	LinphoneConsolidatedPresenceOnline,
	LinphoneConsolidatedPresenceBusy,
	LinphoneConsolidatedPresenceDoNotDisturb,
	LinphoneConsolidatedPresenceOffline
} LinphoneConsolidatedPresence;

typedef struct _LinphoneContactProvider LinphoneContactProvider;

typedef struct _LinphoneContactSearch LinphoneContactSearch;

typedef unsigned int LinphoneContactSearchID;

/**
 * Old name of #LinphoneContactSearchID
 * @deprecated 03/02/2017
 * @donotwrap
 */
LINPHONE_DEPRECATED typedef LinphoneContactSearchID ContactSearchID;

typedef struct belle_sip_dict LinphoneDictionary;

/**
 * @brief Enum describing the result of the echo canceller calibration process.
 * @ingroup media_parameters
**/
typedef enum _LinphoneEcCalibratorStatus {
	LinphoneEcCalibratorInProgress, /**< The echo canceller calibration process is on going */
	LinphoneEcCalibratorDone, /**< The echo canceller calibration has been performed and produced an echo delay measure */
	LinphoneEcCalibratorFailed, /**< The echo canceller calibration process has failed */
	LinphoneEcCalibratorDoneNoEcho /**< The echo canceller calibration has been performed and no echo has been detected */
} LinphoneEcCalibratorStatus;

/**
 * @brief Object representing full details about a signaling error or status.
 * 
 * All #LinphoneErrorInfo object returned by the liblinphone API are readonly and transcients. For safety they must be used immediately
 * after obtaining them. Any other function call to the liblinphone may change their content or invalidate the pointer.
 * @ingroup misc
**/
typedef struct _LinphoneErrorInfo LinphoneErrorInfo;

/**
 * @brief Object representing an event state, which is subcribed or published.
 * 
 * @see linphone_core_publish()
 * @see linphone_core_subscribe()
 * @ingroup event_api
**/
typedef struct _LinphoneEvent LinphoneEvent;

/**
 * @brief An object to handle the callbacks for handling the LinphoneEvent operations.
 * @ingroup event_api
**/
typedef struct _LinphoneEventCbs LinphoneEventCbs;

/**
 * @brief Policy to use to pass through firewalls.
 * @ingroup network_parameters
 * @deprecated 03/02/2017 Use #LinphoneNatPolicy instead.
 * @donotwrap
**/
typedef enum _LinphoneFirewallPolicy {
	LinphonePolicyNoFirewall, /**< Do not use any mechanism to pass through firewalls */
	LinphonePolicyUseNatAddress, /**< Use the specified public adress */
	LinphonePolicyUseStun, /**< Use a STUN server to get the public address */
	LinphonePolicyUseIce, /**< Use the ICE protocol */
	LinphonePolicyUseUpnp, /**< Use the uPnP protocol */
} LinphoneFirewallPolicy;

/**
 * @brief Enum describing ICE states.
 * @ingroup initializing
**/
typedef enum _LinphoneIceState {
	LinphoneIceStateNotActivated, /**< ICE has not been activated for this call or stream*/
	LinphoneIceStateFailed, /**< ICE processing has failed */
	LinphoneIceStateInProgress, /**< ICE process is in progress */
	LinphoneIceStateHostConnection, /**< ICE has established a direct connection to the remote host */
	LinphoneIceStateReflexiveConnection, /**< ICE has established a connection to the remote host through one or several NATs */
	LinphoneIceStateRelayConnection /**< ICE has established a connection through a relay */
} LinphoneIceState;

/**
 * @brief IM encryption engine.
 * 
 * @see https://wiki.linphone.org/xwiki/wiki/public/view/Lib/Features/Instant%20Messaging%20Encryption%20Engine/
 * @ingroup misc
 * @donotwrap
 */
typedef struct _LinphoneImEncryptionEngine LinphoneImEncryptionEngine;

/**
 * @brief An object to handle the callbacks for the handling a #LinphoneImEncryptionEngine object.
 * @ingroup misc
 * @donotwrap
 */
typedef struct _LinphoneImEncryptionEngineCbs LinphoneImEncryptionEngineCbs;

/**
 * @brief Policy to use to send/receive instant messaging composing/delivery/display notifications.
 * 
 * The sending of this information is done as in the RFCs 3994 (is_composing) and 5438 (imdn delivered/displayed).
 * @ingroup chatroom
 */
typedef struct _LinphoneImNotifPolicy LinphoneImNotifPolicy;

/**
 * @brief Object representing an informational message sent or received by the core.
 * @ingroup misc
**/
typedef struct _LinphoneInfoMessage LinphoneInfoMessage;

typedef struct _LinphoneLDAPContactProvider LinphoneLDAPContactProvider;

typedef struct _LinphoneLDAPContactSearch LinphoneLDAPContactSearch;

/**
 * @ingroup network_parameters
 */
typedef enum _LinphoneLimeState {
	LinphoneLimeDisabled, /**< Lime is not used at all */
	LinphoneLimeMandatory, /**< Lime is always used */
	LinphoneLimePreferred, /**< Lime is used only if we already shared a secret with remote */
} LinphoneLimeState;

/**
 * @brief Session Timers refresher
 * @ingroup initializing
 */
typedef enum _LinphoneSessionExpiresRefresher {
  LinphoneSessionExpiresRefresherUnspecified,
  LinphoneSessionExpiresRefresherUAS,
  LinphoneSessionExpiresRefresherUAC
} LinphoneSessionExpiresRefresher;

/**
 * @brief Whether or not to keep a file with the logs.
 * @ingroup initializing
 */
typedef enum _LinphoneLogCollectionState {
	LinphoneLogCollectionDisabled,
	LinphoneLogCollectionEnabled,
	LinphoneLogCollectionEnabledWithoutPreviousLogHandler
} LinphoneLogCollectionState;

/**
 * @brief Used to notify if log collection upload have been succesfully delivered or not.
 * @ingroup initializing
 */
typedef enum _LinphoneCoreLogCollectionUploadState {
	LinphoneCoreLogCollectionUploadStateInProgress, /**< Delivery in progress */
	LinphoneCoreLogCollectionUploadStateDelivered, /**< Log collection upload successfully delivered and acknowledged by remote end point */
	LinphoneCoreLogCollectionUploadStateNotDelivered, /**< Log collection upload was not delivered */
} LinphoneCoreLogCollectionUploadState;

/**
 * @brief Indicates for a given media the stream direction.
 * @ingroup call_control
 */
typedef enum _LinphoneMediaDirection {
	LinphoneMediaDirectionInvalid = -1,
	LinphoneMediaDirectionInactive, /** No active media not supported yet*/
	LinphoneMediaDirectionSendOnly, /** Send only mode*/
	LinphoneMediaDirectionRecvOnly, /** recv only mode*/
	LinphoneMediaDirectionSendRecv, /** send receive*/
} LinphoneMediaDirection;

/**
 * @brief Media resource usage.
 * @ingroup media_parameters
**/
typedef enum _LinphoneMediaResourceMode {
	LinphoneExclusiveMediaResources, /**< Media resources are not shared */
	LinphoneSharedMediaResources, /**< Media resources are shared */
} LinphoneMediaResourceMode;

/**
 * @brief Enum describing type of media encryption types.
 * @ingroup media_parameters
**/
typedef enum _LinphoneMediaEncryption {
	LinphoneMediaEncryptionNone, /**< No media encryption is used */
	LinphoneMediaEncryptionSRTP, /**< Use SRTP media encryption */
	LinphoneMediaEncryptionZRTP, /**< Use ZRTP media encryption */
	LinphoneMediaEncryptionDTLS /**< Use DTLS media encryption */
} LinphoneMediaEncryption;

/**
 * @brief Enum describing the ZRTP SAS validation status of a peer URI.
 * @ingroup media_parameters
**/
typedef enum _LinphoneZrtpPeerStatus {
	LinphoneZrtpPeerStatusUnknown, /**< Peer URI unkown or never validated/invalidated the SAS */
	LinphoneZrtpPeerStatusInvalid, /**< Peer URI SAS rejected in database */
	LinphoneZrtpPeerStatusValid /**< Peer URI SAS validated in database */
} LinphoneZrtpPeerStatus;

/**
 * @brief Policy to use to pass through NATs/firewalls.
 * @ingroup network_parameters
 */
typedef struct _LinphoneNatPolicy LinphoneNatPolicy;

/**
 * @brief Enum describing remote friend status.
 * @deprecated 03/02/2017 Use #LinphonePresenceModel and #LinphonePresenceActivity instead
 * @donotwrap
 */
typedef enum _LinphoneOnlineStatus{
	LinphoneStatusOffline, /**< Offline */
	LinphoneStatusOnline, /**< Online */
	LinphoneStatusBusy, /**< Busy */
	LinphoneStatusBeRightBack, /**< Be right back */
	LinphoneStatusAway, /**< Away */
	LinphoneStatusOnThePhone, /** On the phone */
	LinphoneStatusOutToLunch, /**< Out to lunch */
	LinphoneStatusDoNotDisturb, /**< Do not disturb */
	LinphoneStatusMoved, /**< Moved in this sate, call can be redirected if an alternate contact address has been set using function linphone_core_set_presence_info() */
	LinphoneStatusAltService, /**< Using another messaging service */
	LinphoneStatusPending, /**< Pending */
	LinphoneStatusVacation, /**< Vacation */

	LinphoneStatusEnd
} LinphoneOnlineStatus;

/**
 * @brief Player interface.
 * @ingroup call_control
**/
typedef struct _LinphonePlayer LinphonePlayer;

/**
 * @brief An object to handle the callbacks for the handling a #LinphonePlayer objects.
 * @ingroup call_control
 */
typedef struct _LinphonePlayerCbs LinphonePlayerCbs;

/**
 * @brief The state of a #LinphonePlayer.
 * @ingroup call_control
 */
typedef enum LinphonePlayerState {
	LinphonePlayerClosed, /**< No file is opened for playing. */
	LinphonePlayerPaused, /**< The player is paused. */
	LinphonePlayerPlaying /**< The player is playing. */
} LinphonePlayerState;

/**
 * @brief Presence activity type holding information about a presence activity.
 * @ingroup buddy_list
 */
typedef struct _LinphonePresenceActivity LinphonePresenceActivity;

/**
 * @brief Activities as defined in section 3.2 of RFC 4480
 * @ingroup buddy_list
 */
typedef enum LinphonePresenceActivityType {
	/** The person has a calendar appointment, without specifying exactly of what type. This activity is
	 *  indicated if more detailed information is not available or the person chooses not to reveal more
	 * information. */
	LinphonePresenceActivityAppointment,

	/** The person is physically away from all interactive communication devices. */
	LinphonePresenceActivityAway,

	/** The person is eating the first meal of the day, usually eaten in the morning. */
	LinphonePresenceActivityBreakfast,

	/** The person is busy, without further details. */
	LinphonePresenceActivityBusy,

	/** The person is having his or her main meal of the day, eaten in the evening or at midday. */
	LinphonePresenceActivityDinner,

	/**  This is a scheduled national or local holiday. */
	LinphonePresenceActivityHoliday,

	/** The person is riding in a vehicle, such as a car, but not steering. */
	LinphonePresenceActivityInTransit,

	/** The person is looking for (paid) work. */
	LinphonePresenceActivityLookingForWork,

	/** The person is eating his or her midday meal. */
	LinphonePresenceActivityLunch,

	/** The person is scheduled for a meal, without specifying whether it is breakfast, lunch, or dinner,
	 *  or some other meal. */
	LinphonePresenceActivityMeal,

	/** The person is in an assembly or gathering of people, as for a business, social, or religious purpose.
	 *  A meeting is a sub-class of an appointment. */
	LinphonePresenceActivityMeeting,

	/** The person is talking on the telephone. */
	LinphonePresenceActivityOnThePhone,

	/** The person is engaged in an activity with no defined representation. A string describing the activity
	 *  in plain text SHOULD be provided. */
	LinphonePresenceActivityOther,

	/** A performance is a sub-class of an appointment and includes musical, theatrical, and cinematic
	 *  performances as well as lectures. It is distinguished from a meeting by the fact that the person
	 *  may either be lecturing or be in the audience, with a potentially large number of other people,
	 *  making interruptions particularly noticeable. */
	LinphonePresenceActivityPerformance,

	/** The person will not return for the foreseeable future, e.g., because it is no longer working for
	 *  the company. */
	LinphonePresenceActivityPermanentAbsence,

	/** The person is occupying himself or herself in amusement, sport, or other recreation. */
	LinphonePresenceActivityPlaying,

	/** The person is giving a presentation, lecture, or participating in a formal round-table discussion. */
	LinphonePresenceActivityPresentation,

	/** The person is visiting stores in search of goods or services. */
	LinphonePresenceActivityShopping,

	/** The person is sleeping.*/
	LinphonePresenceActivitySleeping,

	/** The person is observing an event, such as a sports event. */
	LinphonePresenceActivitySpectator,

	/** The person is controlling a vehicle, watercraft, or plane. */
	LinphonePresenceActivitySteering,

	/** The person is on a business or personal trip, but not necessarily in-transit. */
	LinphonePresenceActivityTravel,

	/** The person is watching television. */
	LinphonePresenceActivityTV,

	/** The activity of the person is unknown. */
	LinphonePresenceActivityUnknown,

	/** A period of time devoted to pleasure, rest, or relaxation. */
	LinphonePresenceActivityVacation,

	/** The person is engaged in, typically paid, labor, as part of a profession or job. */
	LinphonePresenceActivityWorking,

	/** The person is participating in religious rites. */
	LinphonePresenceActivityWorship
} LinphonePresenceActivityType;

/**
 * @brief Basic status as defined in section 4.1.4 of RFC 3863
 * @ingroup buddy_list
 */
typedef enum LinphonePresenceBasicStatus {
	/** This value means that the associated contact element, if any, is ready to accept communication. */
	LinphonePresenceBasicStatusOpen,

	/** This value means that the associated contact element, if any, is unable to accept communication. */
	LinphonePresenceBasicStatusClosed
} LinphonePresenceBasicStatus;

/**
 * @brief Presence model type holding information about the presence of a person.
 * @ingroup buddy_list
 */
typedef struct _LinphonePresenceModel LinphonePresenceModel;

/**
 * @brief Presence note type holding information about a presence note.
 * @ingroup buddy_list
 */
typedef struct _LinphonePresenceNote LinphonePresenceNote;

/**
 * @brief Presence person holding information about a presence person.
 * @ingroup buddy_list
 */
typedef struct _LinphonePresencePerson LinphonePresencePerson;

/**
 * @brief Presence service type holding information about a presence service.
 * @ingroup buddy_list
 */
typedef struct _LinphonePresenceService LinphonePresenceService;

/**
 * @ingroup call_control
 * Defines privacy policy to apply as described by rfc3323
**/
typedef enum _LinphonePrivacy {
	/**
	 * Privacy services must not perform any privacy function
	 */
	LinphonePrivacyNone = 0x0,
	/**
	 * Request that privacy services provide a user-level privacy
	 * function.
	 * With this mode, "from" header is hidden, usually replaced by  From: "Anonymous" <sip:anonymous@anonymous.invalid>
	 */
	LinphonePrivacyUser = 0x1,
	/**
	 * Request that privacy services modify headers that cannot
	 * be set arbitrarily by the user (Contact/Via).
	 */
	LinphonePrivacyHeader = 0x2,
	/**
	 *  Request that privacy services provide privacy for session
	 * media
	 */
	LinphonePrivacySession = 0x4,
	/**
	 * rfc3325
	 * The presence of this privacy type in
	 * a Privacy header field indicates that the user would like the Network
	 * Asserted Identity to be kept private with respect to SIP entities
	 * outside the Trust Domain with which the user authenticated.  Note
	 * that a user requesting multiple types of privacy MUST include all of
	 * the requested privacy types in its Privacy header field value
	 *
	 */
	LinphonePrivacyId = 0x8,
	/**
	 * Privacy service must perform the specified services or
	 * fail the request
	 *
	 **/
	LinphonePrivacyCritical = 0x10,

	/**
	 * Special keyword to use privacy as defined either globally or by proxy using linphone_proxy_config_set_privacy()
	 */
	LinphonePrivacyDefault = 0x8000,
} LinphonePrivacy;
/* WARNING This enum MUST be kept in sync with the SalPrivacy enum from sal.h */

/**
 * @brief A mask of #LinphonePrivacy values
 * @ingroup call_control
 */
typedef unsigned int LinphonePrivacyMask;

/**
 * @brief Enum for publish states.
 * @ingroup event_api
**/
typedef enum _LinphonePublishState{
	LinphonePublishNone, /**< Initial state, do not use */
	LinphonePublishProgress, /**< An outgoing publish was created and submitted */
	LinphonePublishOk, /**< Publish is accepted */
	LinphonePublishError, /**< Publish encoutered an error, linphone_event_get_reason() gives reason code */
	LinphonePublishExpiring, /**< Publish is about to expire, only sent if [sip]->refresh_generic_publish property is set to 0 */
	LinphonePublishCleared /**< Event has been un published */
} LinphonePublishState;

/**
 * @brief Enum describing various failure reasons or contextual information for some events.
 * @see linphone_call_get_reason()
 * @see linphone_proxy_config_get_error()
 * @see linphone_error_info_get_reason()
 * @ingroup misc
**/
typedef enum _LinphoneReason{
	LinphoneReasonNone, /**< No reason has been set by the core */
	LinphoneReasonNoResponse, /**< No response received from remote */
	LinphoneReasonForbidden, /**< Authentication failed due to bad credentials or resource forbidden */
	LinphoneReasonDeclined, /**< The call has been declined */
	LinphoneReasonNotFound, /**< Destination of the call was not found */
	LinphoneReasonNotAnswered, /**< The call was not answered in time (request timeout) */
	LinphoneReasonBusy, /**< Phone line was busy */
	LinphoneReasonUnsupportedContent, /**< Unsupported content */
	LinphoneReasonBadEvent, /**< Bad event */
	LinphoneReasonIOError, /**< Transport error: connection failures, disconnections etc... */
	LinphoneReasonDoNotDisturb, /**< Do not disturb reason */
	LinphoneReasonUnauthorized, /**< Operation is unauthorized because missing credential */
	LinphoneReasonNotAcceptable, /**< Operation is rejected due to incompatible or unsupported media parameters */
	LinphoneReasonNoMatch, /**< Operation could not be executed by server or remote client because it didn't have any context for it */
	LinphoneReasonMovedPermanently, /**< Resource moved permanently */
	LinphoneReasonGone, /**< Resource no longer exists */
	LinphoneReasonTemporarilyUnavailable, /**< Temporarily unavailable */
	LinphoneReasonAddressIncomplete, /**< Address incomplete */
	LinphoneReasonNotImplemented, /**< Not implemented */
	LinphoneReasonBadGateway, /**< Bad gateway */
	LinphoneReasonSessionIntervalTooSmall, /**< The received request contains a Session-Expires header field with a duration below the minimum timer */
	LinphoneReasonServerTimeout, /**< Server timeout */
	LinphoneReasonUnknown /**< Unknown reason */
} LinphoneReason;

#define LinphoneReasonBadCredentials LinphoneReasonForbidden

/*for compatibility*/
#define LinphoneReasonMedia LinphoneReasonUnsupportedContent

typedef struct _LinphoneRingtonePlayer LinphoneRingtonePlayer;

/**
 * Linphone core SIP transport ports.
 * Special values #LC_SIP_TRANSPORT_RANDOM, #LC_SIP_TRANSPORT_RANDOM, #LC_SIP_TRANSPORT_DONTBIND can be used.
 * Use with #linphone_core_set_sip_transports()
 * @deprecated 18/04/2017
 * @donotwrap
 */
typedef struct _LinphoneSipTransports {
	int udp_port; /**< SIP/UDP port */
	int tcp_port; /**< SIP/TCP port */
	int dtls_port; /**< SIP/DTLS port */
	int tls_port; /**< SIP/TLS port */
} LinphoneSipTransports;

/**
 * Old name of #LinphoneSipTransports
 * @deprecated 03/02/2017
 * @donotwrap
 */
LINPHONE_DEPRECATED typedef struct _LinphoneSipTransports LCSipTransports;

typedef struct _LinphoneSoundDaemon LinphoneSoundDaemon;

/**
 * @brief Enum describing the stream types.
 * @ingroup initializing
**/
typedef enum _LinphoneStreamType {
	LinphoneStreamTypeAudio,
	LinphoneStreamTypeVideo,
	LinphoneStreamTypeText,
	LinphoneStreamTypeUnknown /* WARNING: Make sure this value remains the last one in the list */
} LinphoneStreamType;

/**
 * @brief Enum controlling behavior for incoming subscription request.
 * Use by linphone_friend_set_inc_subscribe_policy()
 * @ingroup buddy_list
 */
typedef enum _LinphoneSubscribePolicy {
	/**
	 * Does not automatically accept an incoming subscription request.
	 * This policy implies that a decision has to be taken for each incoming subscription request notified by callback LinphoneCoreVTable.new_subscription_requested
	 */
	LinphoneSPWait,
	LinphoneSPDeny, /**< Rejects incoming subscription request */
	LinphoneSPAccept /**< Automatically accepts a subscription request */
} LinphoneSubscribePolicy;

/**
 * @brief Enum for subscription direction (incoming or outgoing).
 * @ingroup event_api
**/
typedef enum _LinphoneSubscriptionDir{
	LinphoneSubscriptionIncoming, /**< Incoming subscription. */
	LinphoneSubscriptionOutgoing, /**< Outgoing subscription. */
	LinphoneSubscriptionInvalidDir /**< Invalid subscription direction. */
} LinphoneSubscriptionDir;

/**
 * @brief Enum for subscription states.
 * #LinphoneSubscriptionTerminated and #LinphoneSubscriptionError are final states.
 * @ingroup event_api
**/
typedef enum _LinphoneSubscriptionState{
	LinphoneSubscriptionNone, /**< Initial state, should not be used */
	LinphoneSubscriptionOutgoingProgress, /**< An outgoing subcription was sent */
	LinphoneSubscriptionIncomingReceived, /**< An incoming subcription is received */
	LinphoneSubscriptionPending, /**< Subscription is pending, waiting for user approval */
	LinphoneSubscriptionActive, /**< Subscription is accepted */
	LinphoneSubscriptionTerminated, /**< Subscription is terminated normally */
	LinphoneSubscriptionError, /**< Subscription was terminated by an error, indicated by linphone_event_get_reason() */
	LinphoneSubscriptionExpiring, /**< Subscription is about to expire, only sent if [sip]->refresh_generic_subscribe property is set to 0 */
} LinphoneSubscriptionState;

/**
 * @brief Enum listing frequent telephony tones.
 * @ingroup misc
**/
typedef enum _LinphoneToneID {
	LinphoneToneUndefined, /**< Not a tone */
	LinphoneToneBusy, /**< Busy tone */
	LinphoneToneCallWaiting, /** Call waiting tone */
	LinphoneToneCallOnHold, /** Call on hold tone */
	LinphoneToneCallLost, /** Tone played when call is abruptly disconnected (media lost)*/
	LinphoneToneCallEnd /** When the call end for any reason but lost */
} LinphoneToneID;

/**
 * @brief Enum describing transport type for LinphoneAddress.
 * @ingroup linphone_address
**/
typedef enum _LinphoneTransportType {
	LinphoneTransportUdp,
	LinphoneTransportTcp,
	LinphoneTransportTls,
	LinphoneTransportDtls
} LinphoneTransportType;
/* WARNING This enum MUST be kept in sync with the SalTransport enum from sal.h */

/**
 * @brief Linphone tunnel object.
 * @ingroup tunnel
 */
typedef struct _LinphoneTunnel LinphoneTunnel;

/**
 * @brief Tunnel settings.
 * @ingroup tunnel
 */
typedef struct _LinphoneTunnelConfig LinphoneTunnelConfig;

/**
 * @brief Enum describing the tunnel modes.
 * @ingroup tunnel
**/
typedef enum _LinphoneTunnelMode {
	LinphoneTunnelModeDisable, /**< The tunnel is disabled */
	LinphoneTunnelModeEnable, /**< The tunnel is enabled */
	LinphoneTunnelModeAuto /**< The tunnel is enabled automatically if it is required */
} LinphoneTunnelMode;

/**
 * @brief Enum describing uPnP states.
 * @ingroup initializing
**/
typedef enum _LinphoneUpnpState {
	LinphoneUpnpStateIdle, /**< uPnP is not activate */
	LinphoneUpnpStatePending, /**< uPnP process is in progress */
	LinphoneUpnpStateAdding, /**< Internal use: Only used by port binding */
	LinphoneUpnpStateRemoving, /**< Internal use: Only used by port binding */
	LinphoneUpnpStateNotAvailable, /**< uPnP is not available */
	LinphoneUpnpStateOk, /**< uPnP is enabled */
	LinphoneUpnpStateKo, /**< uPnP processing has failed */
	LinphoneUpnpStateBlacklisted, /**< IGD router is blacklisted */
} LinphoneUpnpState;

/**
 * @brief Object storing contact information using vCard 4.0 format.
 * @ingroup carddav_vcard
 */
typedef struct _LinphoneVcard LinphoneVcard;

/**
 * @brief Enum describing the result of a version update check.
 * @ingroup misc
 */
typedef enum _LinphoneVersionUpdateCheckResult {
	LinphoneVersionUpdateCheckUpToDate,
	LinphoneVersionUpdateCheckNewVersionAvailable,
	LinphoneVersionUpdateCheckError
} LinphoneVersionUpdateCheckResult;

/**
 * @brief Structure describing policy regarding video streams establishments.
 * @ingroup media_parameters
 * @deprecated 18/04/17
 * @donotwrap
**/
typedef struct _LinphoneVideoPolicy {
	bool_t automatically_initiate; /**<Whether video shall be automatically proposed for outgoing calls.*/
	bool_t automatically_accept; /**<Whether video shall be automatically accepted for incoming calls*/
	bool_t unused[2];
} LinphoneVideoPolicy;

typedef struct LinphoneVideoSizeDef {
	MSVideoSize vsize;
	const char *name;
} LinphoneVideoSizeDef;

/**
 * Old name of #LinphoneVideoSizeDef
 * @deprecated 03/02/2017
 * @donotwrap
 */
typedef LinphoneVideoSizeDef MSVideoSizeDef;

typedef enum _LinphoneWaitingState {
	LinphoneWaitingStart,
	LinphoneWaitingProgress,
	LinphoneWaitingFinished
} LinphoneWaitingState;

/**
* @brief Enum describing the types of argument for LinphoneXmlRpcRequest.
* @ingroup misc
**/
typedef enum _LinphoneXmlRpcArgType {
	LinphoneXmlRpcArgNone,
	LinphoneXmlRpcArgInt,
	LinphoneXmlRpcArgString,
	LinphoneXmlRpcArgStringStruct
} LinphoneXmlRpcArgType;

/**
 * @brief The #LinphoneXmlRpcRequest object representing a XML-RPC request to be sent.
 * @ingroup misc
**/
typedef struct _LinphoneXmlRpcRequest LinphoneXmlRpcRequest;

/**
 * @brief An object to handle the callbacks for handling the #LinphoneXmlRpcRequest operations.
 * @ingroup misc
**/
typedef struct _LinphoneXmlRpcRequestCbs LinphoneXmlRpcRequestCbs;

/**
 * @brief The #LinphoneXmlRpcSession object used to send XML-RPC requests and handle their responses.
 * @ingroup misc
**/
typedef struct _LinphoneXmlRpcSession LinphoneXmlRpcSession;

/**
* @brief Enum describing the status of a LinphoneXmlRpcRequest.
* @ingroup misc
**/
typedef enum _LinphoneXmlRpcStatus {
	LinphoneXmlRpcStatusPending,
	LinphoneXmlRpcStatusOk,
	LinphoneXmlRpcStatusFailed
} LinphoneXmlRpcStatus;

typedef struct _LsdPlayer LsdPlayer;

/**
 * @brief Object representing an RTP payload type.
 * @ingroup media_parameters
 */
typedef struct _LinphonePayloadType LinphonePayloadType;

/**
 * @brief Structure describing a range of integers
 * @ingroup misc
 */
typedef struct _LinphoneRange LinphoneRange;

/**
 * @brief Status code returned by some functions to
 * notify whether the execution has been succesfully
 * done or not.
 * @ingroup misc
 */
typedef int LinphoneStatus;

/**
 * @brief Object representing a chain of protocol headers.
 * It provides read/write access to the headers of the underlying protocol.
 * @ingroup misc
**/
typedef struct _LinphoneHeaders LinphoneHeaders;

#endif /* LINPHONE_TYPES_H_ */
