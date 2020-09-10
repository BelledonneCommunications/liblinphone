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

#include "linphone/factory.h"

// pour call par exemple le .h est dans le même dossier
// que  faire ici ?
//#include "linphone/api/c-factory.h"

#include "c-wrapper/c-wrapper.h"

#include "address/address-p.h"
#include "core/paths/paths.h"
#include "bctoolbox/vfs_encrypted.hh"
#include "bctoolbox/crypto.h"
#include "sqlite3_bctbx_vfs.h"

// TODO: From coreapi. Remove me later.
#include "private.h"

#ifndef PACKAGE_SOUND_DIR
	#define PACKAGE_SOUND_DIR "."
#endif
#ifndef PACKAGE_RING_DIR
	#define PACKAGE_RING_DIR "."
#endif

#ifndef PACKAGE_DATA_DIR
	#define PACKAGE_DATA_DIR "."
#endif

LINPHONE_BEGIN_NAMESPACE

Factory::Factory(){
	mTopResourcesDir = bctbx_strdup(PACKAGE_DATA_DIR);
	initialize_supported_video_definitions(this);
	/* register the bctbx sqlite vfs. It is not used by default */
	/* sqlite3_bctbx_vfs use the default bctbx_vfs, so if encryption is turned on by default, it will apply to sqlte3 db */
	sqlite3_bctbx_vfs_register(0);
	mEvfsMasterKey = nullptr;
}

static LinphoneFactory* Factory::create(void){

	LinphoneFactory *factory = belle_sip_object_new(LinphoneFactory);
	factory->mTopResourcesDir = bctbx_strdup(PACKAGE_DATA_DIR);
	initialize_supported_video_definitions(factory);
	// register the bctbx sqlite vfs. It is not used by default
	// sqlite3_bctbx_vfs use the default bctbx_vfs, so if encryption is turned on by default, it will apply to sqlte3 db //
	sqlite3_bctbx_vfs_register(0);
	factory->mEvfsMasterKey = nullptr;
	return factory;

}

static void Factory::uninit(Factory *obj){
	bctbx_list_free_with_data(obj->mSupportedVideoDefinitions, (bctbx_list_free_func)linphone_video_definition_unref);

	//STRING_RESET(obj->top_resources_dir);
	//STRING_RESET(obj->data_resources_dir);
	//STRING_RESET(obj->sound_resources_dir);
	//STRING_RESET(obj->ring_resources_dir);
	//STRING_RESET(obj->image_resources_dir);
	//STRING_RESET(obj->msplugins_dir);

	//STRING_RESET(obj->cached_data_resources_dir);
	//STRING_RESET(obj->cached_sound_resources_dir);
	//STRING_RESET(obj->cached_ring_resources_dir);
	//STRING_RESET(obj->cached_image_resources_dir);
	//STRING_RESET(obj->cached_msplugins_dir);

	//STRING_RESET(obj->cached_config_dir);
	//STRING_RESET(obj->cached_data_dir);
	//STRING_RESET(obj->cached_download_dir);

	// sqlite3 vfs is registered at factory creation, so unregister it when destroying it
	sqlite3_bctbx_vfs_unregister();

	// proper cleaning of EVFS master key if any is set
	if (obj->mEvfsMasterKey != nullptr) {
		bctbx_clean(obj->mEvfsMasterKey->data(), obj->mEvfsMasterKey->size());
		obj->mEvfsMasterKey = nullptr;
	}
}

//BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneFactory);

//useless ???
/*BELLE_SIP_INSTANCIATE_VPTR(LinphoneFactory, belle_sip_object_t,
	linphone_factory_uninit, // destroy
	NULL, // clone
	NULL, // Marshall
	FALSE
);*/

static Factory *_factory = NULL;

