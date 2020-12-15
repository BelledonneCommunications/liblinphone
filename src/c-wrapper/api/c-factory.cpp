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

#include "linphone/api/c-factory.h"
#include "factory/factory.h"
#include "c-wrapper/c-wrapper.h"

using namespace LinphonePrivate;

LinphoneFactory *linphone_factory_get(void) {
  return Factory::get().get()->toC();
}

void linphone_factory_clean(void){
	Factory::clean();
}

LinphoneCore *linphone_factory_create_core (
	const LinphoneFactory *factory,
	LinphoneCoreCbs *cbs,
	const char *config_path,
	const char *factory_config_path
) {

  return Factory::toCpp(factory)->createCore(
    cbs,
    config_path ? config_path : "",
    factory_config_path ? factory_config_path : ""
  );

}

LinphoneCore *linphone_factory_create_core_2 (
	const LinphoneFactory *factory,
	LinphoneCoreCbs *cbs,
	const char *config_path,
	const char *factory_config_path,
	void *user_data,
	void *system_context
) {
  return Factory::toCpp(factory)->createCore(
    cbs,
    config_path ? config_path : "",
    factory_config_path ? factory_config_path : "",
    user_data,
    system_context
  );

}

LinphoneCore *linphone_factory_create_core_3 (
	const LinphoneFactory *factory,
	const char *config_path,
	const char *factory_config_path,
	void *system_context
) {
  return Factory::toCpp(factory)->createCore(
    config_path ? config_path : "",
    factory_config_path ? factory_config_path : "",
    system_context
  );

}

LinphoneCore *linphone_factory_create_shared_core (
	const LinphoneFactory *factory,
	const char *config_filename,
	const char *factory_config_path,
	void *system_context,
	const char *app_group_id,
	bool_t main_core
) {

  return Factory::toCpp(factory)->createSharedCore(
    config_filename ? config_filename : "",
    factory_config_path ? factory_config_path : "",
    system_context,
    app_group_id ? app_group_id : "",
    main_core
  );

}

LinphoneCore *linphone_factory_create_core_with_config (
	const LinphoneFactory *factory,
	LinphoneCoreCbs *cbs,
	LinphoneConfig *config
) {
  return Factory::toCpp(factory)->createCoreWithConfig(cbs, config);
}

LinphoneCore *linphone_factory_create_core_with_config_2 (
	const LinphoneFactory *factory,
	LinphoneCoreCbs *cbs,
	LinphoneConfig *config,
	void *user_data,
	void *system_context
) {
  return Factory::toCpp(factory)->createCoreWithConfig(cbs, config, user_data, system_context);
}

LinphoneCore *linphone_factory_create_core_with_config_3 (
	const LinphoneFactory *factory,
	LinphoneConfig *config,
	void *system_context
) {
  return Factory::toCpp(factory)->createCoreWithConfig(config, system_context);
}

LinphoneCore *linphone_factory_create_shared_core_with_config (
	const LinphoneFactory *factory,
	LinphoneConfig *config,
	void *system_context,
	const char *app_group_id,
	bool_t main_core
) {
  return Factory::toCpp(factory)->createSharedCoreWithConfig(config, system_context, app_group_id ? app_group_id : "", main_core);

}

LinphoneCoreCbs *linphone_factory_create_core_cbs(const LinphoneFactory *factory) {
  return Factory::toCpp(factory)->createCoreCbs();
}

LinphoneAddress *linphone_factory_create_address(const LinphoneFactory *factory, const char *addr) {
  return Factory::toCpp(factory)->createAddress(addr ? addr : "");
}

LinphoneParticipantDeviceIdentity *linphone_factory_create_participant_device_identity(
	const LinphoneFactory *factory,
	const LinphoneAddress *address,
	const char *name
) {
  return Factory::toCpp(factory)->createParticipantDeviceIdentity(
    address,
    name ? name : "");
}

LinphoneAuthInfo *linphone_factory_create_auth_info(const LinphoneFactory *factory, const char *username, const char *userid, const char *passwd, const char *ha1, const char *realm, const char *domain) {
  return Factory::toCpp(factory)->createAuthInfo(
    username ? username : "",
    userid ? userid : "",
    passwd ? passwd : "",
    ha1 ? ha1 : "",
    realm ? realm : "",
    domain ? domain : ""
  );
}

LinphoneAuthInfo *linphone_factory_create_auth_info_2(const LinphoneFactory *factory, const char *username, const char *userid, const char *passwd, const char *ha1, const char *realm, const char *domain, const char *algorithm) {
  return Factory::toCpp(factory)->createAuthInfo(
    username ? username : "",
    userid ? userid : "",
    passwd ? passwd : "",
    ha1 ? ha1 : "",
    realm ? realm : "",
    domain ? domain : "",
    algorithm ? algorithm : ""
  );

}

LinphoneCallCbs * linphone_factory_create_call_cbs(const LinphoneFactory *factory) {
  return Factory::toCpp(factory)->createCallCbs();
}

