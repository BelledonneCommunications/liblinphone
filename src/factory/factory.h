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

#include <belle-sip/object++.hh>
#include "linphone/api/c-types.h"
#include "c-wrapper/c-wrapper.h"

// TODO: From coreapi. Remove me later.
#include "private.h"


LINPHONE_BEGIN_NAMESPACE

class Factory : public bellesip::HybridObject<LinphoneFactory, Factory> {
public:

  Factory ();
  ~Factory ();

  static void initializeSupportedVideoDefinitions(Factory *factory);

  static std::shared_ptr<Factory> get(void);

  static void clean(void);

  LinphoneCore *_createCore (
    LinphoneCoreCbs *cbs,
  	const char *config_path,
  	const char *factory_config_path,
  	void *user_data,
  	void *system_context,
  	bool_t automatically_start
  ) const;

  LinphoneCore *_createSharedCore (
  	LinphoneCoreCbs *cbs,
  	const char *config_filename,
  	const char *factory_config_path,
  	void *user_data,
  	void *system_context,
  	bool_t automatically_start,
  	const char *app_group_id,
  	bool_t main_core
  ) const;

  LinphoneCore *createCore (
  	LinphoneCoreCbs *cbs,
  	const char *config_path,
  	const char *factory_config_path
  ) const;

  LinphoneCore *createCore (
  	LinphoneCoreCbs *cbs,
  	const char *config_path,
  	const char *factory_config_path,
  	void *user_data,
  	void *system_context
  ) const;

  LinphoneCore *createCore (
  	const char *config_path,
  	const char *factory_config_path,
  	void *system_context
  ) const;

  LinphoneCore *createSharedCore (
  	const char *config_filename,
  	const char *factory_config_path,
  	void *system_context,
  	const char *app_group_id,
  	bool_t main_core
  ) const;

  LinphoneCore *createCoreWithConfig (
  	LinphoneCoreCbs *cbs,
  	LinphoneConfig *config
  ) const;

  LinphoneCore *createCoreWithConfig (
  	LinphoneCoreCbs *cbs,
  	LinphoneConfig *config,
  	void *user_data,
  	void *system_context
  ) const;

  LinphoneCore *createCoreWithConfig (
  	LinphoneConfig *config,
  	void *system_context
  ) const;

  LinphoneCore *createSharedCoreWithConfig (
  	LinphoneConfig *config,
  	void *system_context,
  	const char *app_group_id,
  	bool_t main_core
  ) const;

  LinphoneCoreCbs *createCoreCbs() const ;

  LinphoneAddress *createAddress(const char *addr) const;

  LinphoneParticipantDeviceIdentity *createParticipantDeviceIdentity(const LinphoneAddress *address,
  	const char *name
  ) const;

  LinphoneAuthInfo *createAuthInfo(const char *username, const char *userid, const char *passwd, const char *ha1, const char *realm, const char *domain) const;

  LinphoneAuthInfo *createAuthInfo(const char *username, const char *userid, const char *passwd, const char *ha1, const char *realm, const char *domain, const char *algorithm) const;

  LinphoneCallCbs * createCallCbs() const;

  LinphoneConferenceCbs * createConferenceCbs() const;

  LinphoneChatRoomCbs * createChatRoomCbs() const;

  LinphoneChatMessageCbs * createChatMessageCbs() const;
  LinphoneMagicSearchCbs * createMagicSearchCbs() const;

  LinphoneVcard *createVcard() const;

  LinphoneVideoDefinition * createVideoDefinition(unsigned int width, unsigned int height) const;

  LinphoneVideoDefinition * createVideoDefinitionFromName(const char *name) const;

  const bctbx_list_t * getSupportedVideoDefinitions() const;

  LinphoneVideoDefinition * findSupportedVideoDefinition(unsigned int width, unsigned int height) const;

  LinphoneVideoDefinition * findSupportedVideoDefinitionByName(const char *name) const;

  const std::string & getTopResourcesDir() const;

  void setTopResourcesDir(const char *path);

  const std::string & getDataResourcesDir();

  void setDataResourcesDir(const char *path);

  const std::string & getSoundResourcesDir();

  void setSoundResourcesDir(const char *path);

  const std::string & getRingResourcesDir();

  void setRingResourcesDir(const char *path);

  const std::string & getImageResourcesDir();

  void setImageResourcesDir(const char *path);

  const std::string & getMspluginsDir() const;

  void setMspluginsDir(const char *path);

  LinphoneErrorInfo *createErrorInfo() const;

  LinphoneRange *createRange() const;

  LinphoneTransports *createTransports() const;

  LinphoneVideoActivationPolicy *createVideoActivationPolicy() const;

  LinphoneContent *createContent() const;

  LinphoneBuffer *createBuffer() const;

  LinphoneBuffer *createBufferFromData(const uint8_t *data, size_t size) const;

  LinphoneBuffer *createBufferFromString(const char *data) const;

  LinphoneConfig *createConfig(const char *path) const;

  LinphoneConfig *createConfigWithFactory(const char *path, const char *factory_path) const;

  LinphoneConfig *createConfigFromString(const char *data) const;

  const bctbx_list_t * getDialPlans() const;

  void *getUserData() const;

  void setUserData(void *data);

  void setLogCollectionPath(const char *path) const;

  void enableLogCollection(LinphoneLogCollectionState state) const;

  LinphoneTunnelConfig *createTunnelConfig() const;

  LinphoneAccountCbs *createAccountCbs() const;

  LinphoneLoggingServiceCbs *createLoggingServiceCbs() const;

  LinphonePlayerCbs *createPlayerCbs() const;

  LinphoneEventCbs *createEventCbs() const;

  LinphoneFriendListCbs *createFriendListCbs() const;

  LinphoneAccountCreatorCbs *createAccountCreatorCbs() const;

  LinphoneXmlRpcRequestCbs *createXmlRpcRequestCbs() const;

  bool_t isChatroomBackendAvailable(LinphoneChatRoomBackend chatroom_backend) const;

  bool_t isDatabaseStorageAvailable() const;

  bool_t isImdnAvailable() const;

  const std::string & getConfigDir(void *context);

  const std::string & getDataDir(void *context);

  const std::string & getDownloadDir(void *context);

  void setVfsEncryption(const uint16_t encryptionModule, const uint8_t *secret, const size_t secretSize);

protected:
  static void _DestroyingCb(void);
  static std::shared_ptr<Factory> instance;

  std::string mPackageSoundDir;
  std::string mPackageRingDir;
  std::string mPackageDataDir;

private:
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