static void _DestroyingCb(void) {
	if (_factory != NULL) {
		belle_sip_object_unref(_factory);
		_factory = NULL;
}

#define ADD_SUPPORTED_VIDEO_DEFINITION(factory, width, height, name) \
	(factory)->supported_video_definitions = bctbx_list_append((factory)->supported_video_definitions, \
		linphone_video_definition_new(width, height, name))

static void Factory::initializeSupportedVideoDefinitions(Factory *factory) {
#if !defined(__ANDROID__) && !TARGET_OS_IPHONE
	ADD_SUPPORTED_VIDEO_DEFINITION(factory, MS_VIDEO_SIZE_1080P_W, MS_VIDEO_SIZE_1080P_H, "1080p");
#endif
#if !defined(__ANDROID__) && !TARGET_OS_MAC /*limit to most common sizes because mac video API cannot list supported resolutions*/
	ADD_SUPPORTED_VIDEO_DEFINITION(factory, MS_VIDEO_SIZE_UXGA_W, MS_VIDEO_SIZE_UXGA_H, "uxga");
	ADD_SUPPORTED_VIDEO_DEFINITION(factory, MS_VIDEO_SIZE_SXGA_MINUS_W, MS_VIDEO_SIZE_SXGA_MINUS_H, "sxga-");
#endif
	ADD_SUPPORTED_VIDEO_DEFINITION(factory, MS_VIDEO_SIZE_720P_W, MS_VIDEO_SIZE_720P_H, "720p");
#if !defined(__ANDROID__) && !TARGET_OS_MAC
	ADD_SUPPORTED_VIDEO_DEFINITION(factory, MS_VIDEO_SIZE_XGA_W, MS_VIDEO_SIZE_XGA_H, "xga");
#endif
#if !defined(__ANDROID__) && !TARGET_OS_IPHONE
	ADD_SUPPORTED_VIDEO_DEFINITION(factory, MS_VIDEO_SIZE_SVGA_W, MS_VIDEO_SIZE_SVGA_H, "svga");
	ADD_SUPPORTED_VIDEO_DEFINITION(factory, MS_VIDEO_SIZE_4CIF_W, MS_VIDEO_SIZE_4CIF_H, "4cif");
#endif
	ADD_SUPPORTED_VIDEO_DEFINITION(factory, MS_VIDEO_SIZE_VGA_W, MS_VIDEO_SIZE_VGA_H, "vga");
#if TARGET_OS_IPHONE
	ADD_SUPPORTED_VIDEO_DEFINITION(factory, MS_VIDEO_SIZE_IOS_MEDIUM_H, MS_VIDEO_SIZE_IOS_MEDIUM_W, "ios-medium");
#endif
	ADD_SUPPORTED_VIDEO_DEFINITION(factory, MS_VIDEO_SIZE_CIF_W, MS_VIDEO_SIZE_CIF_H, "cif");
#if !TARGET_OS_MAC || TARGET_OS_IPHONE /* OS_MAC is 1 for iPhone, but we need QVGA */
	ADD_SUPPORTED_VIDEO_DEFINITION(factory, MS_VIDEO_SIZE_QVGA_W, MS_VIDEO_SIZE_QVGA_H, "qvga");
#endif
	ADD_SUPPORTED_VIDEO_DEFINITION(factory, MS_VIDEO_SIZE_QCIF_W, MS_VIDEO_SIZE_QCIF_H, "qcif");
}




static Factory* Factory::get(void) {
	if (_factory == NULL) {
		_factory = linphone_factory_new();
		atexit(_linphone_factory_destroying_cb);
	}
	return _factory;
}

static void Factory::clean(void){
	LinphonePrivate::AddressPrivate::clearSipAddressesCache();
	if (_factory){
		belle_sip_object_unref(_factory);
		_factory = NULL;
	}
}

LinphoneCore *_createCore (
	LinphoneCoreCbs *cbs,
	const char *config_path,
	const char *factory_config_path,
	void *user_data,
	void *system_context,
	bool_t automatically_start
) {
	bctbx_init_logger(FALSE);
	LpConfig *config = linphone_config_new_with_factory(config_path, factory_config_path);
	LinphoneCore *lc = _linphone_core_new_with_config(cbs, config, user_data, system_context, automatically_start);
	linphone_config_unref(config);
	bctbx_uninit_logger();
	return lc;
}

LinphoneCore *_createSharedCore (
	LinphoneCoreCbs *cbs,
	const char *config_filename,
	const char *factory_config_path,
	void *user_data,
	void *system_context,
	bool_t automatically_start,
	const char *app_group_id,
	bool_t main_core
) {
	bctbx_init_logger(FALSE);
	LpConfig *config = linphone_config_new_for_shared_core(app_group_id, config_filename, factory_config_path);
	LinphoneCore *lc = _linphone_core_new_shared_with_config(cbs, config, user_data, system_context, automatically_start, app_group_id, main_core);
	linphone_config_unref(config);
	bctbx_uninit_logger();
	return lc;
}

LinphoneCore* Factory::createCore (
	LinphoneCoreCbs *cbs,
	const char *config_path,
	const char *factory_config_path
) {
	return Factory::_createCore(cbs, config_path, factory_config_path, NULL, NULL, TRUE);
}

LinphoneCore* Factory::createCore (
	LinphoneCoreCbs *cbs,
	const char *config_path,
	const char *factory_config_path,
	void *user_data,
	void *system_context
) {
	return Factory::_createCore(cbs, config_path, factory_config_path, user_data, system_context, TRUE);
}

LinphoneCore* Factory::createCore (
	const char *config_path,
	const char *factory_config_path,
	void *system_context
) {
	return Factory::_createCore(NULL, config_path, factory_config_path, NULL, system_context, FALSE);
}

LinphoneCore* Factory::createSharedCore (
	const char *config_filename,
	const char *factory_config_path,
	void *system_context,
	const char *app_group_id,
	bool_t main_core
) {
	return Factory::_CreateSharedCore(NULL, factory_config_path, factory_config_path, NULL, system_context, FALSE, app_group_id, main_core);
}

LinphoneCore* Factory::createCoreWithConfig (
	LinphoneCoreCbs *cbs,
	LinphoneConfig *config
) {
	return _linphone_core_new_with_config(cbs, config, NULL, NULL, TRUE);
}

LinphoneCore* Factory::createCoreWithConfig (
	LinphoneCoreCbs *cbs,
	LinphoneConfig *config,
	void *user_data,
	void *system_context
) {
	return _linphone_core_new_with_config(cbs, config, user_data, system_context, TRUE);
}

LinphoneCore* Factory::createCoreWithConfig (
	LinphoneConfig *config,
	void *system_context
) {
	return _linphone_core_new_with_config(NULL, config, NULL, system_context, FALSE);
}

LinphoneCore* Factory::createSharedCoreWithConfig (
	LinphoneConfig *config,
	void *system_context,
	const char *app_group_id,
	bool_t main_core
) {
	return _linphone_core_new_shared_with_config(NULL, config, NULL, system_context, FALSE, app_group_id, main_core);
}

LinphoneCoreCbs* Factory::createCoreCbs() {
	return _linphone_core_cbs_new();
}

LinphoneAddress* Factory::createAddress(const char *addr) {
	return linphone_address_new(addr);
}

static LinphoneParticipantDeviceIdentity* Factory::createParticipantDeviceIdentity(
	const LinphoneAddress *address,
	const char *name
) {
#ifdef HAVE_ADVANCED_IM
	return linphone_participant_device_identity_new(address, name);
#else
	ms_warning("Advanced IM such as group chat is disabled");
	return NULL;
#endif
}

LinphoneAuthInfo* Factory::createAuthInfo(const char *username, const char *userid, const char *passwd, const char *ha1, const char *realm, const char *domain) {
	return linphone_auth_info_new(username, userid, passwd, ha1, realm, domain);
}

LinphoneAuthInfo* Factory::createAuthInfo(const char *username, const char *userid, const char *passwd, const char *ha1, const char *realm, const char *domain, const char *algorithm) {
	return linphone_auth_info_new_for_algorithm(username, userid, passwd, ha1, realm, domain, algorithm);
}

LinphoneCallCbs* Factory::createCallCbs() {
	return _linphone_call_cbs_new();
}

LinphoneChatRoomCbs* Factory::createChatRoomCbs() {
	return _linphone_chat_room_cbs_new();
}

LinphoneChatMessageCbs* Factory::createChatMessageCbs() {
	return linphone_chat_message_cbs_new();
}

LinphoneVcard* Factory::createVcard() {
	return _linphone_vcard_new();
}

LinphoneVideoDefinition* Factory::createVideoDefinition(unsigned int width, unsigned int height) {
	LinphoneVideoDefinition *supported = Factory::findSupportedVideoDefinition(this, width, height);
	return supported ? linphone_video_definition_clone(supported) : linphone_video_definition_new(width, height, NULL);
}

LinphoneVideoDefinition* Factory::createVideoDefinitionFromName(const char *name) {
	unsigned int width = 0;
	unsigned int height = 0;
	LinphoneVideoDefinition *vdef = Factory::findSupportedVideoDefinitionByName(this, name);
	if (vdef != NULL) return vdef;
	if (sscanf(name, "%ux%u", &width, &height) == 2) {
		return linphone_video_definition_new(width, height, NULL);
	}
	return linphone_video_definition_new(0, 0, NULL);
}

const bctbx_list_t* Factory::getSupportedVideoDefinitions() {
	return mSupportedVideoDefinitions;
}

LinphoneVideoDefinition* Factory::findSupportedVideoDefinition(unsigned int width, unsigned int height) {
	const bctbx_list_t *item;
	const bctbx_list_t *supported = Factory::getSupportedVideoDefinitions(this);
	LinphoneVideoDefinition *searched_vdef = linphone_video_definition_new(width, height, NULL);
	LinphoneVideoDefinition *found = NULL;

	for (item = supported; item != NULL; item = bctbx_list_next(item)) {
		LinphoneVideoDefinition *svdef = (LinphoneVideoDefinition *)bctbx_list_get_data(item);
		if (linphone_video_definition_equals(svdef, searched_vdef)) {
			found = svdef;
			break;
		}
	}
	linphone_video_definition_unref(searched_vdef);

	if (!found) {
		ms_warning("Couldn't find supported video definition for %ux%u", width, height);
	}
	return found;
}

LinphoneVideoDefinition* Factory::findSupportedVideoDefinitionByName(const char *name) {
	const bctbx_list_t *item;
	const bctbx_list_t *supported = Factory::getSupportedVideoDefinitions(this);

	for (item = supported; item != NULL; item = bctbx_list_next(item)) {
		LinphoneVideoDefinition *svdef = (LinphoneVideoDefinition *)bctbx_list_get_data(item);
		if (strcmp(linphone_video_definition_get_name(svdef), name) == 0) {
			return svdef;
		}
	}

	ms_error("Couldn't find a supported video definition for name [%s]", name);
	return NULL;
}

const char* Factory::getTopResourcesDir() {
	return mTopResourcesDir;
}

void Factory::setTopResourcesDir(const char *path) {
	mTopResourcesDir = path);
}

