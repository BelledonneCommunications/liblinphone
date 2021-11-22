/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
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

#include <cstdio>
#include <fstream>
#include <sstream>

#include "address/address.h"
#include "core/paths/paths.h"
#include "bctoolbox/vfs_encrypted.hh"
#include "bctoolbox/crypto.h"
#include "chat/ics/ics.h"
#include "conference/conference-info.h"
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

std::shared_ptr<Factory> Factory::instance;

Factory::Factory(){
	mTopResourcesDir = PACKAGE_DATA_DIR;
	
	/*used to avoid crash when using bctbx_list_append in ADD_SUPPORTED_VIDEO_DEFINITION
	since the element is null, bctbx_list_append return the added element directly*/
	mSupportedVideoDefinitions = nullptr;
	initializeSupportedVideoDefinitions(this);
	
	mUserData = nullptr;
	
	mPackageSoundDir = PACKAGE_SOUND_DIR;
	mPackageRingDir = PACKAGE_RING_DIR;
	mPackageDataDir = PACKAGE_DATA_DIR;
	
	/* register the bctbx sqlite vfs. It is not used by default */
	/* sqlite3_bctbx_vfs use the default bctbx_vfs, so if encryption is turned on by default, it will apply to sqlte3 db */
	sqlite3_bctbx_vfs_register(0);
	mEvfsMasterKey = nullptr;
}


void Factory::_DestroyingCb(void) {
	if (Factory::instance != NULL)
		Factory::instance.reset();
}

#define ADD_SUPPORTED_VIDEO_DEFINITION(factory, width, height, name) \
	(factory)->mSupportedVideoDefinitions = bctbx_list_append((factory)->mSupportedVideoDefinitions, \
	linphone_video_definition_new(width, height, name))

void  Factory::initializeSupportedVideoDefinitions(Factory *factory) {
	ADD_SUPPORTED_VIDEO_DEFINITION(factory, MS_VIDEO_SIZE_1080P_W, MS_VIDEO_SIZE_1080P_H, "1080p");
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
	LinphonePrivate::Address::clearSipAddressesCache();
	if (Factory::instance){
		Factory::instance.reset();
	}
}

LinphoneCore *Factory::_createCore (
		LinphoneCoreCbs *cbs,
		const std::string& config_path,
		const std::string& factory_config_path,
		void *user_data,
		void *system_context,
		bool_t automatically_start
		) const {
	bctbx_init_logger(FALSE);
	LpConfig *config = linphone_config_new_with_factory(config_path.c_str(), factory_config_path.c_str());
	LinphoneCore *lc = _linphone_core_new_with_config(cbs, config, user_data, system_context, automatically_start);
	linphone_config_unref(config);
	bctbx_uninit_logger();
	return lc;
}

LinphoneCore *Factory::_createSharedCore (
		LinphoneCoreCbs *cbs,
		const std::string& config_filename,
		const std::string& factory_config_path,
		void *user_data,
		void *system_context,
		bool_t automatically_start,
		const std::string& app_group_id,
		bool_t main_core
		) const{
	bctbx_init_logger(FALSE);
	LpConfig *config = linphone_config_new_for_shared_core(app_group_id.c_str(), config_filename.c_str(), factory_config_path.c_str());
	LinphoneCore *lc = _linphone_core_new_shared_with_config(cbs, config, user_data, system_context, automatically_start, app_group_id.c_str(), main_core);
	linphone_config_unref(config);
	bctbx_uninit_logger();
	return lc;
}

LinphoneCore* Factory::createCore (
		LinphoneCoreCbs *cbs,
		const std::string& config_path,
		const std::string& factory_config_path
		) const {
	return _createCore(cbs, config_path.c_str(), factory_config_path.c_str(), NULL, NULL, TRUE);
}