LinphoneConferenceCbs * linphone_factory_create_conference_cbs(const LinphoneFactory *factory) {
  return Factory::toCpp(factory)->createConferenceCbs();
}

LinphoneChatRoomCbs * linphone_factory_create_chat_room_cbs(const LinphoneFactory *factory) {
  return Factory::toCpp(factory)->createChatRoomCbs();
}

LinphoneChatMessageCbs * linphone_factory_create_chat_message_cbs(const LinphoneFactory *factory) {
  return Factory::toCpp(factory)->createChatMessageCbs();
}

LinphoneMagicSearchCbs * linphone_factory_create_magic_search_cbs(const LinphoneFactory *factory){
     return Factory::toCpp(factory)->createMagicSearchCbs();
}

LinphoneVcard *linphone_factory_create_vcard(LinphoneFactory *factory) {
  return Factory::toCpp(factory)->createVcard();
}

LinphoneVideoDefinition * linphone_factory_create_video_definition(const LinphoneFactory *factory, unsigned int width, unsigned int height) {
  return Factory::toCpp(factory)->createVideoDefinition(width, height);

}

LinphoneVideoDefinition * linphone_factory_create_video_definition_from_name(const LinphoneFactory *factory, const char *name) {
  return Factory::toCpp(factory)->createVideoDefinitionFromName(
    name ? name : ""
  );
}

const bctbx_list_t * linphone_factory_get_supported_video_definitions(const LinphoneFactory *factory) {
  return Factory::toCpp(factory)->getSupportedVideoDefinitions();
}

LinphoneVideoDefinition * linphone_factory_find_supported_video_definition(const LinphoneFactory *factory, unsigned int width, unsigned int height) {
  return Factory::toCpp(factory)->findSupportedVideoDefinition(width, height);
}

LinphoneVideoDefinition * linphone_factory_find_supported_video_definition_by_name(const LinphoneFactory *factory, const char *name) {
  return Factory::toCpp(factory)->findSupportedVideoDefinitionByName(
    name ? name : ""
  );
}

const char * linphone_factory_get_top_resources_dir(const LinphoneFactory *factory) {
	return Factory::nullifyEmptyString(Factory::toCpp(factory)->getTopResourcesDir());
}

void linphone_factory_set_top_resources_dir(LinphoneFactory *factory, const char *path) {
  Factory::toCpp(factory)->setTopResourcesDir(
    path ? path : ""
  );
}

const char * linphone_factory_get_data_resources_dir(LinphoneFactory *factory) {
  return Factory::nullifyEmptyString(Factory::toCpp(factory)->getDataResourcesDir());
}

void linphone_factory_set_data_resources_dir(LinphoneFactory *factory, const char *path) {
  Factory::toCpp(factory)->setDataResourcesDir(
    path ? path : ""
  );

}

const char * linphone_factory_get_sound_resources_dir(LinphoneFactory *factory) {
  return Factory::nullifyEmptyString(Factory::toCpp(factory)->getSoundResourcesDir());
}

void linphone_factory_set_sound_resources_dir(LinphoneFactory *factory, const char *path) {
  Factory::toCpp(factory)->setSoundResourcesDir(
    path ? path : ""
  );

}

const char * linphone_factory_get_ring_resources_dir(LinphoneFactory *factory) {
  return Factory::nullifyEmptyString(Factory::toCpp(factory)->getRingResourcesDir());
}

void linphone_factory_set_ring_resources_dir(LinphoneFactory *factory, const char *path) {
  Factory::toCpp(factory)->setRingResourcesDir(
    path ? path : ""
  );

}

const char * linphone_factory_get_image_resources_dir(LinphoneFactory *factory) {
  return Factory::nullifyEmptyString(Factory::toCpp(factory)->getImageResourcesDir());
}

void linphone_factory_set_image_resources_dir(LinphoneFactory *factory, const char *path) {
    Factory::toCpp(factory)->setImageResourcesDir(
      path ? path : ""
    );
}

const char * linphone_factory_get_msplugins_dir(LinphoneFactory *factory) {
  return Factory::nullifyEmptyString(Factory::toCpp(factory)->getMspluginsDir());
}

void linphone_factory_set_msplugins_dir(LinphoneFactory *factory, const char *path) {
  Factory::toCpp(factory)->setMspluginsDir(
    path ? path : ""
  );
}

LinphoneErrorInfo *linphone_factory_create_error_info(LinphoneFactory *factory){
  return Factory::toCpp(factory)->createErrorInfo();
}

LinphoneRange *linphone_factory_create_range(LinphoneFactory *factory) {
  return Factory::toCpp(factory)->createRange();
}

LinphoneTransports *linphone_factory_create_transports(LinphoneFactory *factory) {
  return Factory::toCpp(factory)->createTransports();
}

LinphoneVideoActivationPolicy *linphone_factory_create_video_activation_policy(LinphoneFactory *factory) {
  return Factory::toCpp(factory)->createVideoActivationPolicy();
}