const char* Factory::getDataResourcesDir() {
	if (mDataResourcesDir) return mDataResourcesDir;
	if (mTopResourcesDir){
		mCachedDataResourcesDir = mTopResourcesDir + "/linphone";
	}else{
		mCachedDataResourcesDir = PACKAGE_DATA_DIR + "/linphone";
	}
	return mCachedDataResourcesDir;
}

void Factory::setDataResourcesDir(const char *path) {
	mDataResourcesDir = path;
}

const char* Factory::getSoundResourcesDir() {
	if (mSoundResourcesDir) return mSoundResourcesDir;
	if (mTopResourcesDir){
		mcachedSoundResourcesDir = mTopResourcesDir + "/sounds/linphone";
		return mcachedSoundResourcesDir;
	}
	return PACKAGE_SOUND_DIR;
}

void Factory::setSoundResourcesDir(const char *path) {
	mSoundResourcesDir = path;
}

const char* Factory::getRingResourcesDir() {
	if (mRingResourcesDir) return mRingResourcesDir;
	if (mSoundResourcesDir){
		mcachedRingResourcesDir = mSoundResourcesDir + "/rings";
		return mcachedRingResourcesDir;
	}
	if (mTopResourcesDir) {
		mcachedRingResourcesDir = mTopResourcesDir + "/sounds/linphone/rings";
		return mcachedRingResourcesDir;
	}
	return PACKAGE_RING_DIR;
}

