/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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

#ifndef _L_ACCOUNT_PARAMS_H_
#define _L_ACCOUNT_PARAMS_H_

#include <belle-sip/object++.hh>
#include "linphone/api/c-types.h"
#include "linphone/api/c-push-notification-config.h"
#include "linphone/types.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class AccountParams : public bellesip::HybridObject<LinphoneAccountParams, AccountParams> {
	friend class Account;

public:
	AccountParams (LinphoneCore *lc);
	AccountParams (LinphoneCore *lc, int index);
	AccountParams (const AccountParams &other);
	~AccountParams ();

	AccountParams* clone () const override;

	// Setters
	void setExpires (int expires);
	void setQualityReportingInterval (int qualityReportingInterval);
	void setPublishExpires (int publishExpires);
	void setAvpfRrInterval (uint8_t avpfRrInterval);
	void setRegisterEnabled (bool enable);
	void setDialEscapePlusEnabled (bool enable);
	void setQualityReportingEnabled (bool enable);
	void setPublishEnabled (bool enable);
	void setOutboundProxyEnabled (bool enable);
	void setPushNotificationAllowed (bool allow);
	void setRemotePushNotificationAllowed (bool allow);
	void setUseInternationalPrefixForCallsAndChats (bool enable);
	void setUserData (void *userData);
	void setInternationalPrefix (const std::string &internationalPrefix);
	void setProxy (const std::string &proxy);
	void setRealm (const std::string &realm);
	void setQualityReportingCollector (const std::string &qualityReportingCollector);
	void setContactParameters (const std::string &contactParameters);
	void setContactUriParameters (const std::string &contactUriParameters);
	void setRefKey (const std::string &refKey);
	void setDependsOn (const std::string &dependsOn);
	void setIdKey (const std::string &idKey);
	void setConferenceFactoryUri (const std::string &conferenceFactoryUri);
	void setFileTranferServer (const std::string &fileTransferServer);
	void setPrivacy (LinphonePrivacyMask privacy);
	void setAvpfMode (LinphoneAVPFMode avpfMode);
	void setNatPolicy (LinphoneNatPolicy *natPolicy);
	void setPushNotificationConfig (LinphonePushNotificationConfig *pushNotificationConfig);
	LinphoneStatus setIdentityAddress (const LinphoneAddress* identityAddress);
	LinphoneStatus setRoutes (const bctbx_list_t *routes);
	LinphoneStatus setRoutesFromStringList (const bctbx_list_t *routes);

	// Getters
	int getExpires () const;
	int getQualityReportingInterval () const;
	int getPublishExpires () const;
	uint8_t getAvpfRrInterval () const;
	bool getRegisterEnabled () const;
	bool getDialEscapePlusEnabled () const;
	bool getQualityReportingEnabled () const;
	bool getPublishEnabled () const;
	bool getOutboundProxyEnabled () const;
	bool getPushNotificationAllowed () const;
	bool getRemotePushNotificationAllowed () const;
	bool getUseInternationalPrefixForCallsAndChats () const;
	bool isPushNotificationAvailable () const;
	void* getUserData () const;
	const std::string& getInternationalPrefix () const;
	const std::string& getProxy () const;
	const std::string& getRealm () const;
	const std::string& getQualityReportingCollector () const;
	const std::string& getContactParameters () const;
	const std::string& getContactUriParameters () const;
	const std::string& getRefKey () const;
	const std::string& getDependsOn () const;
	const std::string& getIdKey () const;
	const std::string& getConferenceFactoryUri () const;
	const std::string& getFileTransferServer () const;
	const std::string& getIdentity () const;
	const char* getDomain () const;
	const bctbx_list_t* getRoutes () const;
	const bctbx_list_t* getRoutesString () const;
	LinphonePrivacyMask getPrivacy () const;
	LinphoneAddress* getIdentityAddress () const;
	LinphoneAVPFMode getAvpfMode () const;
	LinphoneNatPolicy* getNatPolicy () const;
	LinphonePushNotificationConfig *getPushNotificationConfig () const;

	// Other
	LinphoneStatus setServerAddress (const LinphoneAddress *serverAddr);
	const LinphoneAddress *getServerAddress() const;

	LinphoneStatus setServerAddressAsString (const std::string &serverAddr);
	const std::string& getServerAddressAsString () const;

	void setTransport (LinphoneTransportType transport);
	LinphoneTransportType getTransport () const;

	void writeToConfigFile (LinphoneConfig *config, int index);

private:
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
	bool mUseInternationalPrefixForCallsAndChats;

	void *mUserData;

	std::string mInternationalPrefix;
	std::string mProxy;
	std::string mRealm;
	std::string mQualityReportingCollector;
	std::string mContactParameters;
	std::string mContactUriParameters;
	std::string mRefKey;
	std::string mDependsOn;
	std::string mIdKey;
	std::string mConferenceFactoryUri;
	std::string mFileTransferServer;
	std::string mIdentity;

	bctbx_list_t *mRoutes = nullptr;
	bctbx_list_t *mRoutesString = nullptr;

	LinphonePrivacyMask mPrivacy;

	LinphoneAddress *mIdentityAddress = nullptr;
	LinphoneAddress *mProxyAddress = nullptr;

	LinphoneAVPFMode mAvpfMode;

	LinphoneNatPolicy *mNatPolicy = nullptr;

	LinphonePushNotificationConfig *mPushNotificationConfig = nullptr;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_ACCOUNT_PARAMS_H_
