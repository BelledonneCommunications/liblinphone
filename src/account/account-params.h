/*
 * Copyright (c) 2010-2025 Belledonne Communications SARL.
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

#ifndef _L_ACCOUNT_PARAMS_H_
#define _L_ACCOUNT_PARAMS_H_

#include "belle-sip/object++.hh"

#include "address/address.h"
#include "c-wrapper/list-holder.h"
#include "linphone/api/c-push-notification-config.h"
#include "linphone/api/c-types.h"
#include "linphone/types.h"
#include "utils/custom-params.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE
class PushNotificationConfig;
class NatPolicy;

class LINPHONE_PUBLIC AccountParams : public bellesip::HybridObject<LinphoneAccountParams, AccountParams>,
                                      public CustomParams {
	friend class Account;

public:
	AccountParams(LinphoneCore *lc, bool useDefaultValues);
	AccountParams(LinphoneCore *lc, int index);
	AccountParams(const AccountParams &other);
	virtual ~AccountParams();

	AccountParams *clone() const override;

	// Setters
	void setExpires(int expires);
	void setQualityReportingInterval(int qualityReportingInterval);
	void setPublishExpires(int publishExpires);
	void setAvpfRrInterval(uint8_t avpfRrInterval);
	void setRegisterEnabled(bool enable);
	void setDialEscapePlusEnabled(bool enable);
	void setQualityReportingEnabled(bool enable);
	void setPublishEnabled(bool enable);
	void setOutboundProxyEnabled(bool enable);
	void setPushNotificationAllowed(bool allow);
	void setRemotePushNotificationAllowed(bool allow);
	void setForceRegisterOnPushNotification(bool allow);
	void setUnregisterAtStop(bool unregister);
	void setUseInternationalPrefixForCallsAndChats(bool enable);
	void setCpimMessagesAllowedInBasicChatRooms(bool allow);
	void setUserData(void *userData);
	void setInternationalPrefix(const std::string &internationalPrefix);
	void setInternationalPrefixIsoCountryCode(const std::string &internationalPrefixIsoCountryCode);
	void setProxy(const std::string &proxy);
	void setRealm(const std::string &realm);
	void setQualityReportingCollector(const std::string &qualityReportingCollector);
	void setContactParameters(const std::string &contactParameters);
	void setContactUriParameters(const std::string &contactUriParameters);
	void setRefKey(const std::string &refKey);
	void setDependsOn(const std::string &dependsOn);
	void setIdKey(const std::string &idKey);
	void setConferenceFactoryUri(const std::string &conferenceFactoryUri);
	void setConferenceFactoryAddress(const std::shared_ptr<const Address> &factoryAddress);
	void
	setAudioVideoConferenceFactoryAddress(const std::shared_ptr<const Address> &audioVideoConferenceFactoryAddress);
	void setCcmpServerUrl(const std::string &ccmpServerAddress);
	void setFileTransferServer(const std::string &fileTransferServer);
	void setPrivacy(LinphonePrivacyMask privacy);
	void setAvpfMode(LinphoneAVPFMode avpfMode);
	void setNatPolicy(const std::shared_ptr<NatPolicy> &natPolicy);
	void setPushNotificationConfig(PushNotificationConfig *pushNotificationConfig);
	LinphoneStatus setIdentityAddress(const std::shared_ptr<const Address> &identityAddress);
	LinphoneStatus setRoutes(const std::list<std::shared_ptr<Address>> &routes);
	LinphoneStatus setRoutesFromStringList(const bctbx_list_t *routes);
	void enableRtpBundle(bool value);
	void enableRtpBundleAssumption(bool value);
	void setCustomContact(const std::shared_ptr<const Address> &contact);
	void setLimeServerUrl(const std::string &url);
	/**
	 * valid algorithms are: c25519, c448 and c25519k512. Empty string is also valid, it will unset the value
	 * @param algo 	The base algorithm to use for lime on this account
	 */
	void setLimeAlgo(const std::string &algo);
	void setPictureUri(const std::string &uri);
	void setMwiServerAddress(const std::shared_ptr<Address> &address);
	void setVoicemailAddress(const std::shared_ptr<Address> &address);
	void setInstantMessagingEncryptionMandatory(bool mandatory);
	void setSupportedTagsList(const std::list<std::string> &supportedTagsList);

	// Getters
	int getExpires() const;
	int getQualityReportingInterval() const;
	int getPublishExpires() const;
	uint8_t getAvpfRrInterval() const;
	bool getRegisterEnabled() const;
	bool getDialEscapePlusEnabled() const;
	bool getQualityReportingEnabled() const;
	bool getPublishEnabled() const;
	bool getOutboundProxyEnabled() const;
	bool getPushNotificationAllowed() const;
	bool getRemotePushNotificationAllowed() const;
	bool getForceRegisterOnPushNotification() const;
	bool getUnregisterAtStop() const;
	bool getUseInternationalPrefixForCallsAndChats() const;
	bool isPushNotificationAvailable() const;
	bool isCpimMessagesAllowedInBasicChatRooms() const;
	void *getUserData() const;
	const std::string &getInternationalPrefix() const;
	const std::string &getInternationalPrefixIsoCountryCode() const;
	const std::string &getProxy() const;
	const std::string &getRealm() const;
	const std::string &getQualityReportingCollector() const;
	const std::string &getContactParameters() const;
	const std::string &getContactUriParameters() const;
	const std::string &getRefKey() const;
	const std::string &getDependsOn() const;
	const std::string &getIdKey() const;
	std::shared_ptr<const Address> getConferenceFactoryAddress() const;
	std::shared_ptr<const Address> getAudioVideoConferenceFactoryAddress() const;
	const char *getCcmpServerUrlCstr() const;
	const std::string &getCcmpServerUrl() const;
	const char *getCcmpUserIdCstr() const;
	const std::string &getCcmpUserId() const;
	const std::string &getFileTransferServer() const;
	const std::string &getIdentity() const;
	const char *getDomainCstr() const;
	const char *getConferenceFactoryCstr() const;
	const std::string getDomain() const;
	const std::list<std::shared_ptr<Address>> &getRoutes() const;
	const std::list<std::string> getRoutesString() const;
	const bctbx_list_t *getRoutesCString() const;
	LinphonePrivacyMask getPrivacy() const;
	std::shared_ptr<Address> getIdentityAddress() const;
	LinphoneAVPFMode getAvpfMode() const;
	std::shared_ptr<NatPolicy> getNatPolicy() const;
	PushNotificationConfig *getPushNotificationConfig() const;
	bool rtpBundleEnabled() const;
	bool rtpBundleAssumptionEnabled() const;
	std::shared_ptr<const Address> getCustomContact() const;
	const std::string &getLimeServerUrl() const;
	const std::string &getLimeAlgo() const;
	const std::string &getPictureUri() const;
	std::shared_ptr<const Address> getMwiServerAddress() const;
	std::shared_ptr<const Address> getVoicemailAddress() const;
	bool isInstantMessagingEncryptionMandatory() const;
	const std::list<std::string> &getSupportedTagsList() const;
	const bctbx_list_t *getSupportedTagsCList() const;
	bool useSupportedTags() const;

	// Other
	LinphoneStatus setServerAddress(const std::shared_ptr<const Address> &serverAddr);
	std::shared_ptr<const Address> getServerAddress() const;

	LinphoneStatus setServerAddressAsString(const std::string &serverAddr);
	const std::string &getServerAddressAsString() const;

	void setTransport(LinphonePrivate::Transport transport);
	LinphonePrivate::Transport getTransport() const;

	void writeToConfigFile(LinphoneConfig *config, int index);

