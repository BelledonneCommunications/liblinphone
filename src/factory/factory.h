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

#ifndef FACTORY_H_
#define FACTORY_H_

#include <belle-sip/object++.hh>
#include "linphone/api/c-types.h"
#include "c-wrapper/c-wrapper.h"
#include "conference/conference-info.h"

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
			const std::string& config_path,
			const std::string& factory_config_path,
			void *user_data,
			void *system_context,
			bool_t automatically_start
			) const;
	
	LinphoneCore *_createSharedCore (
			LinphoneCoreCbs *cbs,
			const std::string& config_filename,
			const std::string& factory_config_path,
			void *user_data,
			void *system_context,
			bool_t automatically_start,
			const std::string& app_group_id,
			bool_t main_core
			) const;
	
	LinphoneCore *createCore (
			LinphoneCoreCbs *cbs,
			const std::string& config_path,
			const std::string& factory_config_path
			) const;
	
	LinphoneCore *createCore (
			LinphoneCoreCbs *cbs,
			const std::string& config_path,
			const std::string& factory_config_path,
			void *user_data,
			void *system_context
			) const;
	
	LinphoneCore *createCore (
			const std::string& config_path,
			const std::string& factory_config_path,
			void *system_context
			) const;
	
	LinphoneCore *createSharedCore (
			const std::string& config_filename,
			const std::string& factory_config_path,
			void *system_context,
			const std::string& app_group_id,
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
			const std::string& app_group_id,
			bool_t main_core
			) const;
	
	LinphoneCoreCbs *createCoreCbs() const ;
	LinphoneAddress *createAddress(const std::string& addr) const;
	LinphoneParticipantDeviceIdentity *createParticipantDeviceIdentity(const LinphoneAddress *address,
																	   const std::string& name
																	   ) const;
	LinphoneAuthInfo *createAuthInfo(const std::string& username, const std::string& userid, const std::string& passwd, const std::string& ha1, const std::string& realm, const std::string& domain) const;
	LinphoneAuthInfo *createAuthInfo(const std::string& username, const std::string& userid, const std::string& passwd, const std::string& ha1, const std::string& realm, const std::string& domain, const std::string& algorithm) const;
	LinphoneCallCbs * createCallCbs() const;
	LinphoneConferenceCbs * createConferenceCbs() const;
	LinphoneParticipantDeviceCbs * createParticipantDeviceCbs() const;
	LinphoneChatRoomCbs * createChatRoomCbs() const;
	LinphoneChatMessageCbs * createChatMessageCbs() const;
	LinphoneMagicSearchCbs * createMagicSearchCbs() const;
	
	LinphoneDigestAuthenticationPolicy *createDigestAuthenticationPolicy() const;
	
	LinphoneVcard *createVcard() const;
	
	LinphoneVideoDefinition * createVideoDefinition(unsigned int width, unsigned int height) const;
	
	LinphoneVideoDefinition * createVideoDefinitionFromName(const std::string& name) const;
	
	const bctbx_list_t * getSupportedVideoDefinitions() const;
	const bctbx_list_t* getRecommendedVideoDefinitions()const;	// Return a list of ref on supported video definition as this list is not updatable.
	
	LinphoneVideoDefinition * findSupportedVideoDefinition(unsigned int width, unsigned int height) const;
	
	LinphoneVideoDefinition * findSupportedVideoDefinitionByName(const std::string& name) const;
	
	const std::string & getTopResourcesDir() const;
	void setTopResourcesDir(const std::string& path);
	
	const std::string & getDataResourcesDir();
	void setDataResourcesDir(const std::string& path);
	
	const std::string & getSoundResourcesDir();
	void setSoundResourcesDir(const std::string& path);
	
	const std::string & getRingResourcesDir();
	void setRingResourcesDir(const std::string& path);
	
	const std::string & getImageResourcesDir();
	void setImageResourcesDir(const std::string& path);
	
	const std::string & getMspluginsDir() const;
	void setMspluginsDir(const std::string& path);
	
	LinphoneErrorInfo *createErrorInfo() const;
	LinphoneRange *createRange() const;
	LinphoneTransports *createTransports() const;
	LinphoneVideoActivationPolicy *createVideoActivationPolicy() const;
	LinphoneContent *createContent() const;
	LinphoneBuffer *createBuffer() const;
	LinphoneBuffer *createBufferFromData(const uint8_t *data, size_t size) const;
	LinphoneBuffer *createBufferFromString(const std::string& data) const;
	LinphoneConfig *createConfig(const std::string& path) const;
	LinphoneConfig *createConfigWithFactory(const std::string& path, const std::string& factory_path) const;
	LinphoneConfig *createConfigFromString(const std::string& data) const;
	
	const bctbx_list_t * getDialPlans() const;
	
	void *getUserData() const;
	void setUserData(void *data);
	
	void setLogCollectionPath(const std::string& path) const;
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
	bool isConfigDirSet() const;
	void setConfigDir(const std::string& path);
	
	const std::string & getDataDir(void *context);
	bool isDataDirSet() const;
	void setDataDir(const std::string& path);
	
	const std::string & getDownloadDir(void *context);
	bool isDownloadDirSet() const;
	void setDownloadDir(const std::string& path);
	
	const std::string & getCacheDir(void *context);
	bool isCacheDirSet() const;
	void setCacheDir(const std::string& path);

	void setVfsEncryption(const uint16_t encryptionModule, const uint8_t *secret, const size_t secretSize);
	
	std::shared_ptr<ConferenceInfo> createConferenceInfo() const;
	
	std::shared_ptr<ConferenceInfo> createConferenceInfoFromIcalendarContent(LinphoneContent *content) const;
	
	LinphoneConferenceSchedulerCbs *createConferenceSchedulerCbs() const;
	
	
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
	std::string mConfigDir;
	std::string mDataDir;
	std::string mDownloadDir;
	std::string mCacheDir;
	
	/*these are the cached result computed from directories set by the application*/
	std::string mCachedDataResourcesDir;
	std::string mCachedSoundResourcesDir;
	std::string mCachedRingResourcesDir;
	std::string mCachedImageResourcesDir;
	std::string mCachedMspluginsDir;
	std::string mCachedConfigDir;
	std::string mCachedDataDir;
	std::string mCachedDownloadDir;
	std::string mCachedCacheDir;
	
	//LinphoneErrorInfo* ei; useless ????????
	
	/* the EVFS encryption key */
	std::shared_ptr<std::vector<uint8_t>> mEvfsMasterKey; // use a shared_ptr as _LinphoneFactory is not really an object and vector destructor end up never being called otherwise
	void *mUserData;
};
LINPHONE_END_NAMESPACE

#endif // ifndef FACTORY_H_
