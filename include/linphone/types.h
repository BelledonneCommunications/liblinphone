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

#ifndef LINPHONE_TYPES_H_
#define LINPHONE_TYPES_H_

#include "linphone/defs.h"
#include "mediastreamer2/msinterfaces.h"
#include "mediastreamer2/msvideo.h"
#include "mediastreamer2/zrtp.h"
#include "ortp/payloadtype.h"

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
 * A minimal proxy config consists of an identity address (sip:username\@domain.tld)
 * and the proxy server address, @see linphone_proxy_config_set_server_addr().
 *
 * If any, it will be stored inside the default configuration file, so it will survive the destruction
 * of the #LinphoneCore and be available at the next start.
 *
 * The account set with linphone_core_set_default_proxy_config() will be used as default
 * for outgoing calls & chat messages unless specified otherwise.
 * @ingroup proxies
 * @deprecated 06/04/2020 Use #LinphoneAccount object instead
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
	LinphoneAccountCreatorPhoneNumberStatusOk = 0x1,                 /**< Phone number ok */
	LinphoneAccountCreatorPhoneNumberStatusTooShort = 0x2,           /**< Phone number too short */
	LinphoneAccountCreatorPhoneNumberStatusTooLong = 0x4,            /**< Phone number too long */
	LinphoneAccountCreatorPhoneNumberStatusInvalidCountryCode = 0x8, /**< Country code invalid */
	LinphoneAccountCreatorPhoneNumberStatusInvalid = 0x10            /**< Phone number invalid */
} LinphoneAccountCreatorPhoneNumberStatus;

/**
 * @brief A mask of #LinphoneAccountCreatorPhoneNumberStatus values.
 * @ingroup account_creator
 */
typedef unsigned int LinphoneAccountCreatorPhoneNumberStatusMask;

/**
 * @brief Enum describing backend used in the #LinphoneAccountCreator.
 * @ingroup account_creator
 **/
typedef enum _LinphoneAccountCreatorBackend {
	LinphoneAccountCreatorBackendXMLRPC = 0,   /**< XMLRPC Backend */
	LinphoneAccountCreatorBackendFlexiAPI = 1, /**< FlexiAPI Backend */
} LinphoneAccountCreatorBackend;

/**
 * @brief Enum describing username checking, used by the #LinphoneAccountCreator.
 * @ingroup account_creator
 **/
typedef enum _LinphoneAccountCreatorUsernameStatus {
	LinphoneAccountCreatorUsernameStatusOk = 0,                /**< Username ok */
	LinphoneAccountCreatorUsernameStatusTooShort = 1,          /**< Username too short */
	LinphoneAccountCreatorUsernameStatusTooLong = 2,           /**< Username too long */
	LinphoneAccountCreatorUsernameStatusInvalidCharacters = 3, /**< Contain invalid characters */
	LinphoneAccountCreatorUsernameStatusInvalid = 4            /**< Invalid username */
} LinphoneAccountCreatorUsernameStatus;

/**
 * @brief Enum describing email checking, used by the #LinphoneAccountCreator.
 * @ingroup account_creator
 **/
typedef enum _LinphoneAccountCreatorEmailStatus {
	LinphoneAccountCreatorEmailStatusOk = 0,               /**< Email ok */
	LinphoneAccountCreatorEmailStatusMalformed = 1,        /**< Email malformed */
	LinphoneAccountCreatorEmailStatusInvalidCharacters = 2 /**< Contain invalid characters */
} LinphoneAccountCreatorEmailStatus;

/**
 * @brief Enum describing password checking, used by the #LinphoneAccountCreator.
 * @ingroup account_creator
 **/
typedef enum _LinphoneAccountCreatorPasswordStatus {
	LinphoneAccountCreatorPasswordStatusOk = 0,                /**< Password ok */
	LinphoneAccountCreatorPasswordStatusTooShort = 1,          /**< Password too short */
	LinphoneAccountCreatorPasswordStatusTooLong = 2,           /**< Password too long */
	LinphoneAccountCreatorPasswordStatusInvalidCharacters = 3, /**< Contain invalid characters */
	LinphoneAccountCreatorPasswordStatusMissingCharacters = 4  /**< Missing specific characters */
} LinphoneAccountCreatorPasswordStatus;

/**
 * @brief Enum describing language checking, used by the #LinphoneAccountCreator.
 * @ingroup account_creator
 **/
typedef enum _LinphoneAccountCreatorLanguageStatus {
	LinphoneAccountCreatorLanguageStatusOk = 0 /**< Language ok */
} LinphoneAccountCreatorLanguageStatus;

/**
 * @brief Enum describing algorithm checking, used by the #LinphoneAccountCreator.
 * @ingroup account_creator
 **/
typedef enum _LinphoneAccountCreatorAlgoStatus {
	LinphoneAccountCreatorAlgoStatusOk = 0,          /**< Algorithm ok */
	LinphoneAccountCreatorAlgoStatusNotSupported = 1 /**< Algorithm not supported */
} LinphoneAccountCreatorAlgoStatus;

/**
 * @brief Enum describing activation code checking, used by the #LinphoneAccountCreator.
 * @ingroup account_creator
 **/
typedef enum _LinphoneAccountCreatorActivationCodeStatus {
	LinphoneAccountCreatorActivationCodeStatusOk = 0,               /**< Activation code ok */
	LinphoneAccountCreatorActivationCodeStatusTooShort = 1,         /**< Activation code too short */
	LinphoneAccountCreatorActivationCodeStatusTooLong = 2,          /**< Activation code too long */
	LinphoneAccountCreatorActivationCodeStatusInvalidCharacters = 3 /**< Contain invalid characters */
} LinphoneAccountCreatorActivationCodeStatus;

/**
 * @brief Enum describing domain checking, used by the #LinphoneAccountCreator.
 * @ingroup account_creator
 **/
typedef enum _LinphoneAccountCreatorDomainStatus {
	LinphoneAccountCreatorDomainOk = 0,     /**< Domain ok */
	LinphoneAccountCreatorDomainInvalid = 1 /**< Domain invalid */
} LinphoneAccountCreatorDomainStatus;

/**
 * @brief Enum describing transport checking, used by the #LinphoneAccountCreator.
 * @ingroup account_creator
 **/
