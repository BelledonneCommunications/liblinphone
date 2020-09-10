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

 #ifndef FACTORY_H_
 #define FACTORY_H_

#include "linphone/factory.h"

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

class Factory : public bellesip::HybridObject<LinphoneFactory, Factory> {
public:
	Factory ();
  ~Factory ();

  static LinphoneFactory *create(void);

  static void uninit(Factory *obj);

  //BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneFactory);
  /*BELLE_SIP_INSTANCIATE_VPTR(LinphoneFactory, belle_sip_object_t,
  	linphone_factory_uninit, // destroy
  	NULL, // clone
  	NULL, // Marshall
  	FALSE
  );*/

  //static LinphoneFactory *_factory = NULL;

  //static void _linphone_factory_destroying_cb(void);

  #define ADD_SUPPORTED_VIDEO_DEFINITION(factory, width, height, name) \
  	(factory)->supported_video_definitions = bctbx_list_append((factory)->supported_video_definitions, \
  		linphone_video_definition_new(width, height, name))

  static void initializeSupportedVideoDefinitions(Factory *factory);

  static Factory *get(void);

  static void clean(void);

  LinphoneCore *_createCore (
    LinphoneCoreCbs *cbs,
  	const char *config_path,
  	const char *factory_config_path,
  	void *user_data,
  	void *system_context,
  	bool_t automatically_start
  );

  LinphoneCore *_createSharedCore (
  	LinphoneCoreCbs *cbs,
  	const char *config_filename,
  	const char *factory_config_path,
  	void *user_data,
  	void *system_context,
  	bool_t automatically_start,
  	const char *app_group_id,
  	bool_t main_core
  );

  LinphoneCore *createCore (
  	LinphoneCoreCbs *cbs,
  	const char *config_path,
  	const char *factory_config_path
  );

  LinphoneCore *createCore (
  	LinphoneCoreCbs *cbs,
  	const char *config_path,
  	const char *factory_config_path,
  	void *user_data,
  	void *system_context
  );

  LinphoneCore *createCore (
  	const char *config_path,
  	const char *factory_config_path,
  	void *system_context
  );

  LinphoneCore *createSharedCore (
  	const char *config_filename,
  	const char *factory_config_path,
  	void *system_context,
  	const char *app_group_id,
  	bool_t main_core
  );

  LinphoneCore *createCoreWithConfig (
  	LinphoneCoreCbs *cbs,
  	LinphoneConfig *config
  );

  LinphoneCore *createCoreWithConfig (
  	LinphoneCoreCbs *cbs,
  	LinphoneConfig *config,
  	void *user_data,
  	void *system_context
  );

  LinphoneCore *createCoreWithConfig (
  	LinphoneConfig *config,
  	void *system_context
  );

  LinphoneCore *createSharedCoreWithConfig (
  	LinphoneConfig *config,
  	void *system_context,
  	const char *app_group_id,
  	bool_t main_core
  );

  LinphoneCoreCbs *createCoreCbs();

  LinphoneAddress *createAddress(const char *addr);

  LinphoneParticipantDeviceIdentity *createParticipantDeviceIdentity(const LinphoneAddress *address,
  	const char *name
  );

  LinphoneAuthInfo *createAuthInfo(const char *username, const char *userid, const char *passwd, const char *ha1, const char *realm, const char *domain);

  LinphoneAuthInfo *createAuthInfo(const char *username, const char *userid, const char *passwd, const char *ha1, const char *realm, const char *domain, const char *algorithm);

  LinphoneCallCbs * createCallCbs();

  LinphoneChatRoomCbs * createChatRoomCbs();

  LinphoneChatMessageCbs * createChatMessageCbs();

  LinphoneVcard *createVcard();

  LinphoneVideoDefinition * createVideoDefinition(unsigned int width, unsigned int height);

  LinphoneVideoDefinition * createVideoDefinitionFromName(const char *name);

  const bctbx_list_t * getSupportedVideoDefinitions();

  LinphoneVideoDefinition * findSupportedVideoDefinition(unsigned int width, unsigned int height);

  LinphoneVideoDefinition * findSupportedVideoDefinitionByName(const char *name);

  const char * getTopResourcesDir();

  void setTopResourcesDir(const char *path);

  const char * getDataResourcesDir();

  void setDataResourcesDir(const char *path);

  const char * getSoundResourcesDir();

  void setSoundResourcesDir(const char *path);

  const char * getRingResourcesDir();

  void setRingResourcesDir(const char *path);

  const char * getImageResourcesDir();

  void setImageResourcesDir(const char *path);

  const char * getMspluginsDir();

  void setMspluginsDir(const char *path);

  LinphoneErrorInfo *createErrorInfo();

  LinphoneRange *createRange();

  LinphoneTransports *createTransports();

  LinphoneVideoActivationPolicy *createVideoActivationPolicy();

  LinphoneContent *createContent();

  LinphoneBuffer *createBuffer();

  LinphoneBuffer *createBufferFromData(const uint8_t *data, size_t size);

  LinphoneBuffer *createBufferFromString(const char *data);

  LinphoneConfig *createConfig(const char *path);

  LinphoneConfig *createConfigWithFactory(const char *path, const char *factory_path);

  LinphoneConfig *createConfigFromString(const char *data);

  const bctbx_list_t * getDialPlans();

  void *getUserData();

  void setUserData(void *data);

  void setLogCollectionPath(const char *path);

  void enableLogCollection(LinphoneLogCollectionState state);

  LinphoneTunnelConfig *createTunnelConfig();

  LinphoneLoggingServiceCbs *createLoggingServiceCbs();

  LinphonePlayerCbs *createPlayerCbs();

  LinphoneEventCbs *createEventCbs();

  LinphoneFriendListCbs *createFriendListCbs();

  LinphoneAccountCreatorCbs *createAccountCreatorCbs();

  LinphoneXmlRpcRequestCbs *createXmlRpcRequestCbs();

  bool_t isChatroomBackendAvailable(LinphoneChatRoomBackend chatroom_backend);

  bool_t isDatabaseStorageAvailable();

  bool_t isImdnAvailable();

  const char *getConfigDir(void *context);

  const char *getDataDir(void *context);

  const char *getDownloadDir(void *context);

  void setVfsEncryption(const uint16_t encryptionModule, const uint8_t *secret, const size_t secretSize);

private:
  belle_sip_object_t mBase;

  bctbx_list_t *mSupportedVideoDefinitions;

  /*these are the directories set by the application*/
  std::string mTopResourcesDir;
  std::string mDataResourcesDir;
  std::string mSoundResourcesDir;
  std::string mRingResourcesDir;
  std::string mImageResourcesDir;
  std::string mMspluginsDir;

  /*these are the cached result computed from directories set by the application*/
  std::string mCachedDataResourcesDir;
  std::string mCachedSoundResourcesDir;
  std::string mCachedRingResourcesDir;
  std::string mCachedImageResourcesDir;
  std::string mCachedMspluginsDir;
  std::string mCachedConfigDir;
  std::string mCachedDataDir;
  std::string mCachedDownloadDir;
  //LinphoneErrorInfo* ei; useless ????????

  /* the EVFS encryption key */
  std::shared_ptr<std::vector<uint8_t>> mEvfsMasterKey; // use a shared_ptr as _LinphoneFactory is not really an object and vector destructor end up never being called otherwise
  void *mUserData;
};
LINPHONE_END_NAMESPACE

#endif // ifndef FACTORY_H_