private:
	void updateRoutesCString();
	void setCustomContact(const std::string &contact);
	const char *getMwiServerAddressCstr() const;
	const char *getVoicemailAddressCstr() const;

	int mExpires;
	int mQualityReportingInterval;
	int mPublishExpires;

	uint8_t mAvpfRrInterval;

	bool mRegisterEnabled;
	bool mDialEscapePlusEnabled;
	bool mQualityReportingEnabled;
	bool mPublishEnabled;
	bool mPushNotificationAllowed;
	bool mRemotePushNotificationAllowed;
	bool mForceRegisterOnPush;
	bool mUnregisterAtStop;
	bool mUseInternationalPrefixForCallsAndChats;
	bool mRtpBundleEnabled;
	bool mRtpBundleAssumption;
	bool mAllowCpimMessagesInBasicChatRooms;
	bool mInstantMessagingEncryptionMandatory;

	void *mUserData;

	mutable char *mConferenceFactoryAddressCstr = nullptr;
	mutable char *mMwiServerAddressCstr = nullptr;
	mutable char *mVoicemailAddressCstr = nullptr;
	mutable char *mCcmpServerUrlCstr = nullptr;
	mutable char *mCcmpUserIdCstr = nullptr;

	std::string mInternationalPrefix;
	std::string mInternationalPrefixIsoCountryCode;
	std::string mProxy;
	std::string mRealm;
	std::string mQualityReportingCollector;
	std::string mContactParameters;
	std::string mContactUriParameters;
	std::string mRefKey;
	std::string mDependsOn;
	std::string mIdKey;
	std::string mFileTransferServer;
	std::string mLimeServerUrl;
	std::string mLimeAlgo;
	std::string mIdentity;
	std::string mPictureUri;
	std::string mCcmpServerUrl;
	mutable std::string mCcmpUserId;

	std::list<std::shared_ptr<Address>> mRoutes;
	bctbx_list_t *mRoutesCString = nullptr;

	LinphonePrivacyMask mPrivacy;

	std::shared_ptr<Address> mIdentityAddress = nullptr;
	std::shared_ptr<Address> mProxyAddress = nullptr;

	LinphoneAVPFMode mAvpfMode;

	std::shared_ptr<NatPolicy> mNatPolicy = nullptr;

	PushNotificationConfig *mPushNotificationConfig;

	std::shared_ptr<Address> mConferenceFactoryAddress = nullptr;
	std::shared_ptr<Address> mAudioVideoConferenceFactoryAddress = nullptr;
	std::shared_ptr<Address> mCustomContact = nullptr;
	std::shared_ptr<Address> mMwiServerAddress = nullptr;
	std::shared_ptr<Address> mVoicemailAddress = nullptr;

	ListHolder<std::string> mSupportedTagsList = {};
	bool mUseSupportedTags = false;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_ACCOUNT_PARAMS_H_