typedef enum _LinphoneAccountCreatorTransportStatus {
	LinphoneAccountCreatorTransportOk = 0,         /**< Transport ok */
	LinphoneAccountCreatorTransportUnsupported = 1 /**< Transport invalid */
} LinphoneAccountCreatorTransportStatus;

/**
 * @brief Enum describing the status of server request, used by the #LinphoneAccountCreator.
 * @ingroup account_creator_request
 **/
typedef enum _LinphoneAccountCreatorStatus {
	/** Request status **/
	LinphoneAccountCreatorStatusRequestOk = 0,        /**< Request passed */
	LinphoneAccountCreatorStatusRequestFailed = 1,    /**< Request failed */
	LinphoneAccountCreatorStatusMissingArguments = 2, /**< Request failed due to missing argument(s) */
	LinphoneAccountCreatorStatusMissingCallbacks = 3, /**< Request failed due to missing callback(s) */

	/** Account status **/
	/* Creation */
	LinphoneAccountCreatorStatusAccountCreated = 4,    /**< Account created */
	LinphoneAccountCreatorStatusAccountNotCreated = 5, /**< Account not created */
	/* Existence */
	LinphoneAccountCreatorStatusAccountExist = 6,          /**< Account exist */
	LinphoneAccountCreatorStatusAccountExistWithAlias = 7, /**< Account exist with alias */
	LinphoneAccountCreatorStatusAccountNotExist = 8,       /**< Account not exist */
	LinphoneAccountCreatorStatusAliasIsAccount = 9,        /**< Account was created with Alias */
	LinphoneAccountCreatorStatusAliasExist = 10,           /**< Alias exist */
	LinphoneAccountCreatorStatusAliasNotExist = 11,        /**< Alias not exist */
	/* Activation */
	LinphoneAccountCreatorStatusAccountActivated = 12,        /**< Account activated */
	LinphoneAccountCreatorStatusAccountAlreadyActivated = 13, /**< Account already activated */
	LinphoneAccountCreatorStatusAccountNotActivated = 14,     /**< Account not activated */
	/* Linking */
	LinphoneAccountCreatorStatusAccountLinked = 15,    /**< Account linked */
	LinphoneAccountCreatorStatusAccountNotLinked = 16, /**< Account not linked */

	/** Server **/
	LinphoneAccountCreatorStatusServerError = 17, /**< Error server */

	LinphoneAccountCreatorStatusPhoneNumberInvalid = 18,  /**< Error cannot send SMS */
	LinphoneAccountCreatorStatusWrongActivationCode = 19, /**< Error key doesn't match */
	LinphoneAccountCreatorStatusPhoneNumberOverused = 20, /**< Error too many SMS sent */
	LinphoneAccountCreatorStatusAlgoNotSupported = 21,    /**< Error algo isn't MD5 or SHA-256 */
	LinphoneAccountCreatorStatusUnexpectedError = 22,     /**< Generic error */
	LinphoneAccountCreatorStatusNotImplementedError = 23, /**< This API isn't implemented in the current backend */
	LinphoneAccountCreatorStatusRequestNotAuthorized =
	    24, /**< Request has been denied, probably due to invalid auth token */
	LinphoneAccountCreatorStatusRequestTooManyRequests =
	    25, /**< Request has been denied, due to too many requests sent in given period */
} LinphoneAccountCreatorStatus;

// -----------------------------------------------------------------------------
// Call.
// -----------------------------------------------------------------------------

/**
 * @brief Enum representing the direction of a call.
 * @ingroup call_logs
 **/
typedef enum _LinphoneCallDir {
	LinphoneCallOutgoing = 0, /**< outgoing calls*/
	LinphoneCallIncoming = 1  /**< incoming calls*/
} LinphoneCallDir;

/**
 * @ingroup call_misc
 */
typedef enum _LinphoneSupportLevel {
	LinphoneSupportLevelNoSupport = 0, /**< No support for the feature */
	LinphoneSupportLevelOptional = 1,  /**< Optional support for the feature */
	LinphoneSupportLevelMandatory = 2, /**< Mandatory support for the feature */
} LinphoneSupportLevel;

/**
 * @brief Enum representing the status of a call.
 * @ingroup call_logs
 **/
typedef enum _LinphoneCallStatus {
	LinphoneCallSuccess = 0,  /**< The call was sucessful */
	LinphoneCallAborted = 1,  /**< The call was aborted (caller hanged up) */
	LinphoneCallMissed = 2,   /**< The call was missed (incoming call timed out without being answered or hanged up) */
	LinphoneCallDeclined = 3, /**< The call was declined, either locally or by remote end */
	LinphoneCallEarlyAborted =
	    4, /**<The call was aborted before being advertised to the application - for protocol reasons*/
	LinphoneCallAcceptedElsewhere = 5, /**<The call was answered on another device*/
	LinphoneCallDeclinedElsewhere = 6  /**<The call was declined on another device*/
} LinphoneCallStatus;

/**
 * @brief Enum representing the state of a recording.
 * @ingroup call_control
 **/
typedef enum _LinphoneRecorderState {
	LinphoneRecorderClosed, /**< No file is opened for recording. */
	LinphoneRecorderPaused, /**< The recorder is paused. */
	LinphoneRecorderRunning /**< The recorder is running. */
} LinphoneRecorderState;

/**
 * @brief Enum representing the type of a video source.
 * @ingroup call_control
 **/
typedef enum _LinphoneVideoSourceType {
	LinphoneVideoSourceUnknown,
	LinphoneVideoSourceCall,         /**< The video source is another call. */
	LinphoneVideoSourceCamera,       /**< The video source is a camera. */
	LinphoneVideoSourceImage,        /**< The video source is an image. */
	LinphoneVideoSourceScreenSharing /**< The video source is a screen sharing. */
} LinphoneVideoSourceType;

/**
 * @brief Enum representing the sub type of the screen sharing.
 * @ingroup call_control
 **/
typedef enum _LinphoneVideoSourceScreenSharingType {
	LinphoneVideoSourceScreenSharingDisplay, /**< The screen sharing is done from a display. */
	LinphoneVideoSourceScreenSharingWindow,  /**< The screen sharing is done from a window. */
	LinphoneVideoSourceScreenSharingArea     /**< The screen sharing is done from an area. */
} LinphoneVideoSourceScreenSharingType;

// -----------------------------------------------------------------------------
// Friends.
// -----------------------------------------------------------------------------