void Factory::setRingResourcesDir(const char *path) {
	mRingResourcesDir = path;
}

const char* Factory::getImageResourcesDir() {
	if (mImageResourcesDir) return mImageResourcesDir;
	if (mTopResourcesDir) {
		mcachedImageResourcesDir = mTopResourcesDir + "/images";
	}else{
		mcachedImageResourcesDir = PACKAGE_DATA_DIR + "/images";
	}
	return mcachedImageResourcesDir;
}

void Factory::setImageResourcesDir(const char *path) {
	mImageResourcesDir = path;
}

const char* Factory::getMspluginsDir() {
	return mMspluginsDir;
}

void Factory::setMspluginsDir(const char *path) {
	mMspluginsDir = path;
}

LinphoneErrorInfo* Factory::createErrorInfo(){

	return linphone_error_info_new();

}

LinphoneRange* Factory::createRange() {
	return linphone_range_new();
}

LinphoneTransports* Factory::createTransports() {
	return linphone_transports_new();
}

LinphoneVideoActivationPolicy* Factory::createVideoActivationPolicy() {
	return linphone_video_activation_policy_new();
}

LinphoneContent* Factory::createContent() {
	return linphone_content_new();
}

LinphoneBuffer* Factory::createBuffer() {
	return linphone_buffer_new();
}