LinphoneCore* Factory::createCore (
		LinphoneCoreCbs *cbs,
		const std::string& config_path,
		const std::string& factory_config_path,
		void *user_data,
		void *system_context
		) const {
	return _createCore(cbs, config_path, factory_config_path, user_data, system_context, TRUE);
}

LinphoneCore* Factory::createCore (
		const std::string& config_path,
		const std::string& factory_config_path,
		void *system_context
		) const {
	return _createCore(NULL, config_path, factory_config_path, NULL, system_context, FALSE);
}

LinphoneCore* Factory::createSharedCore (
		const std::string& config_filename,
		const std::string& factory_config_path,
		void *system_context,
		const std::string& app_group_id,
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
		const std::string& app_group_id,
		bool_t main_core
		) const {
	return _linphone_core_new_shared_with_config(NULL, config, NULL, system_context, FALSE, app_group_id.c_str(), main_core);
}

LinphoneCoreCbs* Factory::createCoreCbs() const {
	return _linphone_core_cbs_new();
}

LinphoneAddress* Factory::createAddress (const std::string& addr) const {
	return linphone_address_new(addr.c_str());
}

LinphoneParticipantDeviceCbs* Factory::createParticipantDeviceCbs() const {
	return linphone_participant_device_cbs_new();
}

LinphoneParticipantDeviceIdentity* Factory::createParticipantDeviceIdentity (
		const LinphoneAddress *address,
		const std::string& name
		) const {
#ifdef HAVE_ADVANCED_IM
	return linphone_participant_device_identity_new(address, name.c_str());
#else
	ms_warning("Advanced IM such as group chat is disabled");
	return NULL;
#endif
}

LinphoneAuthInfo* Factory::createAuthInfo (const std::string& username, const std::string& userid, const std::string& passwd, const std::string& ha1, const std::string& realm, const std::string& domain) const {
	return linphone_auth_info_new(username.c_str(), userid.c_str(), passwd.c_str(), ha1.c_str(), realm.c_str(), domain.c_str());
}

LinphoneAuthInfo* Factory::createAuthInfo(const std::string& username, const std::string& userid, const std::string& passwd, const std::string& ha1, const std::string& realm, const std::string& domain, const std::string& algorithm) const {
	return linphone_auth_info_new_for_algorithm(username.c_str(), userid.c_str(), passwd.c_str(), ha1.c_str(), realm.c_str(), domain.c_str(), algorithm.c_str());
}

LinphoneCallCbs* Factory::createCallCbs() const {
	return _linphone_call_cbs_new();
}

LinphoneConferenceCbs * Factory::createConferenceCbs() const {
	return _linphone_conference_cbs_new();
}


LinphoneChatRoomCbs* Factory::createChatRoomCbs() const {
	return _linphone_chat_room_cbs_new();
}

LinphoneChatMessageCbs* Factory::createChatMessageCbs() const {
	return linphone_chat_message_cbs_new();
}

LinphoneMagicSearchCbs* Factory::createMagicSearchCbs() const {
	return linphone_magic_search_cbs_new();
}


LinphoneVcard* Factory::createVcard() const {
	return _linphone_vcard_new();
}

LinphoneVideoDefinition*  Factory::createVideoDefinition(unsigned int width, unsigned int height) const {
	LinphoneVideoDefinition *supported = findSupportedVideoDefinition(width, height);
	return supported ? linphone_video_definition_clone(supported) : linphone_video_definition_new(width, height, NULL);
}

LinphoneVideoDefinition*  Factory::createVideoDefinitionFromName(const std::string& name) const {
	unsigned int width = 0;
	unsigned int height = 0;
	LinphoneVideoDefinition *vdef = findSupportedVideoDefinitionByName(name);
	if (vdef != NULL) return linphone_video_definition_clone(vdef);
	if (sscanf(name.c_str(), "%ux%u", &width, &height) == 2) {
		return linphone_video_definition_new(width, height, NULL);
	}
	return linphone_video_definition_new(0, 0, NULL);
}