/**
 * @brief This object is used to store a SIP address.
 *
 * #LinphoneFriend is mainly used to implement an adressbook feature, and are used as data for the #LinphoneMagicSearch
 * object. If your proxy supports it, you can also use it to subscribe to presence information.
 *
 * The objects are stored in a #LinphoneFriendList which are in turn stored inside the #LinphoneCore.
 * They can be stored inside a database if the path to it is configured, otherwise they will be lost after the
 * #LinphoneCore is destroyed.
 *
 * Thanks to the vCard plugin, you can also store more information like phone numbers, organization, etc...
 * @ingroup buddy_list
 */
typedef struct _LinphoneFriend LinphoneFriend;

/**
 * @brief An object to handle the callbacks for #LinphoneFriend.
 * @ingroup buddy_list
 **/
typedef struct _LinphoneFriendCbs LinphoneFriendCbs;

/**
 * @brief Enum describing the capabilities of a #LinphoneFriend, populated through presence subscribe/notify process.
 * @ingroup buddy_list
 **/
typedef enum _LinphoneFriendCapability {
	LinphoneFriendCapabilityNone = 0, /**< No capabilities populated */
	LinphoneFriendCapabilityGroupChat = 1
	                                    << 0, /**< This friend can be invited in a Flexisip backend #LinphoneChatRoom */
	LinphoneFriendCapabilityLimeX3dh =
	    1 << 1, /**< This friend can be invited in a Flexisip backend end-to-end encrypted #LinphoneChatRoom */
	LinphoneFriendCapabilityEphemeralMessages =
	    1 << 2 /**< This friend is able to delete ephemeral messages once they have expired */
} LinphoneFriendCapability;

/**
 * @brief This object representing a list of #LinphoneFriend.
 *
 * You can use it to store contacts locally or synchronize them through CardDAV protocol.
 * @ingroup buddy_list
 **/
typedef struct _LinphoneFriendList LinphoneFriendList;

/**
 * @brief The types of FriendList
 * @ingroup buddy_list
 */
typedef enum _LinphoneFriendListType {
	LinphoneFriendListTypeDefault = -1,        /**< Default value, used when no other type applies */
	LinphoneFriendListTypeCardDAV = 0,         /**< Used when list is synchronized with a remote CardDAV server */
	LinphoneFriendListTypeVCard4 = 1,          /**< Used for simple vCards list remotely provisionned by a server */
	LinphoneFriendListTypeApplicationCache = 2 /**< Friend list used by app for cache purposes, friends added in
	                                              this list will be ignored by #LinphoneMagicSearch  */
} LinphoneFriendListType;

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
	LinphoneFriendListOK = 0,                /**< Operation went fine */
	LinphoneFriendListNonExistentFriend = 1, /**< #LinphoneFriend wasn't found in the #LinphoneFriendList */
	LinphoneFriendListInvalidFriend = 2      /**< #LinphoneFriend is already present in a #LinphoneFriendList */
} LinphoneFriendListStatus;

/**
 * @brief Enum describing the status of a CardDAV synchronization.
 * @ingroup buddy_list
 */
typedef enum _LinphoneFriendListSyncStatus {
	LinphoneFriendListSyncStarted = 0,    /**< Synchronization started */
	LinphoneFriendListSyncSuccessful = 1, /**< Synchronization finished successfuly */
	LinphoneFriendListSyncFailure = 2     /**< Synchronization failed */
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
 * Various types can be used: strings and lists of strings, integers, floats, booleans (written as 0 or 1) and range of
 *integers.
 *
 * Usually a #LinphoneCore is initialized using two #LinphoneConfig, one default (where configuration changes through
 *API calls will be saved) and one named 'factory' which is read-only and overwrites any setting that may exists in the
 *default one.
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
 * @brief Describes the state of the remote configuring process of the #LinphoneCore object, 'Skipped' when the feature
 *is disabled.
 *
 * It is notified via the configuring_status() callback in #LinphoneCoreCbs.
 * @ingroup initializing
 **/
typedef enum _LinphoneConfiguringState {
	LinphoneConfiguringSuccessful = 0,
	LinphoneConfiguringFailed = 1,
	LinphoneConfiguringSkipped = 2
} LinphoneConfiguringState;

/**
 * @brief Describes the global state of the #LinphoneCore object.
 *
 * It is notified via the global_state_changed() callback in #LinphoneCoreCbs.
 * @ingroup initializing
 **/
typedef enum _LinphoneGlobalState {
	/** State in which we're in after linphone_core_stop(). Do not call any method while in this state except for
	   linphone_core_start() */
	LinphoneGlobalOff = 0,
	/** Transient state for when we call linphone_core_start() */
	LinphoneGlobalStartup = 1,
	/** Indicates #LinphoneCore has been started and is up and running */
	LinphoneGlobalOn = 2,
	/** Transient state for when we call linphone_core_stop() */
	LinphoneGlobalShutdown = 3,
	/** Transient state between Startup and On if there is a remote provisionning URI configured */
	LinphoneGlobalConfiguring = 4,
	/** #LinphoneCore state after being created by linphone_factory_create_core(), generally followed by a call to
	   linphone_core_start() */
	LinphoneGlobalReady = 5
} LinphoneGlobalState;

/**
 * @brief Describes proxy registration states.
 *
 * It is notified via the registration_state_changed() callback in #LinphoneCoreCbs.
 * @ingroup proxies
 **/