LinphoneBuffer* Factory::createBufferFromData(const uint8_t *data, size_t size) {
	return linphone_buffer_new_from_data(data, size);
}

LinphoneBuffer* Factory::createbufferFromString(const char *data) {
	return linphone_buffer_new_from_string(data);
}

LinphoneConfig* Factory::createConfig(const char *path) {
	return linphone_config_new(path);
}

LinphoneConfig* Factory::createConfigWithFactory(const char *path, const char *factory_path) {
	return linphone_config_new_with_factory(path, factory_path);
}

LinphoneConfig* Factory::createConfigFromString(const char *data) {
	return linphone_config_new_from_buffer(data);
}

const bctbx_list_t* Factory::getDialPlans() {
	return linphone_dial_plan_get_all_list();
}

void* Factory::getUserData() {
	return mUserData;
}

void Factory::setUserData(void *data) {
	mUserData = data;
}

void Factory::setLogCollectionPath(const char *path) {
	linphone_core_set_log_collection_path(path);
}

void Factory::enableLogCollection(LinphoneLogCollectionState state) {
	linphone_core_enable_log_collection(state);
}

LinphoneTunnelConfig* Factory::createTunnelConfig() {
	return linphone_tunnel_config_new();
}

LinphoneLoggingServiceCbs* Factory::createLoggingServiceCbs() {
	return linphone_logging_service_cbs_new();
}

LinphonePlayerCbs* Factory::createPlayerCbs() {
	return linphone_player_cbs_new();
}

LinphoneEventCbs* Factory::createEventCbs() {
	return linphone_event_cbs_new();
}

LinphoneFriendListCbs* Factory::createFriendListCbs() {
	return linphone_friend_list_cbs_new();
}

LinphoneAccountCreatorCbs* Factory::createAccountCreatorCbs() {
	return linphone_account_creator_cbs_new();
}

LinphoneXmlRpcRequestCbs* Factory::createXmlRpcRequestCbs() {
	return linphone_xml_rpc_request_cbs_new();
}

bool_t Factory::isChatroomBackendAvailable(LinphoneChatRoomBackend chatroom_backend) {
#ifdef HAVE_ADVANCED_IM
	return TRUE;
#else
	return (chatroom_backend != LinphoneChatRoomBackendFlexisipChat);
#endif
}

