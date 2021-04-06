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

#ifndef LINPHONE_FACTORY_H
#define LINPHONE_FACTORY_H

#include "linphone/api/c-types.h"
#include "linphone/types.h"
#include "linphone/logging.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup initializing
 * @{
 */

/**
 * Create the #LinphoneFactory if that has not been done and return a pointer on it.
 * @return A pointer on the #LinphoneFactory @notnil
 */
LINPHONE_PUBLIC LinphoneFactory *linphone_factory_get(void);

/**
 * Clean the factory. This function is generally useless as the factory is unique per process, however
 * calling this function at the end avoid getting reports from belle-sip leak detector about memory leaked in linphone_factory_get().
 */
LINPHONE_PUBLIC void linphone_factory_clean(void);

/**
 * Instantiate a #LinphoneCore object.
 *
 * The #LinphoneCore object is the primary handle for doing all phone actions. It should be unique within your
 * application.
 * The #LinphoneCore object is not started automatically, you need to call linphone_core_start() to that effect.
 * The returned #LinphoneCore will be in #LinphoneGlobalState Ready.
 * Core ressources can be released using linphone_core_stop() which is strongly encouraged on garbage collected languages.
 * @param factory The #LinphoneFactory singleton. @notnil
 * @param config_path A path to a config file. If it does not exists it will be created. The config file is used to
 * store all settings, proxies... so that all these settings become persistent over the life of the #LinphoneCore object.
 * It is allowed to set a NULL config file. In that case #LinphoneCore will not store any settings. @maybenil
 * @param factory_config_path A path to a read-only config file that can be used to store hard-coded preferences
 * such as proxy settings or internal preferences. The settings in this factory file always override the ones in the
 * normal config file. It is optional, use NULL if unneeded. @maybenil
 * @param system_context A pointer to a system object required by the core to operate. Currently it is required to
 * pass an android Context on android, pass NULL on other platforms. @maybenil
 * @return a #LinphoneCore object @notnil
 * @see linphone_core_new_with_config_3()
 */
LINPHONE_PUBLIC LinphoneCore *linphone_factory_create_core_3 (
	const LinphoneFactory *factory,
	const char *config_path,
	const char *factory_config_path,
	void *system_context
);

/**
 * Instantiate a shared #LinphoneCore object.
 *
 * The shared #LinphoneCore allow you to create several #LinphoneCore with the same config.
 * Two #LinphoneCore can't run at the same time.
 *
 * A shared #LinphoneCore can be a "Main Core" or an "Executor Core".
 * A "Main Core" automatically stops a running "Executor Core" when calling linphone_core_start()
 * An "Executor Core" can't start unless no other #LinphoneCore is started. It can be stopped by a "Main Core" and
 * switch to #LinphoneGlobalState Off at any time.
 *
 * Shared Executor Core are used in iOS UNNotificationServiceExtension to receive new messages from push notifications.
 * When the application is in background, its Shared Main Core is stopped.
 *
 * The #LinphoneCore object is not started automatically, you need to call linphone_core_start() to that effect.
 * The returned #LinphoneCore will be in #LinphoneGlobalState Ready.
 * Core ressources can be released using linphone_core_stop() which is strongly encouraged on garbage collected languages.
 * @param factory The #LinphoneFactory singleton. @notnil
 * @param config_filename The name of the config file. If it does not exists it will be created.
 * Its path is computed using the app_group_id. The config file is used to
 * store all settings, proxies... so that all these settings become persistent over the life of the #LinphoneCore object.
 * It is allowed to set a NULL config file. In that case #LinphoneCore will not store any settings. @maybenil
 * @param factory_config_path A path to a read-only config file that can be used to store hard-coded preferences
 * such as proxy settings or internal preferences. The settings in this factory file always override the ones in the
 * normal config file. It is optional, use NULL if unneeded. @maybenil
 * @param system_context A pointer to a system object required by the core to operate. Currently it is required to
 * pass an android Context on android, pass NULL on other platforms. @maybenil
 * @param app_group_id Name of iOS App Group that lead to the file system that is shared between an app and its app extensions. @notnil
 * @param main_core Indicate if we want to create a "Main Core" or an "Executor Core".
 * @return a #LinphoneCore object @notnil
 * @see linphone_factory_create_shared_core_with_config()
 */
LINPHONE_PUBLIC LinphoneCore *linphone_factory_create_shared_core (
	const LinphoneFactory *factory,
	const char *config_filename,
	const char *factory_config_path,
	void *system_context,
	const char *app_group_id,
	bool_t main_core
);

/**
 * Instantiate a #LinphoneCore object with a given LinphoneConfig.
 *
 * The #LinphoneCore object is the primary handle for doing all phone actions. It should be unique within your
 * application.
 * The #LinphoneCore object is not started automatically, you need to call linphone_core_start() to that effect.
 * The returned #LinphoneCore will be in #LinphoneGlobalState Ready.
 * Core ressources can be released using linphone_core_stop() which is strongly encouraged on garbage collected languages.
 * @param factory The #LinphoneFactory singleton. @notnil
 * @param config A #LinphoneConfig object holding the configuration for the #LinphoneCore to be instantiated. @notnil
 * @param system_context A pointer to a system object required by the core to operate. Currently it is required to
 * pass an android Context on android, pass NULL on other platforms. @maybenil
 * @return a #LinphoneCore object @notnil
 * @see linphone_factory_create_core_3()
 */
LINPHONE_PUBLIC LinphoneCore *linphone_factory_create_core_with_config_3 (
	const LinphoneFactory *factory,
	LinphoneConfig *config,
	void *system_context
);

/**
 * Instantiate a shared #LinphoneCore object.
 *
 * The shared #LinphoneCore allow you to create several #LinphoneCore with the same config.
 * Two #LinphoneCore can't run at the same time.
 *
 * A shared #LinphoneCore can be a "Main Core" or an "Executor Core".
 * A "Main Core" automatically stops a running "Executor Core" when calling linphone_core_start()
 * An "Executor Core" can't start unless no other #LinphoneCore is started. It can be stopped by a "Main Core" and
 * switch to #LinphoneGlobalState Off at any time.
 *
 * Shared Executor Core are used in iOS UNNotificationServiceExtension to receive new messages from push notifications.
 * When the application is in background, its Shared Main Core is stopped.
 *
 * The #LinphoneCore object is not started automatically, you need to call linphone_core_start() to that effect.
 * The returned #LinphoneCore will be in #LinphoneGlobalState Ready.
 * Core ressources can be released using linphone_core_stop() which is strongly encouraged on garbage collected languages.
 * @param factory The #LinphoneFactory singleton. @notnil
 * @param config A #LinphoneConfig object holding the configuration for the #LinphoneCore to be instantiated. @notnil
 * @param system_context A pointer to a system object required by the core to operate. Currently it is required to
 * pass an android Context on android, pass NULL on other platforms. @maybenil
 * @param app_group_id Name of iOS App Group that lead to the file system that is shared between an app and its app extensions. @notnil
 * @param main_core Indicate if we want to create a "Main Core" or an "Executor Core".
 * @return a #LinphoneCore object @notnil
 * @see linphone_factory_create_shared_core()
 */
LINPHONE_PUBLIC LinphoneCore *linphone_factory_create_shared_core_with_config (
	const LinphoneFactory *factory,
	LinphoneConfig *config,
	void *system_context,
	const char *app_group_id,
	bool_t main_core
);

/**
 * Instanciate a #LinphoneCoreCbs object.
 * @param factory The #LinphoneFactory singleton. @notnil
 * @return a new #LinphoneCoreCbs. @notnil
 */
LINPHONE_PUBLIC LinphoneCoreCbs *linphone_factory_create_core_cbs(const LinphoneFactory *factory);

/**
 * Parse a string holding a SIP URI and create the according #LinphoneAddress object.
 * @param factory The #LinphoneFactory singleton. @notnil
 * @param addr A string holding the SIP URI to parse. @notnil
 * @return A new #LinphoneAddress. @maybenil
 */
LINPHONE_PUBLIC LinphoneAddress *linphone_factory_create_address(const LinphoneFactory *factory, const char *addr);

/**
 * Create a #LinphoneParticipantDeviceIdentity object.
 * @param factory The #LinphoneFactory singleton. @notnil
 * @param address #LinphoneAddress object. @notnil
 * @param name the name given to the device. @maybenil
 * @return A new #LinphoneParticipantDeviceIdentity. @notnil
 */
LINPHONE_PUBLIC LinphoneParticipantDeviceIdentity *linphone_factory_create_participant_device_identity(
	const LinphoneFactory *factory,
	const LinphoneAddress *address,
	const char *name
);

/**
 * Creates a #LinphoneAuthInfo object.
 * The object can be created empty, that is with all arguments set to NULL.
 * Username, userid, password, realm and domain can be set later using specific methods.
 * At the end, username and passwd (or ha1) are required.
 * @param factory The #LinphoneFactory singleton. @notnil
 * @param username The username that needs to be authenticated @notnil
 * @param userid The userid used for authenticating (use NULL if you don't know what it is) @maybenil
 * @param passwd The password in clear text @maybenil
 * @param ha1 The ha1-encrypted password if password is not given in clear text. @maybenil
 * @param realm The authentication domain (which can be larger than the sip domain. Unfortunately many SIP servers don't use this parameter. @maybenil
 * @param domain The SIP domain for which this authentication information is valid, if it has to be restricted for a single SIP domain. @maybenil
 * @return A #LinphoneAuthInfo object. linphone_auth_info_destroy() must be used to destroy it when no longer needed. The #LinphoneCore makes a copy of #LinphoneAuthInfo
 * passed through linphone_core_add_auth_info(). @notnil
 */
LINPHONE_PUBLIC LinphoneAuthInfo *linphone_factory_create_auth_info(const LinphoneFactory *factory, const char *username, const char *userid, const char *passwd, const char *ha1, const char *realm, const char *domain);

/**
 * Creates a #LinphoneAuthInfo object.
 * The object can be created empty, that is with all arguments set to NULL.
 * Username, userid, password, realm and domain can be set later using specific methods.
 * At the end, username and passwd (or ha1) are required.
 * @param factory The #LinphoneFactory singleton. @notnil
 * @param username The username that needs to be authenticated @notnil
 * @param userid The userid used for authenticating (use NULL if you don't know what it is) @maybenil
 * @param passwd The password in clear text @maybenil
 * @param ha1 The ha1-encrypted password if password is not given in clear text. @maybenil
 * @param realm The authentication domain (which can be larger than the sip domain. Unfortunately many SIP servers don't use this parameter. @maybenil
 * @param domain The SIP domain for which this authentication information is valid, if it has to be restricted for a single SIP domain. @maybenil
 * @param algorithm The algorithm for encrypting password. @maybenil
 * @return A #LinphoneAuthInfo object. linphone_auth_info_destroy() must be used to destroy it when no longer needed. The #LinphoneCore makes a copy of #LinphoneAuthInfo
 * passed through linphone_core_add_auth_info(). @notnil
 */
LINPHONE_PUBLIC LinphoneAuthInfo *linphone_factory_create_auth_info_2(const LinphoneFactory *factory, const char *username, const char *userid, const char *passwd, const char *ha1, const char *realm, const char *domain, const char *algorithm);

/**
 * Create a #LinphoneCallCbs object that holds callbacks for events happening on a call.
 * @param factory #LinphoneFactory singletion object @notnil
 * @return A new #LinphoneCallCbs object @notnil
 */
LINPHONE_PUBLIC LinphoneCallCbs * linphone_factory_create_call_cbs(const LinphoneFactory *factory);

/**
 * Create a #LinphoneConferenceCbs object that holds callbacks for events happening on a conference.
 * @param[in] factory #LinphoneFactory singletion object @notnil
 * @return A new #LinphoneConferenceCbs object @notnil
 */
LINPHONE_PUBLIC LinphoneConferenceCbs * linphone_factory_create_conference_cbs(const LinphoneFactory *factory);

/**
 * Create a LinphoneChatRoomCbs object that holds callbacks for events happening on a chat room.
 * @param factory #LinphoneFactory singletion object @notnil
 * @return A new #LinphoneChatRoomCbs object @notnil
 */
LINPHONE_PUBLIC LinphoneChatRoomCbs * linphone_factory_create_chat_room_cbs(const LinphoneFactory *factory);

/**
 * Create a LinphoneChatMessageCbs object that holds callbacks for events happening on a chat message.
 * @param factory #LinphoneFactory singletion object @notnil
 * @return A new #LinphoneChatMessageCbs object @notnil
 */
LINPHONE_PUBLIC LinphoneChatMessageCbs * linphone_factory_create_chat_message_cbs(const LinphoneFactory *factory);

/**
 * Create an empty #LinphoneVcard.
 * @param factory #LinphoneFactory singletion object @notnil
 * @return a new #LinphoneVcard. @notnil
 * @ingroup initializing
 */
LINPHONE_PUBLIC LinphoneVcard *linphone_factory_create_vcard(LinphoneFactory *factory);

/**
 * Create a #LinphoneVideoDefinition from a given width and height
 * @param factory #LinphoneFactory singleton object @notnil
 * @param width The width of the created video definition
 * @param height The height of the created video definition
 * @return A new #LinphoneVideoDefinition object @notnil
 */
LINPHONE_PUBLIC LinphoneVideoDefinition * linphone_factory_create_video_definition(const LinphoneFactory *factory, unsigned int width, unsigned int height);

/**
 * Create a #LinphoneVideoDefinition from a given standard definition name
 * @param factory #LinphoneFactory singleton object @notnil
 * @param name The standard definition name of the video definition to create @notnil
 * @return A new #LinphoneVideoDefinition object @notnil
 */
LINPHONE_PUBLIC LinphoneVideoDefinition * linphone_factory_create_video_definition_from_name(const LinphoneFactory *factory, const char *name);

/**
 * Get the list of standard video definitions supported by Linphone.
 * @param factory #LinphoneFactory singleton object @notnil
 * @return A list of video definitions. \bctbx_list{LinphoneVideoDefinition} @maybenil
 */
LINPHONE_PUBLIC const bctbx_list_t * linphone_factory_get_supported_video_definitions(const LinphoneFactory *factory);

/**
 * Get the top directory where the resources are located.
 * @param factory #LinphoneFactory object @notnil
 * @return The path to the top directory where the resources are located @notnil
 */
LINPHONE_PUBLIC const char * linphone_factory_get_top_resources_dir(const LinphoneFactory *factory);

/**
 * Set the top directory where the resources are located.
 * If you only define this top directory, the other resources directory will automatically be derived form this one.
 * @param factory #LinphoneFactory object @notnil
 * @param path The path to the top directory where the resources are located @notnil
 */
LINPHONE_PUBLIC void linphone_factory_set_top_resources_dir(LinphoneFactory *factory, const char *path);

/**
 * Get the directory where the data resources are located.
 * @param factory #LinphoneFactory object @notnil
 * @return The path to the directory where the data resources are located @notnil
 */
LINPHONE_PUBLIC const char * linphone_factory_get_data_resources_dir(LinphoneFactory *factory);

/**
 * Set the directory where the data resources are located.
 * @param factory #LinphoneFactory object @notnil
 * @param path The path where the data resources are located @notnil
 */
LINPHONE_PUBLIC void linphone_factory_set_data_resources_dir(LinphoneFactory *factory, const char *path);

/**
 * Get the directory where the sound resources are located.
 * @param factory #LinphoneFactory object @notnil
 * @return The path to the directory where the sound resources are located @notnil
 */
LINPHONE_PUBLIC const char * linphone_factory_get_sound_resources_dir(LinphoneFactory *factory);

/**
 * Set the directory where the sound resources are located.
 * @param factory #LinphoneFactory object @notnil
 * @param path The path where the sound resources are located @notnil
 */
LINPHONE_PUBLIC void linphone_factory_set_sound_resources_dir(LinphoneFactory *factory, const char *path);

/**
 * Get the directory where the ring resources are located.
 * @param factory #LinphoneFactory object @notnil
 * @return The path to the directory where the ring resources are located @notnil
 */
LINPHONE_PUBLIC const char * linphone_factory_get_ring_resources_dir(LinphoneFactory *factory);

/**
 * Set the directory where the ring resources are located.
 * @param factory #LinphoneFactory object @notnil
 * @param path The path where the ring resources are located @notnil
 */
LINPHONE_PUBLIC void linphone_factory_set_ring_resources_dir(LinphoneFactory *factory, const char *path);

/**
 * Get the directory where the image resources are located.
 * @param factory #LinphoneFactory object @notnil
 * @return The path to the directory where the image resources are located @notnil
 */
LINPHONE_PUBLIC const char * linphone_factory_get_image_resources_dir(LinphoneFactory *factory);

/**
 * Set the directory where the image resources are located.
 * @param factory #LinphoneFactory object @notnil
 * @param path The path where the image resources are located @notnil
 */
LINPHONE_PUBLIC void linphone_factory_set_image_resources_dir(LinphoneFactory *factory, const char *path);

/**
 * Get the directory where the mediastreamer2 plugins are located.
 * @param factory #LinphoneFactory object @notnil
 * @return The path to the directory where the mediastreamer2 plugins are located, or NULL if it has not been set. @maybenil
 */
LINPHONE_PUBLIC const char * linphone_factory_get_msplugins_dir(LinphoneFactory *factory);

/**
 * Set the directory where the mediastreamer2 plugins are located.
 * @param factory #LinphoneFactory object @notnil
 * @param path The path to the directory where the mediastreamer2 plugins are located @maybenil
 */
LINPHONE_PUBLIC void linphone_factory_set_msplugins_dir(LinphoneFactory *factory, const char *path);

/**
 * Creates an object LinphoneErrorInfo.
 * @param factory #LinphoneFactory object @notnil
 * @return a #LinphoneErrorInfo object. @notnil
 */
LINPHONE_PUBLIC  LinphoneErrorInfo *linphone_factory_create_error_info(LinphoneFactory *factory);

/**
 * Creates an object LinphoneRange.
 * @param factory #LinphoneFactory object @notnil
 * @return a #LinphoneRange object. @notnil
 */
LINPHONE_PUBLIC LinphoneRange *linphone_factory_create_range(LinphoneFactory *factory);

/**
 * Creates an object LinphoneTransports.
 * @param factory #LinphoneFactory object @notnil
 * @return a #LinphoneTransports object. @notnil
 */
LINPHONE_PUBLIC LinphoneTransports *linphone_factory_create_transports(LinphoneFactory *factory);

/**
 * Creates an object LinphoneVideoActivationPolicy.
 * @param factory #LinphoneFactory object @notnil
 * @return  #LinphoneVideoActivationPolicy object. @notnil
 */
LINPHONE_PUBLIC LinphoneVideoActivationPolicy *linphone_factory_create_video_activation_policy(LinphoneFactory *factory);

/**
 * Returns a bctbx_list_t of all DialPlans
 * @param factory the #LinphoneFactory object @notnil
 * @return A list of #LinphoneDialPlan \bctbx_list{LinphoneDialPlan} @notnil
 */
LINPHONE_PUBLIC const bctbx_list_t * linphone_factory_get_dial_plans(const LinphoneFactory *factory);

/**
 * Creates an object #LinphoneContent
 * @param factory the #LinphoneFactory @notnil
 * @return a #LinphoneContent @notnil
 */
LINPHONE_PUBLIC LinphoneContent *linphone_factory_create_content(LinphoneFactory *factory);

/**
 * Creates an object #LinphoneBuffer
 * @param factory the #LinphoneFactory @notnil
 * @return a #LinphoneBuffer @notnil
 */
LINPHONE_PUBLIC LinphoneBuffer *linphone_factory_create_buffer(LinphoneFactory *factory);

/**
 * Creates an object #LinphoneBuffer
 * @param factory the #LinphoneFactory @notnil
 * @param data the data to set in the buffer @notnil
 * @param size the size of the data
 * @return a #LinphoneBuffer @notnil
 */
LINPHONE_PUBLIC LinphoneBuffer *linphone_factory_create_buffer_from_data(LinphoneFactory *factory, const uint8_t *data, size_t size);

/**
 * Creates an object #LinphoneBuffer
 * @param factory the #LinphoneFactory @notnil
 * @param data the data to set in the buffer @notnil
 * @return a #LinphoneBuffer @notnil
 */
LINPHONE_PUBLIC LinphoneBuffer *linphone_factory_create_buffer_from_string(LinphoneFactory *factory, const char *data);

/**
 * Creates an object #LinphoneConfig
 * @param factory the #LinphoneFactory @notnil
 * @param path the path of the config @maybenil
 * @return a #LinphoneConfig @notnil
 */
LINPHONE_PUBLIC LinphoneConfig *linphone_factory_create_config(LinphoneFactory *factory, const char *path);

/**
 * Creates an object #LinphoneConfig
 * @param factory the #LinphoneFactory @notnil
 * @param path the path of the config @maybenil
 * @param factory_path the path of the factory @maybenil
 * @return a #LinphoneConfig @notnil
 */
LINPHONE_PUBLIC LinphoneConfig *linphone_factory_create_config_with_factory(LinphoneFactory *factory, const char *path, const char *factory_path);

/**
 * Creates an object #LinphoneConfig
 * @param factory the #LinphoneFactory @notnil
 * @param data the config data @notnil
 * @return a #LinphoneConfig @notnil
 */
LINPHONE_PUBLIC LinphoneConfig *linphone_factory_create_config_from_string(LinphoneFactory *factory, const char *data);

/**
 * Gets the user data in the #LinphoneFactory object
 * @param factory the #LinphoneFactory @notnil
 * @return the user data. @maybenil
*/
LINPHONE_PUBLIC void *linphone_factory_get_user_data(const LinphoneFactory *factory);

/**
 * Sets the user data in the #LinphoneFactory object
 * @param factory the #LinphoneFactory object @notnil
 * @param data the user data. @maybenil
*/
LINPHONE_PUBLIC void linphone_factory_set_user_data(LinphoneFactory *factory, void *data);

/**
 * Sets the log collection path
 * @param factory the #LinphoneFactory @notnil
 * @param path the path of the logs @maybenil
 */
LINPHONE_PUBLIC void linphone_factory_set_log_collection_path(LinphoneFactory *factory, const char *path);

/**
 * Enables or disables log collection
 * @param factory the #LinphoneFactory @notnil
 * @param state the #LinphoneLogCollectionState for log collection
 */
LINPHONE_PUBLIC void linphone_factory_enable_log_collection(LinphoneFactory *factory, LinphoneLogCollectionState state);

/**
 * Creates an object #LinphoneTunnelConfig
 * @param factory the #LinphoneFactory @notnil
 * @return a #LinphoneTunnelConfig @notnil
 */
LINPHONE_PUBLIC LinphoneTunnelConfig *linphone_factory_create_tunnel_config(LinphoneFactory *factory);

/**
 * Creates an object #LinphoneAccountCbs
 * @param factory the #LinphoneFactory @notnil
 * @return a #LinphoneAccountCbs @notnil
 */
LINPHONE_PUBLIC LinphoneAccountCbs *linphone_factory_create_account_cbs(LinphoneFactory *factory);

/**
 * Creates an object #LinphoneLoggingServiceCbs
 * @param factory the #LinphoneFactory @notnil
 * @return a #LinphoneLoggingServiceCbs @notnil
 */
LINPHONE_PUBLIC LinphoneLoggingServiceCbs *linphone_factory_create_logging_service_cbs(LinphoneFactory *factory);

/**
 * Creates an object #LinphonePlayerCbs
 * @param factory the #LinphoneFactory @notnil
 * @return a #LinphonePlayerCbs @notnil
 */
LINPHONE_PUBLIC LinphonePlayerCbs *linphone_factory_create_player_cbs(LinphoneFactory *factory);

/**
 * Creates an object #LinphoneEventCbs
 * @param factory the #LinphoneFactory @notnil
 * @return a #LinphoneEventCbs @notnil
 */
LINPHONE_PUBLIC LinphoneEventCbs *linphone_factory_create_event_cbs(LinphoneFactory *factory);

/**
 * Creates an object #LinphoneFriendListCbs
 * @param factory the #LinphoneFactory @notnil
 * @return a #LinphoneFriendListCbs @notnil
 */
LINPHONE_PUBLIC LinphoneFriendListCbs *linphone_factory_create_friend_list_cbs(LinphoneFactory *factory);

/**
 * Creates an object #LinphoneAccountCreatorCbs
 * @param factory the #LinphoneFactory @notnil
 * @return a #LinphoneAccountCreatorCbs @notnil
 */
LINPHONE_PUBLIC LinphoneAccountCreatorCbs *linphone_factory_create_account_creator_cbs(LinphoneFactory *factory);

/**
 * Creates an object #LinphoneXmlRpcRequestCbs
 * @param factory the #LinphoneFactory @notnil
 * @return a #LinphoneXmlRpcRequestCbs @notnil
 */
LINPHONE_PUBLIC LinphoneXmlRpcRequestCbs *linphone_factory_create_xml_rpc_request_cbs(LinphoneFactory *factory);

/**
 * Indicates if the given LinphoneChatRoomBackend is available
 * @param factory the #LinphoneFactory @notnil
 * @param chatroom_backend the #LinphoneChatRoomBackend
 * @return TRUE if the chatroom backend is available, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_factory_is_chatroom_backend_available(LinphoneFactory *factory, LinphoneChatRoomBackend chatroom_backend);

/**
 * Indicates if the storage in database is available
 * @param factory the #LinphoneFactory @notnil
 * @return TRUE if the database storage is available, FALSE otherwise
 */
LINPHONE_PUBLIC bool_t linphone_factory_is_database_storage_available(LinphoneFactory *factory);

/**
 * Indicates if IMDN are available
 * @param factory the #LinphoneFactory @notnil
 * @return TRUE if IDMN are available
 */
LINPHONE_PUBLIC bool_t linphone_factory_is_imdn_available(LinphoneFactory *factory);

/**
 * Get the config path
 * @param factory the #LinphoneFactory @notnil
 * @param context used to compute path. Can be NULL. JavaPlatformHelper on Android and char *appGroupId on iOS with shared core. @maybenil
 * @return The config path @notnil
 * @ingroup misc
**/
LINPHONE_PUBLIC const char *linphone_factory_get_config_dir(LinphoneFactory *factory, void *context);

/**
 * Get the data path
 * @param factory the #LinphoneFactory @notnil
 * @param context used to compute path. Can be NULL. JavaPlatformHelper on Android and char *appGroupId on iOS with shared core. @maybenil
 * @return The data path @notnil
 * @ingroup misc
**/
LINPHONE_PUBLIC const char *linphone_factory_get_data_dir(LinphoneFactory *factory, void *context);

/**
 * Get the download path
 * @param factory the #LinphoneFactory @notnil
 * @param context used to compute path. Can be NULL. JavaPlatformHelper on Android and char *appGroupId on iOS with shared core. @maybenil
 * @return The download path @notnil
 * @ingroup misc
**/
LINPHONE_PUBLIC const char *linphone_factory_get_download_dir(LinphoneFactory *factory, void *context);

/************ */
/* DEPRECATED */
/* ********** */

/**
 * Instanciate a #LinphoneCore object.
 *
 * The #LinphoneCore object is the primary handle for doing all phone actions.
 * It should be unique within your application.
 * @param factory The #LinphoneFactory singleton.
 * @param cbs a #LinphoneCoreCbs object holding your application callbacks. A reference
 * will be taken on it until the destruciton of the core or the unregistration
 * with linphone_core_remove_cbs().
 * @param config_path a path to a config file. If it does not exists it will be created.
 *        The config file is used to store all settings, call logs, friends, proxies... so that all these settings
 *	       become persistent over the life of the LinphoneCore object.
 *	       It is allowed to set a NULL config file. In that case LinphoneCore will not store any settings.
 * @param factory_config_path a path to a read-only config file that can be used to
 *        to store hard-coded preference such as proxy settings or internal preferences.
 *        The settings in this factory file always override the one in the normal config file.
 *        It is OPTIONAL, use NULL if unneeded.
 * @see linphone_core_new_with_config()
 * @deprecated 2018-01-10: Use linphone_factory_create_core_3() instead
 * @donotwrap
 */
LINPHONE_DEPRECATED LINPHONE_PUBLIC LinphoneCore *linphone_factory_create_core(
	const LinphoneFactory *factory,
	LinphoneCoreCbs *cbs,
	const char *config_path,
	const char *factory_config_path
);

/**
 * Instanciate a #LinphoneCore object.
 *
 * The #LinphoneCore object is the primary handle for doing all phone actions.
 * It should be unique within your application.
 * @param factory The #LinphoneFactory singleton.
 * @param cbs a #LinphoneCoreCbs object holding your application callbacks. A reference
 * will be taken on it until the destruciton of the core or the unregistration
 * with linphone_core_remove_cbs().
 * @param config_path a path to a config file. If it does not exists it will be created.
 *        The config file is used to store all settings, call logs, friends, proxies... so that all these settings
 *	       become persistent over the life of the LinphoneCore object.
 *	       It is allowed to set a NULL config file. In that case LinphoneCore will not store any settings.
 * @param factory_config_path a path to a read-only config file that can be used to
 *        to store hard-coded preference such as proxy settings or internal preferences.
 *        The settings in this factory file always override the one in the normal config file.
 *        It is OPTIONAL, use NULL if unneeded.
 * @param user_data an application pointer associated with the returned core.
 * @param system_context a pointer to a system object required by the core to operate. Currently it is required to pass an android Context on android, pass NULL on other platforms.
 * @see linphone_core_new_with_config()
 * @deprecated 2018-01-10: Use linphone_factory_create_core_3() instead
 * @donotwrap
 */
LINPHONE_DEPRECATED LINPHONE_PUBLIC LinphoneCore *linphone_factory_create_core_2 (
	const LinphoneFactory *factory,
	LinphoneCoreCbs *cbs,
	const char *config_path,
	const char *factory_config_path,
	void *user_data,
	void *system_context
);

/**
 * Instantiates a #LinphoneCore object with a given LpConfig.
 *
 * @param factory The #LinphoneFactory singleton.
 * The #LinphoneCore object is the primary handle for doing all phone actions.
 * It should be unique within your application.
 * @param cbs a #LinphoneCoreCbs object holding your application callbacks. A reference
 * will be taken on it until the destruciton of the core or the unregistration
 * with linphone_core_remove_cbs().
 * @param config a pointer to an LpConfig object holding the configuration of the #LinphoneCore to be instantiated.
 * @see linphone_core_new()
 * @deprecated 2018-01-10: Use linphone_factory_create_core_with_config_3() instead
 * @donotwrap
 */
LINPHONE_DEPRECATED LINPHONE_PUBLIC LinphoneCore *linphone_factory_create_core_with_config (
	const LinphoneFactory *factory,
	LinphoneCoreCbs *cbs,
	LinphoneConfig *config
);

/**
 * Instantiates a #LinphoneCore object with a given LpConfig.
 *
 * @param factory The #LinphoneFactory singleton.
 * The #LinphoneCore object is the primary handle for doing all phone actions.
 * It should be unique within your application.
 * @param cbs a #LinphoneCoreCbs object holding your application callbacks. A reference
 * will be taken on it until the destruciton of the core or the unregistration
 * with linphone_core_remove_cbs().
 * @param config a pointer to an LpConfig object holding the configuration of the #LinphoneCore to be instantiated.
 * @param user_data an application pointer associated with the returned core.
 * @param system_context a pointer to a system object required by the core to operate. Currently it is required to pass an android Context on android, pass NULL on other platforms.
 * @see linphone_core_new()
 * @deprecated 2018-01-10: Use linphone_factory_create_core_with_config_3() instead
 * @donotwrap
 */
LINPHONE_DEPRECATED LINPHONE_PUBLIC LinphoneCore *linphone_factory_create_core_with_config_2 (
	const LinphoneFactory *factory,
	LinphoneCoreCbs *cbs,
	LinphoneConfig *config,
	void *user_data,
	void *system_context
);

#define LINPHONE_VFS_ENCRYPTION_PLAIN 0xFFFF
#define LINPHONE_VFS_ENCRYPTION_UNSET 0x0000
#define LINPHONE_VFS_ENCRYPTION_DUMMY 0x0001
#define LINPHONE_VFS_ENCRYPTION_AES256GCM128_SHA256 0x0002
/**
 * Select encryption module and set secret material to encrypt the files
 * @param[in]	factory			the #LinphoneFactory @notnil
 * @param[in]	encryptionModule	One of the available encryption module for VFS, pick in the LINPHONE_VFS_ENCRYPTION_* list
 * 					if set to _UNSET, default bctoolbox VFS is switch to Standard one
 * @param[in]	secret			the secret material used to encrypt the files, can be NULL for the _PLAIN module @maybenil
 * @param[in]	secretSize		size of the secret
 */
LINPHONE_PUBLIC void linphone_factory_set_vfs_encryption(LinphoneFactory *factory, const uint16_t encryptionModule, const uint8_t *secret, const size_t secretSize);
/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif // LINPHONE_FACTORY_H