typedef enum _LinphoneRegistrationState {
	LinphoneRegistrationNone = 0,      /**< Initial state for registrations */
	LinphoneRegistrationProgress = 1,  /**< Registration is in progress */
	LinphoneRegistrationOk = 2,        /**< Registration is successful */
	LinphoneRegistrationCleared = 3,   /**< Unregistration succeeded */
	LinphoneRegistrationFailed = 4,    /**< Registration failed */
	LinphoneRegistrationRefreshing = 5 /**< Registration refreshing */
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
 * It is recommended to add a #LinphoneCoreCbs listener using linphone_core_add_callbacks() to monitor different events.
 *
 * To be able to receive events from the network, you must schedule a call linphone_core_iterate() often, like every
 * 20ms. On Android & iOS linphone_core_is_auto_iterate_enabled() is enabled by default so you don't have to worry about
 * that unless you disable it using linphone_core_enable_auto_iterate() or by setting in the [misc] section of your
 * configuration auto_iterate=0.
 * @warning Our API isn't thread-safe but also isn't blocking, so it is strongly recommend to always call our methods
 * from the main thread.
 *
 * Once you don't need it anymore, call linphone_core_stop() and release the reference on it so it can gracefully
 * shutdown.
 * @ingroup initializing
 */
typedef struct _LinphoneCore LinphoneCore;

/**
 * @brief The factory is a singleton object devoted to the creation of all the objects
 * of Liblinphone that cannot be created by #LinphoneCore itself.
 *
 * It is also used to configure a few behaviors before creating the #LinphoneCore, like the logs verbosity or
 * collection.
 * @ingroup initializing
 */
typedef struct _LinphoneFactory LinphoneFactory;

/**
 * @brief That class holds all the callbacks which are called by #LinphoneCore.
 *
 * Once created, add your #LinphoneCoreCbs using linphone_core_add_callbacks().
 * Keep a reference on it as long as you need it.
 * You can use linphone_core_remove_callbacks() to remove it but that isn't mandatory.
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
 * You can use special values like #LC_SIP_TRANSPORT_DISABLED (0), #LC_SIP_TRANSPORT_RANDOM (-1) and
 * #LC_SIP_TRANSPORT_DONTBIND (-2).
 *
 * Once configuration is complete, use linphone_core_set_transports() to apply it.
 * This will be saved in configuration file so you don't have to do it each time the #LinphoneCore starts.
 * @ingroup initializing
 */
typedef struct _LinphoneTransports LinphoneTransports;

/**
 * @brief Object describing policy regarding video streams establishments.
 *
 * Use linphone_video_activation_policy_set_automatically_accept() and
 *linphone_video_activation_policy_set_automatically_initiate() to tell the Core to automatically accept or initiate
 *video during calls.
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
	LinphoneAddressFamilyInet = 0,   /**< IpV4 */
	LinphoneAddressFamilyInet6 = 1,  /**< IpV6 */
	LinphoneAddressFamilyUnspec = 2, /**< Unknown */
} LinphoneAddressFamily;

/**
 * @brief Enum describing the authentication methods.
 * @ingroup network_parameters
 **/
typedef enum _LinphoneAuthMethod {
	LinphoneAuthHttpDigest = 0, /**< Digest authentication requested */
	LinphoneAuthTls = 1,        /**< Client certificate requested */
	LinphoneAuthBearer = 2,     /**< Bearer authentication */
	LinphoneAuthBasic = 3       /**< Basic authentication */
} LinphoneAuthMethod;

/**
 * @brief Enum describing RTP AVPF activation modes.
 * @ingroup media_parameters
 **/
typedef enum _LinphoneAVPFMode {
	LinphoneAVPFDefault = -1, /**< Use default value defined at upper level */
	LinphoneAVPFDisabled = 0, /**< AVPF is disabled */
	LinphoneAVPFEnabled = 1   /**< AVPF is enabled */
} LinphoneAVPFMode;

/**
 * Consolidated presence information: 'online' means the user is open for communication,
 * 'busy' means the user is open for communication but involved in an other activity,
 * 'do not disturb' means the user is not open for communication, and 'offline' means
 * that no presence information is available.
 * @ingroup buddy_list
 */
