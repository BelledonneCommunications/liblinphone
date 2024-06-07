/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#include "factory.h"

#ifdef QRCODE_ENABLED
#define ZX_USE_UTF8
#ifdef ZXING_USE_BUILD_INTERFACE
#include <BarcodeFormat.h>
#include <BitMatrix.h>
#include <BitMatrixIO.h>
#include <CharacterSet.h>
#include <MultiFormatWriter.h>
#include <TextUtfEncoding.h>
#else
#include <ZXing/BarcodeFormat.h>
#include <ZXing/BitMatrix.h>
#include <ZXing/BitMatrixIO.h>
#include <ZXing/CharacterSet.h>
#include <ZXing/MultiFormatWriter.h>
#include <ZXing/TextUtfEncoding.h>
#endif
#ifdef JPEG_ENABLED
#include <mediastreamer2/msvideo.h>
#include <turbojpeg.h>
#endif
#endif

#include <cstdio>
#include <fstream>
#include <sstream>

#include "bctoolbox/crypto.hh"
#include "bctoolbox/defs.h"
#include "bctoolbox/vfs_encrypted.hh"

#include "address/address.h"
#include "alert/alert.h"
#include "auth-info/auth-info.h"
#include "chat/ics/ics.h"
#include "conference/conference-info.h"
#include "conference/participant-info.h"
#include "content/file-content.h"
#include "core/paths/paths.h"
#include "dictionary/dictionary.h"
#include "linphone/api/c-api.h"
#include "linphone/utils/utils.h"
#include "logger/logger.h"
#include "payload-type/payload-type.h"
#include "signal-information/signal-information.h"
#include "vcard/vcard.h"

#ifdef HAVE_SQLITE
#include "sqlite3_bctbx_vfs.h"
#endif

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

Factory::Factory() {
	mTopResourcesDir = PACKAGE_DATA_DIR;

	/*used to avoid crash when using bctbx_list_append in ADD_SUPPORTED_VIDEO_DEFINITION
	since the element is null, bctbx_list_append return the added element directly*/
	mSupportedVideoDefinitions = nullptr;
	initializeSupportedVideoDefinitions(this);

	mUserData = nullptr;

	mPackageSoundDir = PACKAGE_SOUND_DIR;
	mPackageRingDir = PACKAGE_RING_DIR;
	mPackageDataDir = PACKAGE_DATA_DIR;

#ifdef HAVE_SQLITE
	/* register the bctbx sqlite vfs. It is not used by default */
	/* sqlite3_bctbx_vfs use the default bctbx_vfs, so if encryption is turned on by default, it will apply to sqlte3 db
	 */
	sqlite3_bctbx_vfs_register(0);
#endif
	mEvfsMasterKey = nullptr;
}

void Factory::_DestroyingCb(void) {
	if (Factory::instance != NULL) Factory::instance.reset();
}

#define ADD_SUPPORTED_VIDEO_DEFINITION(factory, width, height, name)                                                   \
	(factory)->mSupportedVideoDefinitions =                                                                            \
	    bctbx_list_append((factory)->mSupportedVideoDefinitions, linphone_video_definition_new(width, height, name))

