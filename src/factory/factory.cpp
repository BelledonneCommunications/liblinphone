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

#include "factory.h"

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
	this->mTopResourcesDir = PACKAGE_DATA_DIR;
	initializeSupportedVideoDefinitions(this);
	/* register the bctbx sqlite vfs. It is not used by default */
	/* sqlite3_bctbx_vfs use the default bctbx_vfs, so if encryption is turned on by default, it will apply to sqlte3 db */
	sqlite3_bctbx_vfs_register(0);
	this->mEvfsMasterKey = nullptr;
}

//BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneFactory);

//useless ???
/*BELLE_SIP_INSTANCIATE_VPTR(LinphoneFactory, belle_sip_object_t,
	linphone_factory_uninit, // destroy
	NULL, // clone
	NULL, // Marshall
	FALSE
);*/


void Factory::_DestroyingCb(void) {
	if (Factory::instance != NULL)
		Factory::instance.reset();
}

#define ADD_SUPPORTED_VIDEO_DEFINITION(factory, width, height, name) \
	(factory)->mSupportedVideoDefinitions = bctbx_list_append((factory)->mSupportedVideoDefinitions, \
		linphone_video_definition_new(width, height, name))

void  Factory::initializeSupportedVideoDefinitions(Factory *factory) {
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




std::shared_ptr<Factory> Factory::get(void) {
	if (Factory::instance == NULL) {
		Factory::instance = create();
		atexit(Factory::_DestroyingCb);
	}
	return Factory::instance;
}

void Factory::clean(void){
	LinphonePrivate::AddressPrivate::clearSipAddressesCache();
	if (Factory::instance){
		Factory::instance.reset();
	}
}

LinphoneCore *Factory::_createCore (
	LinphoneCoreCbs *cbs,
	const char *config_path,
	const char *factory_config_path,
	void *user_data,
	void *system_context,
	bool_t automatically_start
) const {
		bctbx_init_logger(FALSE);
		LpConfig *config = linphone_config_new_with_factory(config_path, factory_config_path);
		LinphoneCore *lc = _linphone_core_new_with_config(cbs, config, user_data, system_context, automatically_start);
		linphone_config_unref(config);
		bctbx_uninit_logger();
		return lc;
}

LinphoneCore *Factory::_createSharedCore (
	LinphoneCoreCbs *cbs,
	const char *config_filename,
	const char *factory_config_path,
	void *user_data,
	void *system_context,
	bool_t automatically_start,
	const char *app_group_id,
	bool_t main_core
) const{
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
) const {
		return _createCore(cbs, config_path, factory_config_path, NULL, NULL, TRUE);
}

LinphoneCore* Factory::createCore (
	LinphoneCoreCbs *cbs,
	const char *config_path,
	const char *factory_config_path,
	void *user_data,
	void *system_context
) const {
		return _createCore(cbs, config_path, factory_config_path, user_data, system_context, TRUE);
}

LinphoneCore* Factory::createCore (
	const char *config_path,
	const char *factory_config_path,
	void *system_context
) const {
		return _createCore(NULL, config_path, factory_config_path, NULL, system_context, FALSE);
}

LinphoneCore* Factory::createSharedCore (
	const char *config_filename,
	const char *factory_config_path,
	void *system_context,
	const char *app_group_id,
	bool_t main_core
) const {
		return _createSharedCore(NULL, factory_config_path, factory_config_path, NULL, system_context, FALSE, app_group_id, main_core);
}

LinphoneCore* Factory::createCoreWithConfig (
	LinphoneCoreCbs *cbs,
	LinphoneConfig *config
) const {
		return _linphone_core_new_with_config(cbs, config, NULL, NULL, TRUE);
}

LinphoneCore* Factory::createCoreWithConfig (
	LinphoneCoreCbs *cbs,
	LinphoneConfig *config,
	void *user_data,
	void *system_context
) const {
		return _linphone_core_new_with_config(cbs, config, user_data, system_context, TRUE);
}

LinphoneCore* Factory::createCoreWithConfig (
	LinphoneConfig *config,
	void *system_context
) const {
		return _linphone_core_new_with_config(NULL, config, NULL, system_context, FALSE);
}

LinphoneCore* Factory::createSharedCoreWithConfig (
	LinphoneConfig *config,
	void *system_context,
	const char *app_group_id,
	bool_t main_core
) const {
		return _linphone_core_new_shared_with_config(NULL, config, NULL, system_context, FALSE, app_group_id, main_core);
}

LinphoneCoreCbs* Factory::createCoreCbs() const {
	return _linphone_core_cbs_new();
}

LinphoneAddress* Factory::createAddress (const char *addr) const {
	return linphone_address_new(addr);
}

LinphoneParticipantDeviceIdentity* Factory::createParticipantDeviceIdentity (
	const LinphoneAddress *address,
	const char *name
) const {
		#ifdef HAVE_ADVANCED_IM
			return linphone_participant_device_identity_new(address, name);
		#else
			ms_warning("Advanced IM such as group chat is disabled");
			return NULL;
		#endif
}

LinphoneAuthInfo* Factory::createAuthInfo (const char *username, const char *userid, const char *passwd, const char *ha1, const char *realm, const char *domain) const {
	return linphone_auth_info_new(username, userid, passwd, ha1, realm, domain);
}

LinphoneAuthInfo* Factory::createAuthInfo(const char *username, const char *userid, const char *passwd, const char *ha1, const char *realm, const char *domain, const char *algorithm) const {
	return linphone_auth_info_new_for_algorithm(username, userid, passwd, ha1, realm, domain, algorithm);
}

LinphoneCallCbs* Factory::createCallCbs() const {
	return _linphone_call_cbs_new();
}

LinphoneChatRoomCbs* Factory::createChatRoomCbs() const {
	return _linphone_chat_room_cbs_new();
}

LinphoneChatMessageCbs* Factory::createChatMessageCbs() const {
	return linphone_chat_message_cbs_new();
}

LinphoneVcard* Factory::createVcard() const {
	return _linphone_vcard_new();
}

LinphoneVideoDefinition*  Factory::createVideoDefinition(unsigned int width, unsigned int height) const {
	LinphoneVideoDefinition *supported = this->findSupportedVideoDefinition(width, height);
	return supported ? linphone_video_definition_clone(supported) : linphone_video_definition_new(width, height, NULL);
}

LinphoneVideoDefinition*  Factory::createVideoDefinitionFromName(const char *name) const {
	unsigned int width = 0;
	unsigned int height = 0;
	LinphoneVideoDefinition *vdef = this->findSupportedVideoDefinitionByName(name);
	if (vdef != NULL) return vdef;
	if (sscanf(name, "%ux%u", &width, &height) == 2) {
		return linphone_video_definition_new(width, height, NULL);
	}
	return linphone_video_definition_new(0, 0, NULL);
}

const bctbx_list_t* Factory::getSupportedVideoDefinitions() const {
	return this->mSupportedVideoDefinitions;
}

LinphoneVideoDefinition*  Factory::findSupportedVideoDefinition(unsigned int width, unsigned int height) const {
	const bctbx_list_t *item;
	const bctbx_list_t *supported = this->getSupportedVideoDefinitions();
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

LinphoneVideoDefinition*  Factory::findSupportedVideoDefinitionByName(const char *name) const {
	const bctbx_list_t *item;
	const bctbx_list_t *supported = this->getSupportedVideoDefinitions();

	for (item = supported; item != NULL; item = bctbx_list_next(item)) {
		LinphoneVideoDefinition *svdef = (LinphoneVideoDefinition *)bctbx_list_get_data(item);
		if (strcmp(linphone_video_definition_get_name(svdef), name) == 0) {
			return svdef;
		}
	}

	ms_error("Couldn't find a supported video definition for name [%s]", name);
	return NULL;
}

const std::string Factory::getTopResourcesDir() const {
	return this->mTopResourcesDir;
}

void Factory::setTopResourcesDir(const char *path) {
	this->mTopResourcesDir = path;
}

const std::string Factory::getDataResourcesDir() {
	if (!this->mDataResourcesDir.empty()) return this->mDataResourcesDir;
	if (!this->mTopResourcesDir.empty()){
		this->mCachedDataResourcesDir = this->mTopResourcesDir + "/linphone";
	}else{
		this->mCachedDataResourcesDir.append(PACKAGE_DATA_DIR);
		this->mCachedDataResourcesDir.append("/linphone");
	}
	return this->mCachedDataResourcesDir;
}

void Factory::setDataResourcesDir(const char *path) {
	this->mDataResourcesDir = path;
}

const std::string Factory::getSoundResourcesDir() {
	if (!this->mSoundResourcesDir.empty()) return this->mSoundResourcesDir;
	if (!this->mTopResourcesDir.empty()){
		this->mCachedDataResourcesDir = this->mTopResourcesDir;
		this->mCachedDataResourcesDir.append("/sounds/linphone");
		return this->mCachedDataResourcesDir;
	}
	return PACKAGE_SOUND_DIR;
}

void Factory::setSoundResourcesDir(const char *path) {
	this->mSoundResourcesDir = path;
}

const std::string Factory::getRingResourcesDir() {
	if (!this->mRingResourcesDir.empty()) return mRingResourcesDir;
	if (!this->mSoundResourcesDir.empty()){
		this->mCachedRingResourcesDir = this->mSoundResourcesDir;
		this->mCachedRingResourcesDir.append("/rings");
		return this->mCachedRingResourcesDir;
	}
	if (!this->mTopResourcesDir.empty()) {
		this->mCachedRingResourcesDir = this->mTopResourcesDir;
		this->mCachedRingResourcesDir.append("/sounds/linphone/rings");
		return this->mCachedRingResourcesDir;
	}
	return PACKAGE_RING_DIR;
}

void Factory::setRingResourcesDir(const char *path) {
	this->mRingResourcesDir = path;
}

const std::string Factory::getImageResourcesDir() {
	if (!this->mImageResourcesDir.empty()) return this->mImageResourcesDir;
	if (!this->mTopResourcesDir.empty()) {
		this->mCachedImageResourcesDir = this->mTopResourcesDir;
		this->mCachedImageResourcesDir.append("/images");
	}else{
		this->mCachedImageResourcesDir = PACKAGE_DATA_DIR;
		this->mCachedImageResourcesDir.append("/images");
	}
	return this->mCachedImageResourcesDir;
}

void Factory::setImageResourcesDir(const char *path) {
	this->mImageResourcesDir = path;
}

const std::string Factory::getMspluginsDir() const {
	return this->mMspluginsDir;
}

void Factory::setMspluginsDir(const char *path) {
	this->mMspluginsDir = path;
}

LinphoneErrorInfo* Factory::createErrorInfo() const {
	return linphone_error_info_new();
}

LinphoneRange* Factory::createRange() const {
	return linphone_range_new();
}

LinphoneTransports* Factory::createTransports() const {
	return linphone_transports_new();
}

LinphoneVideoActivationPolicy*  Factory::createVideoActivationPolicy() const {
	return linphone_video_activation_policy_new();
}

LinphoneContent* Factory::createContent() const {
	return linphone_content_new();
}

LinphoneBuffer* Factory::createBuffer() const {
	return linphone_buffer_new();
}

LinphoneBuffer* Factory::createBufferFromData(const uint8_t *data, size_t size) const {
	return linphone_buffer_new_from_data(data, size);
}

LinphoneBuffer* Factory::createBufferFromString(const char *data) const {
	return linphone_buffer_new_from_string(data);
}

LinphoneConfig* Factory::createConfig(const char *path) const {
	return linphone_config_new(path);
}

LinphoneConfig* Factory::createConfigWithFactory(const char *path, const char *factory_path) const {
	return linphone_config_new_with_factory(path, factory_path);
}

LinphoneConfig* Factory::createConfigFromString(const char *data) const {
	return linphone_config_new_from_buffer(data);
}

const bctbx_list_t* Factory::getDialPlans() const {
	return linphone_dial_plan_get_all_list();
}

void* Factory::getUserData() const {
	return this->mUserData;
}

void Factory::setUserData(void *data) {
	this->mUserData = data;
}

void Factory::setLogCollectionPath(const char *path) const {
	linphone_core_set_log_collection_path(path);
}

void Factory::enableLogCollection(LinphoneLogCollectionState state) const {
	linphone_core_enable_log_collection(state);
}

LinphoneTunnelConfig* Factory::createTunnelConfig() const {
	return linphone_tunnel_config_new();
}

LinphoneLoggingServiceCbs* Factory::createLoggingServiceCbs() const {
	return linphone_logging_service_cbs_new();
}

LinphonePlayerCbs* Factory::createPlayerCbs() const {
	return linphone_player_cbs_new();
}

LinphoneEventCbs* Factory::createEventCbs() const {
	return linphone_event_cbs_new();
}

LinphoneFriendListCbs* Factory::createFriendListCbs() const {
	return linphone_friend_list_cbs_new();
}

LinphoneAccountCreatorCbs* Factory::createAccountCreatorCbs() const {
	return linphone_account_creator_cbs_new();
}

LinphoneXmlRpcRequestCbs* Factory::createXmlRpcRequestCbs() const {
	return linphone_xml_rpc_request_cbs_new();
}

bool_t Factory::isChatroomBackendAvailable(LinphoneChatRoomBackend chatroom_backend) const {
	#ifdef HAVE_ADVANCED_IM
		return TRUE;
	#else
		return (chatroom_backend != LinphoneChatRoomBackendFlexisipChat);
	#endif
}

bool_t Factory::isDatabaseStorageAvailable() const {
	#ifdef HAVE_DB_STORAGE
		return TRUE;
	#else
		return FALSE;
	#endif
}

bool_t Factory::isImdnAvailable() const {
	#ifdef HAVE_ADVANCED_IM
		return TRUE;
	#else
		return FALSE;
	#endif
}

const std::string Factory::getConfigDir(void *context) {
	this->mCachedConfigDir = LinphonePrivate::Paths::getPath(LinphonePrivate::Paths::Config, context);
	return this->mCachedConfigDir;
}

const std::string Factory::getDataDir(void *context) {
	this->mCachedDataDir = LinphonePrivate::Paths::getPath(LinphonePrivate::Paths::Data, context);
	return this->mCachedDataDir;
}

const std::string Factory::getDownloadDir(void *context) {
	this->mCachedDownloadDir = LinphonePrivate::Paths::getPath(LinphonePrivate::Paths::Download, context);
	return this->mCachedDownloadDir;
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
	if (this->mEvfsMasterKey != nullptr) {
		bctbx_clean(this->mEvfsMasterKey->data(), this->mEvfsMasterKey->size());
	}
	this->mEvfsMasterKey = std::make_shared<std::vector<uint8_t>>(secret, secret+secretSize);

	// Set the default bctbx vfs to the encrypted one
	bctbx_vfs_set_default(&bctoolbox::bcEncryptedVfs);

	// Associate the VfsEncryption class callback
	bctoolbox::VfsEncryption::openCallbackSet([module, this](bctoolbox::VfsEncryption &settings) {
		bctbx_message("Encrypted VFS: Open file %s, encryption is set to %s file. Current file's encryption module is %s", settings.filenameGet().data(), encryptionSuiteString(module).data(), encryptionSuiteString(settings.encryptionSuiteGet()).data());

		settings.encryptionSuiteSet(module); // This call will migrate plain files to encrypted ones if needed
		if (module!=bctoolbox::EncryptionSuite::plain) { // do not set keys for plain module
			//settings.secretMaterialSet(*(mEvfsMasterKey));
			settings.secretMaterialSet(*this->mEvfsMasterKey);
		}
	});
}

Factory::~Factory (){
	bctbx_list_free_with_data(this->mSupportedVideoDefinitions, (bctbx_list_free_func)linphone_video_definition_unref);

	// sqlite3 vfs is registered at factory creation, so unregister it when destroying it
	sqlite3_bctbx_vfs_unregister();

	// proper cleaning of EVFS master key if any is set
	if (this->mEvfsMasterKey != nullptr) {
		bctbx_clean(this->mEvfsMasterKey->data(), this->mEvfsMasterKey->size());
		this->mEvfsMasterKey = nullptr;
	}
	this->clean();
}

LINPHONE_END_NAMESPACE