bool_t Factory::isDatabaseStorageAvailable() {
#ifdef HAVE_DB_STORAGE
	return TRUE;
#else
	return FALSE;
#endif
}

bool_t Factory::isImdnAvailable() {
#ifdef HAVE_ADVANCED_IM
	return TRUE;
#else
	return FALSE;
#endif
}

const char* Factory::getConfigDir(void *context) {
	mCachedConfigDir = LinphonePrivate::Paths::getPath(LinphonePrivate::Paths::Config, context);
	return mCachedConfigDir;
}

const char* Factory::getDataDir(void *context) {
	mCachedDataDir = LinphonePrivate::Paths::getPath(LinphonePrivate::Paths::Data, context);
	return mCachedDataDir;
}

const char* Factory::getDownloadDir(void *context) {
	mCachedDownloadDir = LinphonePrivate::Paths::getPath(LinphonePrivate::Paths::Download, context);
	return mCachedDownloadDir;
}

void Factory::setVfsEncryption(const uint16_t encryptionModule, const uint8_t *secret, const size_t secretSize) {

	/* Check encryptionMpdule is valid */
	auto module = bctoolbox::EncryptionSuite::unset;
	switch (encryptionModule) {
		case LINPHONE_VFS_ENCRYPTION_UNSET: // do not use the encrypted VFS
			bctbx_vfs_set_default(bctbx_vfs_get_standard());
			bctoolbox::VfsEncryption::openCallbackSet(nullptr);
			return;
		case LINPHONE_VFS_ENCRYPTION_PLAIN: // use the encrypted VFS but write plain files
			bctbx_warning("linphone_factory_set_vfs_encryption : encryptionModule set to plain text");
			module = bctoolbox::EncryptionSuite::plain;
			break;
		case LINPHONE_VFS_ENCRYPTION_DUMMY:
			module = bctoolbox::EncryptionSuite::dummy;
			break;
		case LINPHONE_VFS_ENCRYPTION_AES256GCM128_SHA256:
			module = bctoolbox::EncryptionSuite::aes256gcm128_sha256;
			break;
		default:
			bctbx_error("linphone_factory_set_vfs_encryption : encryptionModule %04x unknown", encryptionModule);
			return;
	}

	/* save the key */
	if (mEvfsMasterKey != nullptr) {
		bctbx_clean(mEvfsMasterKey->data(), mEvfsMasterKey->size());
	}
	mEvfsMasterKey = std::make_shared<std::vector<uint8_t>>(secret, secret+secretSize);

	// Set the default bctbx vfs to the encrypted one
	bctbx_vfs_set_default(&bctoolbox::bcEncryptedVfs);

	// Associate the VfsEncryption class callback
	bctoolbox::VfsEncryption::openCallbackSet([module](bctoolbox::VfsEncryption &settings) {
		bctbx_message("Encrypted VFS: Open file %s, encryption is set to %s file. Current file's encryption module is %s", settings.filenameGet().data(), encryptionSuiteString(module).data(), encryptionSuiteString(settings.encryptionSuiteGet()).data());

		settings.encryptionSuiteSet(module); // This call will migrate plain files to encrypted ones if needed
		if (module!=bctoolbox::EncryptionSuite::plain) { // do not set keys for plain module
			settings.secretMaterialSet(*(mEvfsMasterKey));
		}
	});
}

~Factory (){
	bctbx_list_free_with_data(mSupportedVideoDefinitions, (bctbx_list_free_func)linphone_video_definition_unref);

	// sqlite3 vfs is registered at factory creation, so unregister it when destroying it
	sqlite3_bctbx_vfs_unregister();

	// proper cleaning of EVFS master key if any is set
	if (mEvfsMasterKey != nullptr) {
		bctbx_clean(mEvfsMasterKey->data(), mEvfsMasterKey->size());
		mEvfsMasterKey = nullptr;
	}
}
LINPHONE_END_NAMESPACE