typedef enum _LinphoneConsolidatedPresence {
	LinphoneConsolidatedPresenceOnline = 0,
	LinphoneConsolidatedPresenceBusy = 1,
	LinphoneConsolidatedPresenceDoNotDisturb = 2,
	LinphoneConsolidatedPresenceOffline = 3
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

/**
 * @brief Enum describing the result of the echo canceller calibration process.
 * @ingroup media_parameters
 **/
typedef enum _LinphoneEcCalibratorStatus {
	LinphoneEcCalibratorInProgress = 0, /**< The echo canceller calibration process is on going */
	LinphoneEcCalibratorDone =
	    1, /**< The echo canceller calibration has been performed and produced an echo delay measure */
	LinphoneEcCalibratorFailed = 2, /**< The echo canceller calibration process has failed */
	LinphoneEcCalibratorDoneNoEcho =
	    3 /**< The echo canceller calibration has been performed and no echo has been detected */
} LinphoneEcCalibratorStatus;

/**
 * @brief Object representing full details about a signaling error or status.
 *
 * All #LinphoneErrorInfo object returned by the liblinphone API are readonly and transcients. For safety they must be
 *used immediately after obtaining them. Any other function call to the liblinphone may change their content or
 *invalidate the pointer.
 * @ingroup misc
 **/
typedef struct _LinphoneErrorInfo LinphoneErrorInfo;

/**
 * @brief Policy to use to pass through firewalls.
 * @ingroup network_parameters
 * @deprecated 03/02/2017 Use #LinphoneNatPolicy instead.
 * @donotwrap
 **/
typedef enum _LinphoneFirewallPolicy {
	LinphonePolicyNoFirewall = 0,    /**< Do not use any mechanism to pass through firewalls */
	LinphonePolicyUseNatAddress = 1, /**< Use the specified public adress */
	LinphonePolicyUseStun = 2,       /**< Use a STUN server to get the public address */
	LinphonePolicyUseIce = 3,        /**< Use the ICE protocol */
	LinphonePolicyUseUpnp = 4,       /**< Use the uPnP protocol */
} LinphoneFirewallPolicy;

/**
 * @brief Enum describing ICE states.
 * @ingroup initializing
 **/
typedef enum _LinphoneIceState {
	LinphoneIceStateNotActivated = 0,   /**< ICE has not been activated for this call or stream*/
	LinphoneIceStateFailed = 1,         /**< ICE processing has failed */
	LinphoneIceStateInProgress = 2,     /**< ICE process is in progress */
	LinphoneIceStateHostConnection = 3, /**< ICE has established a direct connection to the remote host */
	LinphoneIceStateReflexiveConnection =
	    4, /**< ICE has established a connection to the remote host through one or several NATs */
	LinphoneIceStateRelayConnection = 5 /**< ICE has established a connection through a relay */
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
	LinphoneLimeDisabled = 0,  /**< Lime is not used at all */
	LinphoneLimeMandatory = 1, /**< Lime is always used */
	LinphoneLimePreferred = 2, /**< Lime is used only if we already shared a secret with remote */
} LinphoneLimeState;

/**
 * @brief Session Timers refresher
 * @ingroup initializing
 */
typedef enum _LinphoneSessionExpiresRefresher {
	LinphoneSessionExpiresRefresherUnspecified = 0,
	LinphoneSessionExpiresRefresherUAS = 1,
	LinphoneSessionExpiresRefresherUAC = 2
} LinphoneSessionExpiresRefresher;

/**
 * @brief Whether or not to keep a file with the logs.
 * @ingroup initializing
 */
typedef enum _LinphoneLogCollectionState {
	LinphoneLogCollectionDisabled = 0,
	LinphoneLogCollectionEnabled = 1,
	LinphoneLogCollectionEnabledWithoutPreviousLogHandler = 2
} LinphoneLogCollectionState;

/**
 * @brief Used to notify if log collection upload have been successfully delivered or not.
 * @ingroup initializing
 */
typedef enum _LinphoneCoreLogCollectionUploadState {
	LinphoneCoreLogCollectionUploadStateInProgress = 0, /**< Delivery in progress */
	LinphoneCoreLogCollectionUploadStateDelivered =
	    1, /**< Log collection upload successfully delivered and acknowledged by remote end point */
	LinphoneCoreLogCollectionUploadStateNotDelivered = 2, /**< Log collection upload was not delivered */
} LinphoneCoreLogCollectionUploadState;

/**
 * @brief Indicates for a given media the stream direction.
 * @ingroup call_control
 */
typedef enum _LinphoneMediaDirection {
	LinphoneMediaDirectionInvalid = -1, /**< Default value, shouldn't be used */
	LinphoneMediaDirectionInactive = 0, /**< No active media not supported yet*/
	LinphoneMediaDirectionSendOnly = 1, /**< Media is only being sent, it won't be received*/
	LinphoneMediaDirectionRecvOnly = 2, /**< Media will only be received, nothing will be sent*/
	LinphoneMediaDirectionSendRecv = 3, /**< Media will be sent and received*/
} LinphoneMediaDirection;

/**
 * @brief Media resource usage.
 * @ingroup media_parameters
 **/
typedef enum _LinphoneMediaResourceMode {
	LinphoneExclusiveMediaResources = 0, /**< Media resources are not shared */
	LinphoneSharedMediaResources = 1,    /**< Media resources are shared */
} LinphoneMediaResourceMode;

/**
 * @brief Enum describing type of media encryption types.
 * @ingroup media_parameters
 **/
typedef enum _LinphoneMediaEncryption {
	LinphoneMediaEncryptionNone = 0, /**< No media encryption is used */
	LinphoneMediaEncryptionSRTP = 1, /**< Use SRTP media encryption */
	LinphoneMediaEncryptionZRTP = 2, /**< Use ZRTP media encryption */
	LinphoneMediaEncryptionDTLS = 3  /**< Use DTLS media encryption */
} LinphoneMediaEncryption;

/**
 * @brief Enum describing type of SRTP encryption suite
 * @ingroup media_parameters
 **/
typedef enum _LinphoneSrtpSuite {
	LinphoneSrtpSuiteAESCM128HMACSHA180 = 0,
	LinphoneSrtpSuiteAESCM128HMACSHA132 = 1,
	LinphoneSrtpSuiteAES192CMHMACSHA180 = 2,
	LinphoneSrtpSuiteAES192CMHMACSHA132 = 3,
	LinphoneSrtpSuiteAES256CMHMACSHA180 = 4,
	LinphoneSrtpSuiteAES256CMHMACSHA132 = 5,
	LinphoneSrtpSuiteAEADAES128GCM = 6,
	LinphoneSrtpSuiteAEADAES256GCM = 7,
	LinphoneSrtpSuiteInvalid = 0xFF
} LinphoneSrtpSuite;

/**
 * @brief Enum describing the ZRTP SAS validation status of a peer URI.
 * @ingroup media_parameters
 **/
typedef enum _LinphoneZrtpPeerStatus {
	LinphoneZrtpPeerStatusUnknown = 0, /**< Peer URI unkown or never validated/invalidated the SAS */
	LinphoneZrtpPeerStatusInvalid = 1, /**< Peer URI SAS rejected in database */
	LinphoneZrtpPeerStatusValid = 2    /**< Peer URI SAS validated in database */
} LinphoneZrtpPeerStatus;

/**
 * @brief Enum describing the ZRTP key exchange algorithns
 * @ingroup media_parameters
 **/
typedef enum _LinphoneZrtpKeyAgreement {
	LinphoneZrtpKeyAgreementInvalid = 0,
	LinphoneZrtpKeyAgreementDh2k = 1,
	LinphoneZrtpKeyAgreementDh3k = 2,
	LinphoneZrtpKeyAgreementEc25 = 3,
	LinphoneZrtpKeyAgreementEc38 = 4,
	LinphoneZrtpKeyAgreementEc52 = 5,
	LinphoneZrtpKeyAgreementX255 = 6,
	LinphoneZrtpKeyAgreementX448 = 7,
	LinphoneZrtpKeyAgreementK255 = 8,
	LinphoneZrtpKeyAgreementK448 = 9,
	LinphoneZrtpKeyAgreementKyb1 = 10,
	LinphoneZrtpKeyAgreementKyb2 = 11,
	LinphoneZrtpKeyAgreementKyb3 = 12,
	LinphoneZrtpKeyAgreementHqc1 = 13,
	LinphoneZrtpKeyAgreementHqc2 = 14,
	LinphoneZrtpKeyAgreementHqc3 = 15,
	LinphoneZrtpKeyAgreementK255Kyb512 = 16,
	LinphoneZrtpKeyAgreementK255Hqc128 = 17,
	LinphoneZrtpKeyAgreementK448Kyb1024 = 18,
	LinphoneZrtpKeyAgreementK448Hqc256 = 19,
	LinphoneZrtpKeyAgreementK255Kyb512Hqc128 = 20,
	LinphoneZrtpKeyAgreementK448Kyb1024Hqc256 = 21,
	LinphoneZrtpKeyAgreementMlk1 = 22,
	LinphoneZrtpKeyAgreementMlk2 = 23,
	LinphoneZrtpKeyAgreementMlk3 = 24,
	LinphoneZrtpKeyAgreementK255Mlk512 = 25,
	LinphoneZrtpKeyAgreementK448Mlk1024 = 26
} LinphoneZrtpKeyAgreement;

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
typedef enum _LinphoneOnlineStatus {
	LinphoneStatusOffline = 0,      /**< Offline */
	LinphoneStatusOnline = 1,       /**< Online */
	LinphoneStatusBusy = 2,         /**< Busy */
	LinphoneStatusBeRightBack = 3,  /**< Be right back */
	LinphoneStatusAway = 4,         /**< Away */
	LinphoneStatusOnThePhone = 5,   /**< On the phone */
	LinphoneStatusOutToLunch = 6,   /**< Out to lunch */
	LinphoneStatusDoNotDisturb = 7, /**< Do not disturb */
	LinphoneStatusMoved = 8, /**< Moved in this sate, call can be redirected if an alternate contact address has been
	                            set using function linphone_core_set_presence_info() */
	LinphoneStatusAltService = 9, /**< Using another messaging service */
	LinphoneStatusPending = 10,   /**< Pending */
	LinphoneStatusVacation = 11,  /**< Vacation */

	LinphoneStatusEnd = 12
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
	LinphonePlayerClosed = 0, /**< No file is opened for playing. */
	LinphonePlayerPaused = 1, /**< The player is paused. */
	LinphonePlayerPlaying = 2 /**< The player is playing. */
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
	LinphonePresenceActivityAppointment = 0,

	/** The person is physically away from all interactive communication devices. */
	LinphonePresenceActivityAway = 1,

	/** The person is eating the first meal of the day, usually eaten in the morning. */
	LinphonePresenceActivityBreakfast = 2,

	/** The person is busy, without further details. */
	LinphonePresenceActivityBusy = 3,

	/** The person is having his or her main meal of the day, eaten in the evening or at midday. */
	LinphonePresenceActivityDinner = 4,

	/**  This is a scheduled national or local holiday. */
	LinphonePresenceActivityHoliday = 5,

	/** The person is riding in a vehicle, such as a car, but not steering. */
	LinphonePresenceActivityInTransit = 6,

	/** The person is looking for (paid) work. */
	LinphonePresenceActivityLookingForWork = 7,

	/** The person is eating his or her midday meal. */
	LinphonePresenceActivityLunch = 8,

	/** The person is scheduled for a meal, without specifying whether it is breakfast, lunch, or dinner,
	 *  or some other meal. */
	LinphonePresenceActivityMeal = 9,

	/** The person is in an assembly or gathering of people, as for a business, social, or religious purpose.
	 *  A meeting is a sub-class of an appointment. */
	LinphonePresenceActivityMeeting = 10,

	/** The person is talking on the telephone. */
	LinphonePresenceActivityOnThePhone = 11,

	/** The person is engaged in an activity with no defined representation. A string describing the activity
	 *  in plain text SHOULD be provided. */
	LinphonePresenceActivityOther = 12,

	/** A performance is a sub-class of an appointment and includes musical, theatrical, and cinematic
	 *  performances as well as lectures. It is distinguished from a meeting by the fact that the person
	 *  may either be lecturing or be in the audience, with a potentially large number of other people,
	 *  making interruptions particularly noticeable. */
	LinphonePresenceActivityPerformance = 13,

	/** The person will not return for the foreseeable future, e.g., because it is no longer working for
	 *  the company. */
	LinphonePresenceActivityPermanentAbsence = 14,

	/** The person is occupying himself or herself in amusement, sport, or other recreation. */
	LinphonePresenceActivityPlaying = 15,

	/** The person is giving a presentation, lecture, or participating in a formal round-table discussion. */
	LinphonePresenceActivityPresentation = 16,

	/** The person is visiting stores in search of goods or services. */
	LinphonePresenceActivityShopping = 17,

	/** The person is sleeping.*/
	LinphonePresenceActivitySleeping = 18,

	/** The person is observing an event, such as a sports event. */
	LinphonePresenceActivitySpectator = 19,

	/** The person is controlling a vehicle, watercraft, or plane. */
	LinphonePresenceActivitySteering = 20,

	/** The person is on a business or personal trip, but not necessarily in-transit. */
	LinphonePresenceActivityTravel = 21,

	/** The person is watching television. */
	LinphonePresenceActivityTV = 22,

	/** The activity of the person is unknown. */
	LinphonePresenceActivityUnknown = 23,

	/** A period of time devoted to pleasure, rest, or relaxation. */
	LinphonePresenceActivityVacation = 24,

	/** The person is engaged in, typically paid, labor, as part of a profession or job. */
	LinphonePresenceActivityWorking = 25,

	/** The person is participating in religious rites. */
	LinphonePresenceActivityWorship = 26
} LinphonePresenceActivityType;

/**
 * @brief Basic status as defined in section 4.1.4 of RFC 3863
 * @ingroup buddy_list
 */
typedef enum LinphonePresenceBasicStatus {
	/** This value means that the associated contact element, if any, is ready to accept communication. */
	LinphonePresenceBasicStatusOpen = 0,

	/** This value means that the associated contact element, if any, is unable to accept communication. */
	LinphonePresenceBasicStatusClosed = 1
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
typedef enum _LinphonePublishState {
	LinphonePublishNone = 0,             /**< Initial state, do not use */
	LinphonePublishIncomingReceived = 1, /**< An incoming publish is received */
	LinphonePublishOk = 2,               /**< Publish is accepted */
	LinphonePublishError = 3, /**< Publish encoutered an error, linphone_event_get_reason() gives reason code */
	LinphonePublishExpiring =
	    4, /**< Publish is about to expire, only sent if [sip]->refresh_generic_publish property is set to 0 */
	LinphonePublishCleared = 5,          /**< Event has been un published */
	LinphonePublishTerminating = 6,      /**< Publish is about to terminate */
	LinphonePublishOutgoingProgress = 7, /**< An outgoing publish was created and submitted */
	LinphonePublishRefreshing = 8        /**< Publish is about to refresh */
} LinphonePublishState;

/**
 * @brief Enum describing various failure reasons or contextual information for some events.
 * @see linphone_call_get_reason()
 * @see linphone_proxy_config_get_error()
 * @see linphone_error_info_get_reason()
 * @ingroup misc
 **/
typedef enum _LinphoneReason {
	LinphoneReasonNone = 0,               /**< No reason has been set by the core */
	LinphoneReasonNoResponse = 1,         /**< No response received from remote */
	LinphoneReasonForbidden = 2,          /**< Authentication failed due to bad credentials or resource forbidden */
	LinphoneReasonDeclined = 3,           /**< The call has been declined */
	LinphoneReasonNotFound = 4,           /**< Destination of the call was not found */
	LinphoneReasonNotAnswered = 5,        /**< The call was not answered in time (request timeout) */
	LinphoneReasonBusy = 6,               /**< Phone line was busy */
	LinphoneReasonUnsupportedContent = 7, /**< Unsupported content */
	LinphoneReasonBadEvent = 8,           /**< Bad event */
	LinphoneReasonIOError = 9,            /**< Transport error: connection failures, disconnections etc... */
	LinphoneReasonDoNotDisturb = 10,      /**< Do not disturb reason */
	LinphoneReasonUnauthorized = 11,      /**< Operation is unauthorized because missing credential */
	LinphoneReasonNotAcceptable = 12, /**< Operation is rejected due to incompatible or unsupported media parameters */
	LinphoneReasonNoMatch = 13,       /**< Operation could not be executed by server or remote client because it didn't
	                                     have any context for it */
	LinphoneReasonMovedPermanently = 14,         /**< Resource moved permanently */
	LinphoneReasonGone = 15,                     /**< Resource no longer exists */
	LinphoneReasonTemporarilyUnavailable = 16,   /**< Temporarily unavailable */
	LinphoneReasonAddressIncomplete = 17,        /**< Address incomplete */
	LinphoneReasonNotImplemented = 18,           /**< Not implemented */
	LinphoneReasonBadGateway = 19,               /**< Bad gateway */
	LinphoneReasonSessionIntervalTooSmall = 20,  /**< The received request contains a Session-Expires header field
	                                                with a duration below the minimum timer */
	LinphoneReasonServerTimeout = 21,            /**< Server timeout */
	LinphoneReasonUnknown = 22,                  /**< Unknown reason */
	LinphoneReasonTransferred = 23,              /**< The call has been transferred */
	LinphoneReasonConditionalRequestFailed = 24, /**< Conditional Request Failed */
	LinphoneReasonSasCheckRequired = 25
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
	int udp_port;  /**< SIP/UDP port */
	int tcp_port;  /**< SIP/TCP port */
	int dtls_port; /**< SIP/DTLS port */
	int tls_port;  /**< SIP/TLS port */
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
	LinphoneStreamTypeAudio = 0,
	LinphoneStreamTypeVideo = 1,
	LinphoneStreamTypeText = 2,
	LinphoneStreamTypeUnknown = 3 /* WARNING: Make sure this value remains the last one in the list */
} LinphoneStreamType;

/**
 * @brief Enum controlling behavior for incoming subscription request.
 * Use by linphone_friend_set_inc_subscribe_policy()
 * @ingroup buddy_list
 */
typedef enum _LinphoneSubscribePolicy {
	/**
	 * Does not automatically accept an incoming subscription request.
	 * This policy implies that a decision has to be taken for each incoming subscription request notified by callback
	 * LinphoneCoreVTable.new_subscription_requested
	 */
	LinphoneSPWait = 0,
	LinphoneSPDeny = 1,  /**< Rejects incoming subscription request */
	LinphoneSPAccept = 2 /**< Automatically accepts a subscription request */
} LinphoneSubscribePolicy;

/**
 * @brief Enum for subscription direction (incoming or outgoing).
 * @ingroup event_api
 **/
typedef enum _LinphoneSubscriptionDir {
	LinphoneSubscriptionIncoming = 0,  /**< Incoming subscription. */
	LinphoneSubscriptionOutgoing = 1,  /**< Outgoing subscription. */
	LinphoneSubscriptionInvalidDir = 2 /**< Invalid subscription direction. */
} LinphoneSubscriptionDir;

/**
 * @brief Enum for subscription states.
 * #LinphoneSubscriptionTerminated and #LinphoneSubscriptionError are final states.
 * @ingroup event_api
 **/
typedef enum _LinphoneSubscriptionState {
	LinphoneSubscriptionNone = 0,             /**< Initial state, should not be used */
	LinphoneSubscriptionOutgoingProgress = 1, /**< An outgoing subcription was sent */
	LinphoneSubscriptionIncomingReceived = 2, /**< An incoming subcription is received */
	LinphoneSubscriptionPending = 3,          /**< Subscription is pending, waiting for user approval */
	LinphoneSubscriptionActive = 4,           /**< Subscription is accepted */
	LinphoneSubscriptionTerminated = 5,       /**< Subscription is terminated normally */
	LinphoneSubscriptionError =
	    6, /**< Subscription was terminated by an error, indicated by linphone_event_get_reason() */
	LinphoneSubscriptionExpiring =
	    7, /**< Subscription is about to expire, only sent if [sip]->refresh_generic_subscribe property is set to 0 */
} LinphoneSubscriptionState;

/**
 * @brief Enum listing frequent telephony tones.
 * @ingroup misc
 **/
typedef enum _LinphoneToneID {
	LinphoneToneUndefined = 0,       /**< Not a tone */
	LinphoneToneBusy = 1,            /**< Busy tone */
	LinphoneToneCallWaiting = 2,     /**< Call waiting tone */
	LinphoneToneCallOnHold = 3,      /**< Call on hold tone */
	LinphoneToneCallLost = 4,        /**< Tone played when call is abruptly disconnected (media lost)*/
	LinphoneToneCallEnd = 5,         /**< When the call end for any reason but lost */
	LinphoneToneCallNotAnswered = 6, /**< When the call is not answered */
	LinphoneToneSasCheckRequired = 7 /**< When the SAS check is required */
} LinphoneToneID;

/**
 * @brief Enum describing transport type for LinphoneAddress.
 * @ingroup linphone_address
 **/
typedef enum _LinphoneTransportType {
	LinphoneTransportUdp = 0,
	LinphoneTransportTcp = 1,
	LinphoneTransportTls = 2,
	LinphoneTransportDtls = 3
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
	LinphoneTunnelModeDisable = 0, /**< The tunnel is disabled */
	LinphoneTunnelModeEnable = 1,  /**< The tunnel is enabled */
	LinphoneTunnelModeAuto = 2     /**< The tunnel is enabled automatically if it is required */
} LinphoneTunnelMode;

/**
 * @brief Enum describing uPnP states.
 * @ingroup initializing
 **/
typedef enum _LinphoneUpnpState {
	LinphoneUpnpStateIdle = 0,         /**< uPnP is not activate */
	LinphoneUpnpStatePending = 1,      /**< uPnP process is in progress */
	LinphoneUpnpStateAdding = 2,       /**< Internal use: Only used by port binding */
	LinphoneUpnpStateRemoving = 3,     /**< Internal use: Only used by port binding */
	LinphoneUpnpStateNotAvailable = 4, /**< uPnP is not available */
	LinphoneUpnpStateOk = 5,           /**< uPnP is enabled */
	LinphoneUpnpStateKo = 6,           /**< uPnP processing has failed */
	LinphoneUpnpStateBlacklisted = 7,  /**< IGD router is blacklisted */
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
	LinphoneVersionUpdateCheckUpToDate = 0,
	LinphoneVersionUpdateCheckNewVersionAvailable = 1,
	LinphoneVersionUpdateCheckError = 2
} LinphoneVersionUpdateCheckResult;

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
	LinphoneWaitingStart = 0,
	LinphoneWaitingProgress = 1,
	LinphoneWaitingFinished = 2
} LinphoneWaitingState;

/**
 * @brief Enum describing the types of argument for LinphoneXmlRpcRequest.
 * @ingroup misc
 **/
typedef enum _LinphoneXmlRpcArgType {
	LinphoneXmlRpcArgNone = 0,
	LinphoneXmlRpcArgInt = 1,
	LinphoneXmlRpcArgString = 2,
	LinphoneXmlRpcArgStringStruct = 3
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
	LinphoneXmlRpcStatusPending = 0,
	LinphoneXmlRpcStatusOk = 1,
	LinphoneXmlRpcStatusFailed = 2
} LinphoneXmlRpcStatus;

typedef struct _LsdPlayer LsdPlayer;

/**
 * @brief Structure describing a range of integers
 * @ingroup misc
 */
typedef struct _LinphoneRange LinphoneRange;

/**
 * @brief Status code returned by some functions to
 * notify whether the execution has been successfully
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

/**
 * @brief Enum describing the search categories for Magic Search
 * @ingroup buddy_list
 **/
typedef enum _LinphoneMagicSearchSource {
	LinphoneMagicSearchSourceNone = 0, /**< no Source specified. If requested in search, the list should be empty */
	LinphoneMagicSearchSourceFriends = 1 << 0,     /**< Search in friends only */
	LinphoneMagicSearchSourceCallLogs = 1 << 1,    /**< Search in Call Logs */
	LinphoneMagicSearchSourceLdapServers = 1 << 2, /**< Search in LDAP servers */
	LinphoneMagicSearchSourceChatRooms = 1 << 3,   /**< Search in Chat rooms participants */
	LinphoneMagicSearchSourceRequest =
	    1 << 4, /**< Search from request : it is usually an address built from the request */
	LinphoneMagicSearchSourceFavoriteFriends = 1 << 5, /**< Search in "starred" friends only */
	LinphoneMagicSearchSourceConferencesInfo = 1 << 6, /**< Search in conferences info (organizer and participants) */
	LinphoneMagicSearchSourceRemoteCardDAV =
	    1 << 7,                       /**< Search in remote CardDAV servers (not locally synched ones) if any */
	LinphoneMagicSearchSourceAll = -1 /**< Search in all sources */
} LinphoneMagicSearchSource;

/**
 * @brief Enum describing the type of #RemoteContactDirectory (currently CardDAV or LDAP).
 * @ingroup buddy_list
 **/
typedef enum _LinphoneRemoteContactDirectoryType {
	LinphoneRemoteContactDirectoryTypeCardDav = 0, /**< Remote contact directory will use #CardDavParams */
	LinphoneRemoteContactDirectoryTypeLdap = 1,    /**< Remote contact directory will use #LdapParams */
} LinphoneRemoteContactDirectoryType;

/**
 * @brief Enum describing how to merge #LinphoneSearchResult from #LinphoneMagicSearch
 * @ingroup buddy_list
 **/
typedef enum _LinphoneMagicSearchAggregation {
	LinphoneMagicSearchAggregationNone =
	    0, /**< No aggregation is done, you can have multiple SearchResult with the same Friend */
	LinphoneMagicSearchAggregationFriend =
	    1 /**< Aggregation is done by friend, you will have at most a SearchResult per Friend */
} LinphoneMagicSearchAggregation;

/**
 * @brief Enum describing the different contexts for the #LinphoneMessageWaitingIndicationSummary.
 * @ingroup account
 */
typedef enum _LinphoneMessageWaitingIndicationContextClass {
	LinphoneMessageWaitingIndicationVoice = 0,
	LinphoneMessageWaitingIndicationFax = 1,
	LinphoneMessageWaitingIndicationPager = 2,
	LinphoneMessageWaitingIndicationMultimedia = 3,
	LinphoneMessageWaitingIndicationText = 4,
	LinphoneMessageWaitingIndicationNone = 5,
} LinphoneMessageWaitingIndicationContextClass;

/**
 * Object representing a Message Waiting Indication.
 * @ingroup account
 */
typedef struct _LinphoneMessageWaitingIndication LinphoneMessageWaitingIndication;

/**
 * Object representing the summary for a context in a Message Waiting Indication.
 * @ingroup account
 */
typedef struct _LinphoneMessageWaitingIndicationSummary LinphoneMessageWaitingIndicationSummary;

/**
 * #LinphoneBaudotMode enum represents the Baudot mode to use for the call.
 * @ingroup call_control
 */
typedef enum _LinphoneBaudotMode {
	LinphoneBaudotModeVoice,            /**< Send and receive audio. */
	LinphoneBaudotModeTty,              /**< Send and receive Baudot tones. */
	LinphoneBaudotModeHearingCarryOver, /**< Send Baudot tones, but receive audio. */
	LinphoneBaudotModeVoiceCarryOver,   /**< Send audio, but receive Baudot tones. */
} LinphoneBaudotMode;

/**
 * #LinphoneBaudotStandard enum represents the Baudot standard to use to send Baudot tones in the call.
 * @ingroup call_control
 */
typedef enum _LinphoneBaudotStandard {
	LinphoneBaudotStandardUs,     /**< Send 45.45 baud US Baudot tones. */
	LinphoneBaudotStandardEurope, /**< Send 50 baud European Baudot tones. */
} LinphoneBaudotStandard;

#endif /* LINPHONE_TYPES_H_ */