const bctbx_list_t* Factory::getSupportedVideoDefinitions() const {
	return mSupportedVideoDefinitions;
}

const bctbx_list_t* Factory::getRecommendedVideoDefinitions() const {
	bctbx_list_t * recommended = NULL;
	for(const bctbx_list_t *supported = getSupportedVideoDefinitions(); supported ; supported = bctbx_list_next(supported)){
		LinphoneVideoDefinition* def = static_cast<LinphoneVideoDefinition*>(supported->data);
#if defined(__ANDROID__) || TARGET_OS_IPHONE	// On mobile, we recommend a format of max 720p
		if( def->width*def->height <= MS_VIDEO_SIZE_720P_W * MS_VIDEO_SIZE_720P_H)
#endif
			recommended = bctbx_list_append(recommended, linphone_video_definition_ref(def));
	}
	return recommended;
}

LinphoneVideoDefinition*  Factory::findSupportedVideoDefinition(unsigned int width, unsigned int height) const {
	const bctbx_list_t *item;
	const bctbx_list_t *supported = getSupportedVideoDefinitions();
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

LinphoneVideoDefinition*  Factory::findSupportedVideoDefinitionByName(const std::string& name) const {
	const bctbx_list_t *item;
	const bctbx_list_t *supported = getSupportedVideoDefinitions();
	
	for (item = supported; item != NULL; item = bctbx_list_next(item)) {
		LinphoneVideoDefinition *svdef = (LinphoneVideoDefinition *)bctbx_list_get_data(item);
		if (linphone_video_definition_get_name(svdef) == name ) {
			return svdef;
		}
	}
	
	ms_error("Couldn't find a supported video definition for name [%s]", name.c_str());
	return NULL;
}

const std::string & Factory::getTopResourcesDir() const {
	return mTopResourcesDir;
}

void Factory::setTopResourcesDir(const std::string& path) {
	mTopResourcesDir = path;
}

const std::string & Factory::getDataResourcesDir() {
	if (!mDataResourcesDir.empty()) return mDataResourcesDir;
	if (!mTopResourcesDir.empty()){
		mCachedDataResourcesDir = mTopResourcesDir + "/linphone";
	}else{
		mCachedDataResourcesDir.append(PACKAGE_DATA_DIR);
		mCachedDataResourcesDir.append("/linphone");
	}
	return mCachedDataResourcesDir;
}

void Factory::setDataResourcesDir(const std::string& path) {
	mDataResourcesDir = path;
}

const std::string & Factory::getSoundResourcesDir() {
	if (!mSoundResourcesDir.empty()) return mSoundResourcesDir;
	if (!mTopResourcesDir.empty()){
		mCachedDataResourcesDir = mTopResourcesDir;
		mCachedDataResourcesDir.append("/sounds/linphone");
		return mCachedDataResourcesDir;
	}
	return mPackageSoundDir;
}

void Factory::setSoundResourcesDir(const std::string& path) {
	mSoundResourcesDir = path;
}

const std::string & Factory::getRingResourcesDir() {
	if (!mRingResourcesDir.empty()) return mRingResourcesDir;
	if (!mSoundResourcesDir.empty()){
		mCachedRingResourcesDir = mSoundResourcesDir;
		mCachedRingResourcesDir.append("/rings");
		return mCachedRingResourcesDir;
	}
	if (!mTopResourcesDir.empty()) {
		mCachedRingResourcesDir = mTopResourcesDir;
		mCachedRingResourcesDir.append("/sounds/linphone/rings");
		return mCachedRingResourcesDir;
	}
	return mPackageRingDir;
}

void Factory::setRingResourcesDir(const std::string& path) {
	mRingResourcesDir = path;
}

const std::string & Factory::getImageResourcesDir() {
	if (!mImageResourcesDir.empty()) return mImageResourcesDir;
	if (!mTopResourcesDir.empty()) {
		mCachedImageResourcesDir = mTopResourcesDir;
		mCachedImageResourcesDir.append("/images");
	}else{
		mCachedImageResourcesDir = PACKAGE_DATA_DIR;
		mCachedImageResourcesDir.append("/images");
	}
	return mCachedImageResourcesDir;
}

void Factory::setImageResourcesDir(const std::string& path) {
	mImageResourcesDir = path;
}

const std::string & Factory::getMspluginsDir() const {
	return mMspluginsDir;
}

void Factory::setMspluginsDir(const std::string& path) {
	mMspluginsDir = path;
}

const std::string & Factory::getConfigDir(void *context) {
	if(!mConfigDir.empty())
		return mConfigDir;
	mCachedConfigDir = LinphonePrivate::Paths::getPath(LinphonePrivate::Paths::Config, context);
	return mCachedConfigDir;
}

bool_t Factory::isConfigDirSet() const{
	return !mConfigDir.empty();
}

void Factory::setConfigDir(const std::string& path) {
	mConfigDir = path;
}

const std::string & Factory::getDataDir(void *context) {
	if(!mDataDir.empty())
		return mDataDir;
	mCachedDataDir = LinphonePrivate::Paths::getPath(LinphonePrivate::Paths::Data, context);
	return mCachedDataDir;
}

bool_t Factory::isDataDirSet() const{
	return !mDataDir.empty();
}

void Factory::setDataDir(const std::string& path) {
	mDataDir = path;
}

const std::string & Factory::getDownloadDir(void *context) {
	if(!mDownloadDir.empty())
		return mDownloadDir;
	mCachedDownloadDir = LinphonePrivate::Paths::getPath(LinphonePrivate::Paths::Download, context);
	return mCachedDownloadDir;
}

bool_t Factory::isDownloadDirSet() const{
	return !mDownloadDir.empty();
}

void Factory::setDownloadDir(const std::string& path) {
	mDownloadDir = path;
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

LinphoneBuffer* Factory::createBufferFromString(const std::string& data) const {
	return linphone_buffer_new_from_string(data.c_str());
}

LinphoneConfig* Factory::createConfig(const std::string& path) const {
	return linphone_config_new(path.c_str());
}

LinphoneConfig* Factory::createConfigWithFactory(const std::string& path, const std::string& factory_path) const {
	return linphone_config_new_with_factory(path.c_str(), factory_path.c_str());
}

LinphoneConfig* Factory::createConfigFromString(const std::string& data) const {
	return linphone_config_new_from_buffer(data.c_str());
}

const bctbx_list_t* Factory::getDialPlans() const {
	return linphone_dial_plan_get_all_list();
}

void* Factory::getUserData() const {
	return mUserData;
}

void Factory::setUserData(void *data) {
	mUserData = data;
}

void Factory::setLogCollectionPath(const std::string& path) const {
	linphone_core_set_log_collection_path(path.c_str());
}

void Factory::enableLogCollection(LinphoneLogCollectionState state) const {
	linphone_core_enable_log_collection(state);
}

LinphoneTunnelConfig* Factory::createTunnelConfig() const {
	return linphone_tunnel_config_new();
}

LinphoneAccountCbs *Factory::createAccountCbs() const {
	return linphone_account_cbs_new();
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

void Factory::setVfsEncryption(const uint16_t encryptionModule, const uint8_t *secret, const size_t secretSize) {
	
	/* Check encryptionMpdule is valid */
	auto module = bctoolbox::EncryptionSuite::unset;
	switch (encryptionModule) {
		case LINPHONE_VFS_ENCRYPTION_UNSET: // do not use the encrypted VFS
			bctbx_warning("linphone_factory_set_vfs_encryption : disable encryption");
			bctbx_vfs_set_default(bctbx_vfs_get_standard());
			bctoolbox::VfsEncryption::openCallbackSet(nullptr);
			return;
		case LINPHONE_VFS_ENCRYPTION_PLAIN: // use the encrypted VFS but write plain files
			bctbx_warning("linphone_factory_set_vfs_encryption : encryptionModule set to plain text");
			module = bctoolbox::EncryptionSuite::plain;
			break;
		case LINPHONE_VFS_ENCRYPTION_DUMMY:
			bctbx_warning("linphone_factory_set_vfs_encryption : encryptionModule set to dummy: use this setting for testing only");
			module = bctoolbox::EncryptionSuite::dummy;
			break;
		case LINPHONE_VFS_ENCRYPTION_AES256GCM128_SHA256:
			bctbx_message("linphone_factory_set_vfs_encryption : encryptionModule set to AES256GCM_SHA256");
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
	bctoolbox::VfsEncryption::openCallbackSet([module, this](bctoolbox::VfsEncryption &settings) {
		bctbx_message("Encrypted VFS: Open file %s, encryption is set to %s file. Current file's encryption module is %s", settings.filenameGet().data(), encryptionSuiteString(module).data(), encryptionSuiteString(settings.encryptionSuiteGet()).data());
		settings.encryptionSuiteSet(module); // This call will migrate plain files to encrypted ones if needed
		if (module!=bctoolbox::EncryptionSuite::plain) { // do not set keys for plain module
			//settings.secretMaterialSet(*(mEvfsMasterKey));
			settings.secretMaterialSet(*mEvfsMasterKey);
		}
	});
}

LinphoneDigestAuthenticationPolicy * Factory::createDigestAuthenticationPolicy()const{
	return linphone_digest_authentication_policy_new();
}

Factory::~Factory (){
	bctbx_list_free_with_data(mSupportedVideoDefinitions, (bctbx_list_free_func)linphone_video_definition_unref);
	
	// sqlite3 vfs is registered at factory creation, so unregister it when destroying it
	sqlite3_bctbx_vfs_unregister();
	
	// proper cleaning of EVFS master key if any is set
	if (mEvfsMasterKey != nullptr) {
		bctbx_clean(mEvfsMasterKey->data(), mEvfsMasterKey->size());
		mEvfsMasterKey = nullptr;
	}
	clean();
}

std::shared_ptr<ConferenceInfo> Factory::createConferenceInfo() const {
	return ConferenceInfo::create();
}

std::shared_ptr<ConferenceInfo> Factory::createConferenceInfoFromIcalendarContent(LinphoneContent *content) const {
	LinphonePrivate::ContentType contentType = L_GET_CPP_PTR_FROM_C_OBJECT(content)->getContentType();
	if (!contentType.strongEqual(ContentType::Icalendar)) return nullptr;

	std::string filepath = "";
	if (linphone_content_is_file_encrypted(content)) {
		char *tmp = linphone_content_get_plain_file_path(content);
		filepath = tmp ? tmp : "";
		if (tmp) bctbx_free(tmp);
	} else if (linphone_content_get_file_path(content)) {
		filepath = linphone_content_get_file_path(content);
	}

	std::stringstream buffer;
	if (!filepath.empty()) {
		std::ifstream contentFile(filepath);
		if (!contentFile.is_open()) {
			bctbx_error("Could not open Icalendar content file path: %s", filepath.c_str());
			return nullptr;
		}

		buffer << contentFile.rdbuf();

		if (linphone_content_is_file_encrypted(content)) {
			// Remove the plain copy if file is encrypted
			std::remove(filepath.c_str());
		}
	} else {
		const char *body = linphone_content_get_utf8_text(content);
		if (!body) {
			bctbx_error("Icalendar content has no body and no file path");
			return nullptr;
		}

		buffer << body;
	}

	auto ics = Ics::Icalendar::createFromString(buffer.str());

	return ics ? ics->toConferenceInfo() : nullptr;
}

LINPHONE_END_NAMESPACE