LinphoneContent *linphone_factory_create_content(LinphoneFactory *factory) {
  return Factory::toCpp(factory)->createContent();
}

LinphoneBuffer *linphone_factory_create_buffer(LinphoneFactory *factory) {
  return Factory::toCpp(factory)->createBuffer();
}

LinphoneBuffer *linphone_factory_create_buffer_from_data(LinphoneFactory *factory, const uint8_t *data, size_t size) {
  return Factory::toCpp(factory)->createBufferFromData(data, size);

}

LinphoneBuffer *linphone_factory_create_buffer_from_string(LinphoneFactory *factory, const char *data) {
  return Factory::toCpp(factory)->createBufferFromString(
    data ? data : ""
  );
}

LinphoneConfig *linphone_factory_create_config(LinphoneFactory *factory, const char *path) {
  return Factory::toCpp(factory)->createConfig(
    path ? path : ""
  );
}

LinphoneConfig *linphone_factory_create_config_with_factory(LinphoneFactory *factory, const char *path, const char *factory_path) {
  return Factory::toCpp(factory)->createConfigWithFactory(
    path ? path : "",
    factory_path ? factory_path : ""
  );
}

LinphoneConfig *linphone_factory_create_config_from_string(LinphoneFactory *factory, const char *data) {
  return Factory::toCpp(factory)->createConfigFromString(
    data ? data : ""
  );
}

const bctbx_list_t * linphone_factory_get_dial_plans(const LinphoneFactory *factory) {
  return Factory::toCpp(factory)->getDialPlans();
}

void *linphone_factory_get_user_data(const LinphoneFactory *factory) {
  return Factory::toCpp(factory)->getUserData();
}

void linphone_factory_set_user_data(LinphoneFactory *factory, void *data) {
  return Factory::toCpp(factory)->setUserData(data);
}

void linphone_factory_set_log_collection_path(LinphoneFactory *factory, const char *path) {
  return Factory::toCpp(factory)->setLogCollectionPath(
    path ? path : ""
  );
}

void linphone_factory_enable_log_collection(LinphoneFactory *factory, LinphoneLogCollectionState state) {
  return Factory::toCpp(factory)->enableLogCollection(state);
}

LinphoneTunnelConfig *linphone_factory_create_tunnel_config(LinphoneFactory *factory) {
  return Factory::toCpp(factory)->createTunnelConfig();
}

LinphoneAccountCbs * linphone_factory_create_account_cbs(LinphoneFactory *factory) {
  return Factory::toCpp(factory)->createAccountCbs();
}

LinphoneLoggingServiceCbs *linphone_factory_create_logging_service_cbs(LinphoneFactory *factory) {
  return Factory::toCpp(factory)->createLoggingServiceCbs();
}

LinphonePlayerCbs *linphone_factory_create_player_cbs(LinphoneFactory *factory) {
  return Factory::toCpp(factory)->createPlayerCbs();
}

LinphoneEventCbs *linphone_factory_create_event_cbs(LinphoneFactory *factory) {
  return Factory::toCpp(factory)->createEventCbs();
}

LinphoneFriendListCbs *linphone_factory_create_friend_list_cbs(LinphoneFactory *factory) {
  return Factory::toCpp(factory)->createFriendListCbs();
}

LinphoneAccountCreatorCbs *linphone_factory_create_account_creator_cbs(LinphoneFactory *factory) {
  return Factory::toCpp(factory)->createAccountCreatorCbs();
}

LinphoneXmlRpcRequestCbs *linphone_factory_create_xml_rpc_request_cbs(LinphoneFactory *factory) {
  return Factory::toCpp(factory)->createXmlRpcRequestCbs();
}

bool_t linphone_factory_is_chatroom_backend_available(LinphoneFactory *factory, LinphoneChatRoomBackend chatroom_backend) {
  return Factory::toCpp(factory)->isChatroomBackendAvailable(chatroom_backend);
}

bool_t linphone_factory_is_database_storage_available(LinphoneFactory *factory) {
  return Factory::toCpp(factory)->isDatabaseStorageAvailable();
}

bool_t linphone_factory_is_imdn_available(LinphoneFactory *factory) {
  return Factory::toCpp(factory)->isImdnAvailable();
}

const char *linphone_factory_get_config_dir(LinphoneFactory *factory, void *context) {
  return Factory::nullifyEmptyString(Factory::toCpp(factory)->getConfigDir(context));
}

const char *linphone_factory_get_data_dir(LinphoneFactory *factory, void *context) {
  return Factory::nullifyEmptyString(Factory::toCpp(factory)->getDataDir(context));
}

const char *linphone_factory_get_download_dir(LinphoneFactory *factory, void *context) {
  return Factory::nullifyEmptyString(Factory::toCpp(factory)->getDownloadDir(context));
}

void linphone_factory_set_vfs_encryption(LinphoneFactory *factory, const uint16_t encryptionModule, const uint8_t *secret, const size_t secretSize) {
  return Factory::toCpp(factory)->setVfsEncryption(
    encryptionModule,
    secret,
    secretSize
  );
}