void Factory::initializeSupportedVideoDefinitions(Factory *factory) {
	ADD_SUPPORTED_VIDEO_DEFINITION(factory, MS_VIDEO_SIZE_1080P_W, MS_VIDEO_SIZE_1080P_H, "1080p");
#if !defined(__ANDROID__) &&                                                                                           \
    !TARGET_OS_MAC /*limit to most common sizes because mac video API cannot list supported resolutions*/
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

void Factory::clean(void) {
	LinphonePrivate::Address::clearSipAddressesCache();
	if (Factory::instance) {
		Factory::instance.reset();
	}
}

LinphoneCore *Factory::_createCore(LinphoneCoreCbs *cbs,
                                   const std::string &config_path,
                                   const std::string &factory_config_path,
                                   void *user_data,
                                   void *system_context,
                                   bool_t automatically_start) const {
	bctbx_init_logger(FALSE);
	LpConfig *config = linphone_config_new_with_factory(config_path.c_str(), factory_config_path.c_str());
	LinphoneCore *lc = _linphone_core_new_with_config(cbs, config, user_data, system_context, automatically_start);
	linphone_config_unref(config);
	bctbx_uninit_logger();
	return lc;
}

LinphoneCore *Factory::_createSharedCore(LinphoneCoreCbs *cbs,
                                         const std::string &config_filename,
                                         const std::string &factory_config_path,
                                         void *user_data,
                                         void *system_context,
                                         bool_t automatically_start,
                                         const std::string &app_group_id,
                                         bool_t main_core) const {
	bctbx_init_logger(FALSE);
	LpConfig *config =
	    linphone_config_new_for_shared_core(app_group_id.c_str(), config_filename.c_str(), factory_config_path.c_str());
	LinphoneCore *lc = _linphone_core_new_shared_with_config(cbs, config, user_data, system_context,
	                                                         automatically_start, app_group_id.c_str(), main_core);
	linphone_config_unref(config);
	bctbx_uninit_logger();
	return lc;
}

LinphoneCore *Factory::createCore(LinphoneCoreCbs *cbs,
                                  const std::string &config_path,
                                  const std::string &factory_config_path) const {
	return _createCore(cbs, config_path.c_str(), factory_config_path.c_str(), NULL, NULL, TRUE);
}

LinphoneCore *Factory::createCore(LinphoneCoreCbs *cbs,
                                  const std::string &config_path,
                                  const std::string &factory_config_path,
                                  void *user_data,
                                  void *system_context) const {
	return _createCore(cbs, config_path, factory_config_path, user_data, system_context, TRUE);
}

LinphoneCore *Factory::createCore(const std::string &config_path,
                                  const std::string &factory_config_path,
                                  void *system_context) const {
	return _createCore(NULL, config_path, factory_config_path, NULL, system_context, FALSE);
}

LinphoneCore *Factory::createSharedCore(BCTBX_UNUSED(const std::string &config_filename),
                                        const std::string &factory_config_path,
                                        void *system_context,
                                        const std::string &app_group_id,
                                        bool_t main_core) const {
	return _createSharedCore(NULL, factory_config_path, factory_config_path, NULL, system_context, FALSE, app_group_id,
	                         main_core);
}

LinphoneCore *Factory::createCoreWithConfig(LinphoneCoreCbs *cbs, LinphoneConfig *config) const {
	return _linphone_core_new_with_config(cbs, config, NULL, NULL, TRUE);
}

LinphoneCore *Factory::createCoreWithConfig(LinphoneCoreCbs *cbs,
                                            LinphoneConfig *config,
                                            void *user_data,
                                            void *system_context) const {
	return _linphone_core_new_with_config(cbs, config, user_data, system_context, TRUE);
}

LinphoneCore *Factory::createCoreWithConfig(LinphoneConfig *config, void *system_context) const {
	return _linphone_core_new_with_config(NULL, config, NULL, system_context, FALSE);
}

LinphoneCore *Factory::createSharedCoreWithConfig(LinphoneConfig *config,
                                                  void *system_context,
                                                  const std::string &app_group_id,
                                                  bool_t main_core) const {
	return _linphone_core_new_shared_with_config(NULL, config, NULL, system_context, FALSE, app_group_id.c_str(),
	                                             main_core);
}

LinphoneCoreCbs *Factory::createCoreCbs() const {
	return _linphone_core_cbs_new();
}

LinphoneAddress *Factory::createAddress(const std::string &addr) const {
	return linphone_address_new(addr.c_str());
}

LinphoneParticipantDeviceCbs *Factory::createParticipantDeviceCbs() const {
	return linphone_participant_device_cbs_new();
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
LinphoneParticipantDeviceIdentity *Factory::createParticipantDeviceIdentity(const LinphoneAddress *address,
                                                                            const std::string &name) const {
#ifdef HAVE_ADVANCED_IM
	return linphone_participant_device_identity_new(address, name.c_str());
#else
	ms_warning("Advanced IM such as group chat is disabled");
	return NULL;
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

AuthInfo *Factory::createAuthInfo(const std::string &username,
                                  const std::string &userid,
                                  const std::string &passwd,
                                  const std::string &ha1,
                                  const std::string &realm,
                                  const std::string &domain) const {
	return new AuthInfo(username, userid, passwd, ha1, realm, domain);
}

AuthInfo *Factory::createAuthInfo(const std::string &username,
                                  const std::string &userid,
                                  const std::string &passwd,
                                  const std::string &ha1,
                                  const std::string &realm,
                                  const std::string &domain,
                                  const std::string &algorithm) const {
	return new AuthInfo(username, userid, passwd, ha1, realm, domain, algorithm);
}

AuthInfo *Factory::createAuthInfo(const std::string &username,
                                  std::shared_ptr<BearerToken> access_token,
                                  const std::string &realm) const {
	return new AuthInfo(username, access_token, realm);
}

std::string Factory::computeHa1ForAlgorithm(const std::string &userId,
                                            const std::string &password,
                                            const std::string &realm,
                                            const std::string &algorithm) const {
	return Utils::computeHa1ForAlgorithm(userId, password, realm, algorithm);
}

LinphoneCallCbs *Factory::createCallCbs() const {
	return _linphone_call_cbs_new();
}

LinphoneConferenceCbs *Factory::createConferenceCbs() const {
	return _linphone_conference_cbs_new();
}

LinphoneChatRoomCbs *Factory::createChatRoomCbs() const {
	return _linphone_chat_room_cbs_new();
}

LinphoneChatMessageCbs *Factory::createChatMessageCbs() const {
	return linphone_chat_message_cbs_new();
}

LinphoneMagicSearchCbs *Factory::createMagicSearchCbs() const {
	return linphone_magic_search_cbs_new();
}

LinphoneDictionary *Factory::createDictionary() const {
	return Dictionary::createCObject();
}

#ifdef HAVE_ADVANCED_IM
LinphoneEktInfo *Factory::createEktInfo() const {
	return EktInfo::createCObject();
}
#endif // HAVE_ADVANCED_IM

LinphoneAlertCbs *Factory::createAlertCbs() const {
	return AlertCbs::createCObject();
}
LinphoneSignalInformation *Factory::createSignalInformation() const {
	return SignalInformation::createCObject();
}
LinphoneVcard *Factory::createVcard() const {
	return Vcard::createCObject();
}

LinphoneVideoDefinition *Factory::createVideoDefinition(unsigned int width, unsigned int height) const {
	LinphoneVideoDefinition *supported = findSupportedVideoDefinition(width, height);
	return supported ? linphone_video_definition_clone(supported) : linphone_video_definition_new(width, height, NULL);
}

LinphoneVideoDefinition *Factory::createVideoDefinitionFromName(const std::string &name) const {
	unsigned int width = 0;
	unsigned int height = 0;
	LinphoneVideoDefinition *vdef = findSupportedVideoDefinitionByName(name);
	if (vdef != NULL) return linphone_video_definition_clone(vdef);
	if (sscanf(name.c_str(), "%ux%u", &width, &height) == 2) {
		return linphone_video_definition_new(width, height, NULL);
	}
	return linphone_video_definition_new(0, 0, NULL);
}

const bctbx_list_t *Factory::getSupportedVideoDefinitions() const {
	return mSupportedVideoDefinitions;
}

const bctbx_list_t *Factory::getRecommendedVideoDefinitions() const {
	bctbx_list_t *recommended = NULL;
	for (const bctbx_list_t *supported = getSupportedVideoDefinitions(); supported;
	     supported = bctbx_list_next(supported)) {
		LinphoneVideoDefinition *def = static_cast<LinphoneVideoDefinition *>(supported->data);
#if defined(__ANDROID__) || TARGET_OS_IPHONE // On mobile, we recommend a format of max 720p
		if (def->width * def->height <= MS_VIDEO_SIZE_720P_W * MS_VIDEO_SIZE_720P_H)
#endif
			recommended = bctbx_list_append(recommended, linphone_video_definition_ref(def));
	}
	return recommended;
}

LinphoneVideoDefinition *Factory::findSupportedVideoDefinition(unsigned int width, unsigned int height) const {
	return findSupportedVideoDefinition(width, height, false);
}

LinphoneVideoDefinition *
Factory::findSupportedVideoDefinition(unsigned int width, unsigned int height, bool silent) const {
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

	if (!silent && !found) {
		ms_warning("Couldn't find supported video definition for %ux%u", width, height);
	}
	return found;
}

LinphoneVideoDefinition *Factory::findSupportedVideoDefinitionByName(const std::string &name) const {
	const bctbx_list_t *item;
	const bctbx_list_t *supported = getSupportedVideoDefinitions();

	for (item = supported; item != NULL; item = bctbx_list_next(item)) {
		LinphoneVideoDefinition *svdef = (LinphoneVideoDefinition *)bctbx_list_get_data(item);
		if (strcasecmp(linphone_video_definition_get_name(svdef), name.c_str()) == 0) {
			return svdef;
		}
	}

	ms_error("Couldn't find a supported video definition for name [%s]", name.c_str());
	return NULL;
}

const std::string &Factory::getTopResourcesDir() const {
	return mTopResourcesDir;
}

void Factory::setTopResourcesDir(const std::string &path) {
	mTopResourcesDir = path;
}

const std::string &Factory::getDataResourcesDir() {
	if (!mDataResourcesDir.empty()) return mDataResourcesDir;
	if (!mTopResourcesDir.empty()) {
		mCachedDataResourcesDir = mTopResourcesDir + "/linphone";
	} else {
		mCachedDataResourcesDir.append(PACKAGE_DATA_DIR);
		mCachedDataResourcesDir.append("/linphone");
	}
	return mCachedDataResourcesDir;
}

void Factory::setDataResourcesDir(const std::string &path) {
	mDataResourcesDir = path;
}

const std::string &Factory::getSoundResourcesDir() {
	if (!mSoundResourcesDir.empty()) return mSoundResourcesDir;
	if (!mTopResourcesDir.empty()) {
		mCachedDataResourcesDir = mTopResourcesDir;
		mCachedDataResourcesDir.append("/sounds/linphone");
		return mCachedDataResourcesDir;
	}
	return mPackageSoundDir;
}

void Factory::setSoundResourcesDir(const std::string &path) {
	mSoundResourcesDir = path;
}

const std::string &Factory::getRingResourcesDir() {
	if (!mRingResourcesDir.empty()) return mRingResourcesDir;
	if (!mSoundResourcesDir.empty()) {
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

void Factory::setRingResourcesDir(const std::string &path) {
	mRingResourcesDir = path;
}

const std::string &Factory::getImageResourcesDir() {
	if (!mImageResourcesDir.empty()) return mImageResourcesDir;
	if (!mTopResourcesDir.empty()) {
		mCachedImageResourcesDir = mTopResourcesDir;
		mCachedImageResourcesDir.append("/images");
	} else {
		mCachedImageResourcesDir = PACKAGE_DATA_DIR;
		mCachedImageResourcesDir.append("/images");
	}
	return mCachedImageResourcesDir;
}

void Factory::setImageResourcesDir(const std::string &path) {
	mImageResourcesDir = path;
}

const std::string &Factory::getMspluginsDir() const {
	return mMspluginsDir;
}

void Factory::setMspluginsDir(const std::string &path) {
	mMspluginsDir = path;
}

const std::string &Factory::getLiblinphonePluginsDir() const {
	return mLiblinphonePluginsDir;
}

void Factory::setLiblinphonePluginsDir(const std::string &path) {
	mLiblinphonePluginsDir = path;
}

const std::string &Factory::getConfigDir(void *context) {
	if (!mConfigDir.empty()) return mConfigDir;
	mCachedConfigDir = LinphonePrivate::Paths::getPath(LinphonePrivate::Paths::Config, context);
	return mCachedConfigDir;
}

bool Factory::isConfigDirSet() const {
	return !mConfigDir.empty();
}

void Factory::setConfigDir(const std::string &path) {
	mConfigDir = path;
}

const std::string &Factory::getDataDir(void *context) {
	if (!mDataDir.empty()) return mDataDir;
	mCachedDataDir = LinphonePrivate::Paths::getPath(LinphonePrivate::Paths::Data, context);
	return mCachedDataDir;
}

bool Factory::isDataDirSet() const {
	return !mDataDir.empty();
}

void Factory::setDataDir(const std::string &path) {
	mDataDir = path;
}

const std::string &Factory::getDownloadDir(void *context) {
	if (!mDownloadDir.empty()) return mDownloadDir;
	mCachedDownloadDir = LinphonePrivate::Paths::getPath(LinphonePrivate::Paths::Download, context);
	return mCachedDownloadDir;
}

bool Factory::isDownloadDirSet() const {
	return !mDownloadDir.empty();
}

void Factory::setDownloadDir(const std::string &path) {
	mDownloadDir = path;
}

const std::string &Factory::getCacheDir(void *context) {
	if (!mCacheDir.empty()) return mCacheDir;

	// Default system Cache dir is generated in the Data dir as "cache"
	mCachedCacheDir = LinphonePrivate::Paths::getPath(LinphonePrivate::Paths::Data, context).append("cache/");
	// make sure the directory exists
	if (!bctbx_directory_exists(mCachedCacheDir.c_str())) {
		bctbx_mkdir(mCachedCacheDir.c_str());
	}
	return mCachedCacheDir;
}

bool Factory::isCacheDirSet() const {
	return !mCacheDir.empty();
}

void Factory::setCacheDir(const std::string &path) {
	mCacheDir = path;
}

LinphoneErrorInfo *Factory::createErrorInfo() const {
	return linphone_error_info_new();
}

LinphoneRange *Factory::createRange() const {
	return linphone_range_new();
}

LinphoneTransports *Factory::createTransports() const {
	return linphone_transports_new();
}

LinphoneVideoActivationPolicy *Factory::createVideoActivationPolicy() const {
	return linphone_video_activation_policy_new();
}

LinphoneContent *Factory::createContent() const {
	return linphone_content_new();
}

LinphoneContent *Factory::createContentFromFile(const std::string &file_path) const {
	std::string file_name = file_path.substr(file_path.find_last_of("/\\") + 1);
	auto content = FileContent::createCObject<FileContent>();
	linphone_content_set_file_path(content, file_path.c_str());
	linphone_content_set_name(content, file_name.c_str());
	return content;
}

LinphoneBuffer *Factory::createBuffer() const {
	return linphone_buffer_new();
}

LinphoneBuffer *Factory::createBufferFromData(const uint8_t *data, size_t size) const {
	return linphone_buffer_new_from_data(data, size);
}

LinphoneBuffer *Factory::createBufferFromString(const std::string &data) const {
	return linphone_buffer_new_from_string(data.c_str());
}

LinphoneConfig *Factory::createConfig(const std::string &path) const {
	return linphone_config_new(path.c_str());
}

LinphoneConfig *Factory::createConfigWithFactory(const std::string &path, const std::string &factory_path) const {
	return linphone_config_new_with_factory(path.c_str(), factory_path.c_str());
}

LinphoneConfig *Factory::createConfigFromString(const std::string &data) const {
	return linphone_config_new_from_buffer(data.c_str());
}

const bctbx_list_t *Factory::getDialPlans() const {
	return linphone_dial_plan_get_all_list();
}

void *Factory::getUserData() const {
	return mUserData;
}

void Factory::setUserData(void *data) {
	mUserData = data;
}

void Factory::setLogCollectionPath(const std::string &path) const {
	linphone_core_set_log_collection_path(path.c_str());
}

void Factory::enableLogCollection(LinphoneLogCollectionState state) const {
	linphone_core_enable_log_collection(state);
}

LinphoneTunnelConfig *Factory::createTunnelConfig() const {
	return linphone_tunnel_config_new();
}

LinphoneAccountCbs *Factory::createAccountCbs() const {
	return linphone_account_cbs_new();
}

LinphoneAccountManagerServicesRequestCbs *Factory::createAccountManagerServicesRequestCbs() const {
	return _linphone_account_manager_services_request_cbs_new();
}

LinphoneLoggingServiceCbs *Factory::createLoggingServiceCbs() const {
	return linphone_logging_service_cbs_new();
}

LinphonePlayerCbs *Factory::createPlayerCbs() const {
	return linphone_player_cbs_new();
}

LinphoneEventCbs *Factory::createEventCbs() const {
	return linphone_event_cbs_new();
}

LinphoneFriendCbs *Factory::createFriendCbs() const {
	return linphone_friend_cbs_new();
}

LinphoneFriendListCbs *Factory::createFriendListCbs() const {
	return linphone_friend_list_cbs_new();
}

LinphoneAccountCreatorCbs *Factory::createAccountCreatorCbs() const {
	return linphone_account_creator_cbs_new();
}

LinphoneXmlRpcRequestCbs *Factory::createXmlRpcRequestCbs() const {
	return linphone_xml_rpc_request_cbs_new();
}

LinphoneConferenceSchedulerCbs *Factory::createConferenceSchedulerCbs() const {
	return linphone_conference_scheduler_cbs_new();
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
bool_t Factory::isChatroomBackendAvailable(LinphoneChatRoomBackend chatroom_backend) const {
#ifdef HAVE_ADVANCED_IM
	return TRUE;
#else
	return (chatroom_backend != LinphoneChatRoomBackendFlexisipChat);
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

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

bool Factory::setVfsEncryption(const uint16_t encryptionModule, const uint8_t *secret, const size_t secretSize) {

	/* Check encryptionMpdule is valid */
	auto module = bctoolbox::EncryptionSuite::unset;
	switch (encryptionModule) {
		case LINPHONE_VFS_ENCRYPTION_UNSET: // do not use the encrypted VFS
			lWarning() << "Factory::SetVFSEncryption : disable encryption";
			bctbx_vfs_set_default(bctbx_vfs_get_standard());
			bctoolbox::VfsEncryption::openCallbackSet(nullptr);
			return true;
		case LINPHONE_VFS_ENCRYPTION_PLAIN: // use the encrypted VFS but write plain files
			lWarning() << "Factory::SetVFSEncryption : encryptionModule set to plain text";
			module = bctoolbox::EncryptionSuite::plain;
			break;
		case LINPHONE_VFS_ENCRYPTION_DUMMY:
			lWarning()
			    << "Factory::SetVFSEncryption : encryptionModule set to dummy: use this setting for testing only";
			module = bctoolbox::EncryptionSuite::dummy;
			break;
		case LINPHONE_VFS_ENCRYPTION_AES256GCM128_SHA256:
			lInfo() << "Factory::SetVFSEncryption : encryptionModule set to AES256GCM_SHA256";
			module = bctoolbox::EncryptionSuite::aes256gcm128_sha256;
			break;
		default:
			lError() << "Factory::SetVFSEncryption : encryptionModule " << std::hex << encryptionModule << " unknown";
			return false;
	}

	/* save the key */
	if (mEvfsMasterKey != nullptr) {
		bctbx_clean(mEvfsMasterKey->data(), mEvfsMasterKey->size());
	}
	mEvfsMasterKey = std::make_shared<std::vector<uint8_t>>(secret, secret + secretSize);

	// Set the default bctbx vfs to the encrypted one
	bctbx_vfs_set_default(&bctoolbox::bcEncryptedVfs);

	// Associate the VfsEncryption class callback
	bctoolbox::VfsEncryption::openCallbackSet([module, this](bctoolbox::VfsEncryption &settings) {
		bctbx_debug("Encrypted VFS: Open file %s, encryption is set to %s file. Current file's encryption module is %s",
		            settings.filenameGet().data(), encryptionSuiteString(module).data(),
		            encryptionSuiteString(settings.encryptionSuiteGet()).data());
		settings.encryptionSuiteSet(module); // This call will migrate plain files to encrypted ones if needed
		if (module != bctoolbox::EncryptionSuite::plain) { // do not set keys for plain module
			// settings.secretMaterialSet(*(mEvfsMasterKey));
			settings.secretMaterialSet(*mEvfsMasterKey);
		}
	});

	// not secret continuity check when plain is used
	if (module == bctoolbox::EncryptionSuite::plain) return true;

	// Check secret continuity mechanism:
	// in order to detect early - and be specific - an EVFS master key change (the EVFS master key is kept in the OS
	// keychain which may change it) check that a dedicated file exists:
	//  - if not create it (encrypted with current master key)
	//  - if it exists, open it. If we succeed, our key seems valid.
	if (!isDataDirSet()) {
		lWarning()
		    << "Factory::SetVFSEncryption cannot check the secret key continuity as data dir is not set in factory";
		return true;
	}
	std::string filePath(mDataDir);
	filePath.append("/EVFScheck.").append(encryptionSuiteString(module));
	if (bctbx_file_exist(filePath.c_str()) == 0) {
		auto fp = bctbx_file_open(&bctoolbox::bcEncryptedVfs, filePath.c_str(), "r");
		if (fp == NULL) {
			lError() << "Factory::SetVFSEncryptionError: secret key continuity check failure, unable to open "
			         << filePath;
			return false;
		}
		bctbx_file_close(fp);
		return true;
	} else { // the file does not exists yet, create it and write something random in it
		lInfo() << "Factory::SetVFSEncryption: secret key continuity file " << filePath << " created";
		auto fp = bctbx_file_open(&bctoolbox::bcEncryptedVfs, filePath.c_str(), "w");
		std::vector<uint8_t> randomContent(64);
		bctoolbox::RNG::cRandomize(randomContent.data(), randomContent.size());
		bctbx_file_write(fp, randomContent.data(), randomContent.size(), 0);
		bctbx_file_close(fp);
		return true;
	}
}

LinphoneDigestAuthenticationPolicy *Factory::createDigestAuthenticationPolicy() const {
	return linphone_digest_authentication_policy_new();
}

Factory::~Factory() {
	bctbx_list_free_with_data(mSupportedVideoDefinitions, (bctbx_list_free_func)linphone_video_definition_unref);

#ifdef HAVE_SQLITE
	// sqlite3 vfs is registered at factory creation, so unregister it when destroying it
	sqlite3_bctbx_vfs_unregister();
#endif

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

std::shared_ptr<BearerToken> Factory::createBearerToken(const std::string &token, time_t expirationTime) const {
	return BearerToken::create(token, expirationTime);
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
std::shared_ptr<ConferenceInfo> Factory::createConferenceInfoFromIcalendarContent(LinphoneContent *content) const {
#ifdef HAVE_ADVANCED_IM
	LinphonePrivate::ContentType contentType = Content::toCpp(content)->getContentType();
	if (!contentType.strongEqual(ContentType::Icalendar)) return nullptr;

	std::string filepath = "";
	if (linphone_content_is_file_encrypted(content)) {
		char *tmp = linphone_content_export_plain_file(content);
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
#else
	lWarning() << "createConferenceInfoFromIcalendarContent(): no ICS support, ADVANCED_IM is disabled.";
	return nullptr;
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

std::shared_ptr<ParticipantInfo> Factory::createParticipantInfo(const std::shared_ptr<const Address> &address) const {
	return ParticipantInfo::create(address);
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
LinphoneContent *Factory::createQRCode(const std::string &code,
                                       const unsigned int &width,
                                       const unsigned int &height,
                                       const unsigned int &margin) const {
#ifdef QRCODE_ENABLED
	int eccLevel = -1;
	ZXing::CharacterSet encoding = ZXing::CharacterSet::Unknown;
	ZXing::BarcodeFormat format = ZXing::BarcodeFormat::QRCode;

	if (code.empty()) {
		lError() << "Cannot generate a QRCode because the code is empty";
		return nullptr;
	}
	if (width == 0 || height == 0) {
		lError() << "Cannot generate a QRCode because sizes are 0";
		return nullptr;
	}

	auto writer = ZXing::MultiFormatWriter(format).setMargin((int)margin).setEncoding(encoding).setEccLevel(eccLevel);
	auto matrix = writer.encode(ZXing::TextUtfEncoding::FromUtf8(code), (int)width, (int)height);
	auto bitmap = ZXing::ToMatrix<uint8_t>(matrix);

	LinphoneContent *content = Factory::createContent();
	linphone_content_set_buffer(content, bitmap.data(), static_cast<size_t>(bitmap.width() * bitmap.height()));
	linphone_content_add_content_type_parameter(content, "height", std::to_string(bitmap.height()).c_str());
	linphone_content_add_content_type_parameter(content, "width", std::to_string(bitmap.width()).c_str());
	linphone_content_add_content_type_parameter(content, "margin", std::to_string(margin).c_str());

	return content;
#else
	lError() << "linphone_factory_create_qrcode() : not supported";
	return nullptr;
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
int Factory::writeQRCodeFile(const std::string &code,
                             const std::string &filePath,
                             const unsigned int &width,
                             const unsigned int &height,
                             const unsigned int &margin) const {
#ifdef JPEG_ENABLED
	FILE *outFile = NULL;
	size_t bytesWritten = 0;
	int exitCode = 0;
	int error = 0;
	tjhandle turboJpeg = nullptr;
	MSPicture yuvbuf;
	LinphoneContent *bitmapContent = nullptr;
	unsigned char *jpegBuffer = nullptr;
	unsigned long jpegSize = 0;
	uint8_t *buffer = nullptr;

	remove(filePath.c_str());
	outFile = fopen(filePath.c_str(), "wb");
	if (!outFile) {
		ms_error("Can't open %s for writing: %s\n", filePath.c_str(), strerror(errno));
		exitCode = -1;
		goto cleanMemory;
	}

	turboJpeg = tjInitCompress();
	if (!turboJpeg) {
		ms_error("TurboJpeg init error:%s", tjGetErrorStr());
		exitCode = -1;
		goto cleanMemory;
	}
	bitmapContent = createQRCode(code, width, height, margin);
	if (!bitmapContent) { // error has been specified by createQRCode()
		exitCode = -1;
		goto cleanMemory;
	}
	buffer = (uint8_t *)(linphone_content_get_buffer(bitmapContent));
	ms_yuv_buf_init(&yuvbuf, (int)width, (int)height, (int)width, buffer);

	error = tjCompressFromYUVPlanes(
	    turboJpeg,
	    // This auto cast has the purpose to support multiple versions of turboJPEG where it can be a const parameter.
	    bctoolbox::Utils::auto_cast<unsigned char **>(yuvbuf.planes), yuvbuf.w, yuvbuf.strides, yuvbuf.h, TJSAMP_GRAY,
	    &jpegBuffer, &jpegSize, 100, TJFLAG_ACCURATEDCT);

	if (error != 0) {
		ms_error("tjCompressFromYUVPlanes() failed: %s", tjGetErrorStr());
		exitCode = -1;
		goto cleanMemory;
	}
	bytesWritten = fwrite(jpegBuffer, 1, jpegSize, outFile);
	if (bytesWritten == 0 || bytesWritten != jpegSize) {
		ms_error("Error writing QRCode written bytes : %zu [%s]", bytesWritten, strerror(errno));
		exitCode = -1;
	}

cleanMemory:
	if (jpegBuffer != NULL) tjFree(jpegBuffer);
	if (bitmapContent) linphone_content_unref(bitmapContent);
	if (turboJpeg) {
		error = tjDestroy(turboJpeg);
		if (error != 0) {
			ms_error("TurboJpeg destroy error:%s", tjGetErrorStr());
		}
	}
	if (outFile) fclose(outFile);

	return exitCode;
#else
	return -2;
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

LINPHONE_END_NAMESPACE
