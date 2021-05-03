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
 * but WITHOUT ANY WARRANTY{
}
 without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "account-params.h"
#include "c-wrapper/internal/c-tools.h"
#include "linphone/api/c-address.h"
#include "linphone/nat_policy.h"
#include "linphone/types.h"
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

static string generate_account_id() {
	char id[17] = {0};
	belle_sip_random_token(id, 16);
	return string("proxy_config_").append(id); // TODO: change to account
}

AccountParams::AccountParams (LinphoneCore *lc) {
	mExpires = lc ? linphone_config_get_default_int(lc->config, "proxy", "reg_expires", 3600) : 3600;
	mRegisterEnabled = lc ? !!linphone_config_get_default_int(lc->config, "proxy", "reg_sendregister", 1) : 1;
	mInternationalPrefix = lc ? linphone_config_get_default_string(lc->config,"proxy","dial_prefix", "") : "";
	mUseInternationalPrefixForCallsAndChats = lc ? !!linphone_config_get_default_int(lc->config, "proxy", "use_dial_prefix_for_calls_and_chats", true) : true;
	mDialEscapePlusEnabled = lc ? !!linphone_config_get_default_int(lc->config, "proxy", "dial_escape_plus", false) : false;
	mPrivacy = lc ? (LinphonePrivacyMask) linphone_config_get_default_int(lc->config, "proxy", "privacy", LinphonePrivacyDefault) : (LinphonePrivacyMask) LinphonePrivacyDefault;
	mIdentity = lc ? linphone_config_get_default_string(lc->config, "proxy", "reg_identity", "") : "";
	mIdentityAddress = !mIdentity.empty() ? linphone_address_new(mIdentity.c_str()) : nullptr;
	mProxy = lc ? linphone_config_get_default_string(lc->config, "proxy", "reg_proxy", "") : "";
	mProxyAddress = !mProxy.empty() ? linphone_address_new(mProxy.c_str()) : nullptr;
	string route = lc ? linphone_config_get_default_string(lc->config, "proxy", "reg_route", "") : "";
	mRoutes = !route.empty() ? bctbx_list_append(mRoutes, linphone_address_new(route.c_str())) : nullptr;
	mRoutesString = !route.empty() ? bctbx_list_append(mRoutesString, bctbx_strdup(route.c_str())) : nullptr;
	mRealm = lc ? linphone_config_get_default_string(lc->config, "proxy", "realm", "") : "";
	mQualityReportingEnabled = lc ? !!linphone_config_get_default_int(lc->config, "proxy", "quality_reporting_enabled", false) : false;
	mQualityReportingCollector = lc ? linphone_config_get_default_string(lc->config, "proxy", "quality_reporting_collector", "") : "";
	mQualityReportingInterval = lc ? linphone_config_get_default_int(lc->config, "proxy", "quality_reporting_interval", 0) : 0;
	mContactParameters = lc ? linphone_config_get_default_string(lc->config, "proxy", "contact_parameters", "") : "";
	mContactUriParameters = lc ? linphone_config_get_default_string(lc->config, "proxy", "contact_uri_parameters", "") : "";
	mAvpfMode = lc ? static_cast<LinphoneAVPFMode>(linphone_config_get_default_int(lc->config, "proxy", "avpf", LinphoneAVPFDefault)) : LinphoneAVPFDefault;
	mAvpfRrInterval = lc ? !!linphone_config_get_default_int(lc->config, "proxy", "avpf_rr_interval", 5) : 5;
	mPublishExpires = lc ? linphone_config_get_default_int(lc->config, "proxy", "publish_expires", -1) : -1;
	mPublishEnabled = lc ? !!linphone_config_get_default_int(lc->config, "proxy", "publish", false) : false;

	bool pushAllowedDefault = false;
	bool remotePushAllowedDefault = false;
#if defined(__ANDROID__) || TARGET_OS_IPHONE
	pushAllowedDefault = true;
#endif
	mPushNotificationAllowed = lc ? !!linphone_config_get_default_int(lc->config, "proxy", "push_notification_allowed", pushAllowedDefault) : pushAllowedDefault;
	mRemotePushNotificationAllowed = lc ? !!linphone_config_get_default_int(lc->config, "proxy", "remote_push_notification_allowed", remotePushAllowedDefault) : remotePushAllowedDefault;
	mRefKey = lc ? linphone_config_get_default_string(lc->config, "proxy", "refkey", "") : "";
	string natPolicyRef = lc ? linphone_config_get_default_string(lc->config, "proxy", "nat_policy_ref", "") : "";
	if (!natPolicyRef.empty()) {
		LinphoneNatPolicy *policy = linphone_config_create_nat_policy_from_section(lc->config, natPolicyRef.c_str());
		setNatPolicy(policy);
		if (policy) {
			linphone_nat_policy_unref(policy);
		} else {
			lError() << "Cannot create default nat policy with ref [" << natPolicyRef << "] for account [" << this << "]";
		}
	}
	mDependsOn = lc ? linphone_config_get_default_string(lc->config, "proxy", "depends_on", "") : "";
	string idkey = lc ? linphone_config_get_default_string(lc->config, "proxy", "idkey", "") : "";
	if (!idkey.empty()) {
		mIdKey = idkey;
	} else {
		mIdKey = generate_account_id();
	}
	string conferenceFactoryUri = lc ? linphone_config_get_default_string(lc->config, "proxy", "conference_factory_uri", "") : "";
	setConferenceFactoryUri(conferenceFactoryUri);

	if (lc && lc->push_config) {
		mPushNotificationConfig = linphone_push_notification_config_clone(lc->push_config);
	} else {
		mPushNotificationConfig = linphone_push_notification_config_new();
	}
}

AccountParams::AccountParams (LinphoneCore *lc, int index) : AccountParams(lc) {
	LpConfig *config = lc->config;

	char key[50];
	sprintf(key, "proxy_%i", index); // TODO: change to account

	mIdentity = linphone_config_get_string(config, key, "reg_identity", mIdentity.c_str());
	LinphoneAddress *identity_address = linphone_address_new(mIdentity.c_str());
	setIdentityAddress(identity_address);
	if (identity_address) linphone_address_unref(identity_address);

	setServerAddressAsString(linphone_config_get_string(config, key, "reg_proxy", getServerAddressAsString().c_str()));

	bctbx_list_t *routes = linphone_config_get_string_list(config, key, "reg_route", nullptr);
	if (routes != nullptr) {
		setRoutesFromStringList(routes);
		bctbx_list_free_with_data(routes, (bctbx_list_free_func)bctbx_free);
	}

	mRealm = linphone_config_get_string(config, key, "realm", mRealm.c_str());

	mQualityReportingEnabled = !!linphone_config_get_int(config, key, "quality_reporting_enabled", mQualityReportingEnabled);
	mQualityReportingCollector = linphone_config_get_string(config, key, "quality_reporting_collector", mQualityReportingCollector.c_str());
	mQualityReportingInterval = linphone_config_get_int(config, key, "quality_reporting_interval", mQualityReportingInterval);

	mContactParameters = linphone_config_get_string(config, key, "contact_parameters", mContactParameters.c_str());
	mContactUriParameters = linphone_config_get_string(config, key, "contact_uri_parameters", mContactUriParameters.c_str());
	// Use an Address to avoid rewriting a whole new parser to get the push parameters from scratch
	Address lAddr(string("sip:dummy;" + mContactUriParameters));
	
	linphone_push_notification_config_set_provider(mPushNotificationConfig, lAddr.getUriParamValue("pn-provider").c_str());
	linphone_push_notification_config_set_prid(mPushNotificationConfig, lAddr.getUriParamValue("pn-prid").c_str());
	linphone_push_notification_config_set_param(mPushNotificationConfig, lAddr.getUriParamValue("pn-param").c_str());
	
	mExpires = linphone_config_get_int(config, key, "reg_expires", mExpires);
	mRegisterEnabled = !!linphone_config_get_int(config, key, "reg_sendregister", mRegisterEnabled);
	mPublishEnabled = !!linphone_config_get_int(config, key, "publish", mPublishEnabled);
	setPushNotificationAllowed(!!linphone_config_get_int(config, key, "push_notification_allowed", mPushNotificationAllowed));
	setRemotePushNotificationAllowed(!!linphone_config_get_int(config, key, "remote_push_notification_allowed", mRemotePushNotificationAllowed));
	mAvpfMode = static_cast<LinphoneAVPFMode>(linphone_config_get_int(config, key, "avpf", static_cast<int>(mAvpfMode)));
	mAvpfRrInterval = (uint8_t) linphone_config_get_int(config, key, "avpf_rr_interval", (int) mAvpfRrInterval);
	mDialEscapePlusEnabled = !!linphone_config_get_int(config, key, "dial_escape_plus", mDialEscapePlusEnabled);
	mInternationalPrefix = linphone_config_get_string(config, key, "dial_prefix", mInternationalPrefix.c_str());
	mUseInternationalPrefixForCallsAndChats = !!linphone_config_get_int(config, key, "use_dial_prefix_for_calls_and_chats", mUseInternationalPrefixForCallsAndChats);

	mPrivacy = static_cast<LinphonePrivacyMask>(linphone_config_get_int(config, key, "privacy", static_cast<int>(mPrivacy)));

	mRefKey = linphone_config_get_string(config, key, "refkey", mRefKey.c_str());
	mIdKey = linphone_config_get_string(config, key, "idkey", mRefKey.c_str());
	if (mIdKey.empty()) {
		mIdKey = generate_account_id();
		lWarning() << "generated proxyconfig idkey = [" << mIdKey << "]";
	}
	mDependsOn = linphone_config_get_string(config, key, "depends_on", mDependsOn.c_str());

	mPublishExpires = linphone_config_get_int(config, key, "publish_expires", mPublishExpires);

	const char *nat_policy_ref = linphone_config_get_string(config, key, "nat_policy_ref", NULL);
	if (nat_policy_ref != NULL) {
		if (mNatPolicy) linphone_nat_policy_unref(mNatPolicy);
		mNatPolicy = linphone_core_create_nat_policy_from_config(lc, nat_policy_ref);
	}

	mConferenceFactoryUri = linphone_config_get_string(config, key, "conference_factory_uri", mConferenceFactoryUri.c_str());
}

AccountParams::AccountParams (const AccountParams &other) : HybridObject(other) {
	mExpires = other.mExpires;
	mQualityReportingInterval = other.mQualityReportingInterval;
	mPublishExpires = other.mPublishExpires;
	mAvpfRrInterval = other.mAvpfRrInterval;

	mRegisterEnabled = other.mRegisterEnabled;
	mDialEscapePlusEnabled = other.mDialEscapePlusEnabled;
	mQualityReportingEnabled = other.mQualityReportingEnabled;
	mPublishEnabled = other.mPublishEnabled;
	mPushNotificationAllowed = other.mPushNotificationAllowed;
	mRemotePushNotificationAllowed = other.mRemotePushNotificationAllowed;
	mUseInternationalPrefixForCallsAndChats = other.mUseInternationalPrefixForCallsAndChats;

	mUserData = other.mUserData;

	mInternationalPrefix = other.mInternationalPrefix;
	mProxy = other.mProxy;
	mRealm = other.mRealm;
	mQualityReportingCollector = other.mQualityReportingCollector;
	mContactParameters = other.mContactParameters;
	mContactUriParameters = other.mContactUriParameters;
	mRefKey = other.mRefKey;
	mDependsOn = other.mDependsOn;
	mIdKey = other.mIdKey;
	mConferenceFactoryUri = other.mConferenceFactoryUri;
	mFileTransferServer = other.mFileTransferServer;
	mIdentity = other.mIdentity;

	mRoutes = bctbx_list_copy_with_data(other.mRoutes, (bctbx_list_copy_func)linphone_address_clone);
	mRoutesString = bctbx_list_copy_with_data(other.mRoutesString, (bctbx_list_copy_func)bctbx_strdup);

	mPrivacy = other.mPrivacy;

	mIdentityAddress = other.mIdentityAddress ? linphone_address_clone(other.mIdentityAddress) : nullptr;
	mProxyAddress = other.mProxyAddress ? linphone_address_clone(other.mProxyAddress) : nullptr;

	mAvpfMode = other.mAvpfMode;

	setNatPolicy(other.mNatPolicy);
	setPushNotificationConfig(other.mPushNotificationConfig);
}

AccountParams::~AccountParams () {
	if (mIdentityAddress) linphone_address_unref(mIdentityAddress);
	if (mProxyAddress) linphone_address_unref(mProxyAddress);
	if (mRoutes) bctbx_list_free_with_data(mRoutes, (bctbx_list_free_func)linphone_address_unref);
	if (mRoutesString)  bctbx_list_free_with_data(mRoutesString, (bctbx_list_free_func)bctbx_free);
	if (mNatPolicy) linphone_nat_policy_unref(mNatPolicy);
	if (mPushNotificationConfig) linphone_push_notification_config_unref(mPushNotificationConfig);
}

AccountParams* AccountParams::clone () const {
	return new AccountParams(*this);
}

// -----------------------------------------------------------------------------

void AccountParams::setExpires (int expires) {
	if (expires < 0) expires = 600;
	mExpires = expires;
}

void AccountParams::setQualityReportingInterval (int qualityReportingInterval) {
	mQualityReportingInterval = qualityReportingInterval;
}

void AccountParams::setPublishExpires (int publishExpires) {
	mPublishExpires = publishExpires;
}

void AccountParams::setAvpfRrInterval (uint8_t avpfRrInterval) {
	if (avpfRrInterval > 5) avpfRrInterval = 5;
	mAvpfRrInterval = avpfRrInterval;
}

void AccountParams::setRegisterEnabled (bool enable) {
	mRegisterEnabled = enable;
}

void AccountParams::setDialEscapePlusEnabled (bool enable) {
	mDialEscapePlusEnabled = enable;
}

void AccountParams::setQualityReportingEnabled (bool enable) {
	mQualityReportingEnabled = enable;
}

void AccountParams::setPublishEnabled (bool enable) {
	mPublishEnabled = enable;
}

void AccountParams::setOutboundProxyEnabled (bool enable) {
	// If enable we remove all routes to only have the server address as route
	// If disable since we should only have the server address route, we can remove the list
	if (mRoutes) {
		bctbx_list_free_with_data(mRoutes, (bctbx_list_free_func)linphone_address_unref);
		mRoutes = nullptr;
	}
	if (mRoutesString) {
		bctbx_list_free_with_data(mRoutesString, (bctbx_list_free_func)bctbx_free);
		mRoutesString = nullptr;
	}

	// Use only the proxy as route
	if (enable) {
		mRoutes = bctbx_list_append(mRoutes, linphone_address_clone(mProxyAddress));
		mRoutesString = bctbx_list_append(mRoutesString, bctbx_strdup(mProxy.c_str()));
	}
}

void AccountParams::setPushNotificationAllowed (bool allow) {
	mPushNotificationAllowed = allow;
}

void AccountParams::setRemotePushNotificationAllowed (bool allow) {
	mRemotePushNotificationAllowed = allow;
}

void AccountParams::setUseInternationalPrefixForCallsAndChats (bool enable) {
	mUseInternationalPrefixForCallsAndChats = enable;
}

void AccountParams::setUserData (void *userData) {
	mUserData = userData;
}

void AccountParams::setInternationalPrefix (const std::string &internationalPrefix) {
	mInternationalPrefix = internationalPrefix;
}

void AccountParams::setProxy (const std::string &proxy) {
	mProxy = proxy;
}

void AccountParams::setRealm (const std::string &realm) {
	mRealm = realm;
}

void AccountParams::setQualityReportingCollector (const std::string &qualityReportingCollector) {
	if (!qualityReportingCollector.empty()) {
		LinphoneAddress *addr = linphone_address_new(qualityReportingCollector.c_str());

		if (!addr) {
			lError() << "Invalid SIP collector URI: " << qualityReportingCollector << ". Quality reporting will be DISABLED.";
		} else {
			mQualityReportingCollector = qualityReportingCollector;
		}

		if (addr) {
			linphone_address_unref(addr);
		}
	}
}

void AccountParams::setContactParameters (const std::string &contactParameters) {
	mContactParameters = contactParameters;
}

void AccountParams::setContactUriParameters (const std::string &contactUriParameters) {
	mContactUriParameters = contactUriParameters;
}

void AccountParams::setRefKey (const std::string &refKey) {
	mRefKey = refKey;
}

void AccountParams::setDependsOn (const std::string &dependsOn) {
	mDependsOn = dependsOn;
}

void AccountParams::setIdKey (const std::string &idKey) {
	mIdKey = idKey;
}

void AccountParams::setConferenceFactoryUri (const std::string &conferenceFactoryUri) {
	if (!conferenceFactoryUri.empty()) {
		mConferenceFactoryUri = conferenceFactoryUri;
	}
}

void AccountParams::setFileTranferServer (const std::string &fileTransferServer) {
	mFileTransferServer = fileTransferServer;
}

LinphoneStatus AccountParams::setRoutes (const bctbx_list_t *routes) {
	if (mRoutes != nullptr) {
		bctbx_list_free_with_data(mRoutes, (bctbx_list_free_func)linphone_address_unref);
		mRoutes = nullptr;
	}
	if (mRoutesString != nullptr) {
		bctbx_list_free_with_data(mRoutesString, (bctbx_list_free_func)bctbx_free);
		mRoutesString = nullptr;
	}

	bctbx_list_t *iterator = (bctbx_list_t *) routes;
	while (iterator != nullptr) {
		LinphoneAddress *routeAddress = (LinphoneAddress *)bctbx_list_get_data(iterator);
		if (routeAddress != nullptr) {
			mRoutes = bctbx_list_append(mRoutes, linphone_address_clone(routeAddress));
			mRoutesString = bctbx_list_append(mRoutesString, linphone_address_as_string_uri_only(routeAddress));
		}
		iterator = bctbx_list_next(iterator);
	}

	return 0;
}

LinphoneStatus AccountParams::setRoutesFromStringList (const bctbx_list_t *routes) {
	if (mRoutes != nullptr) {
		bctbx_list_free_with_data(mRoutes, (bctbx_list_free_func)linphone_address_unref);
		mRoutes = nullptr;
	}
	if (mRoutesString != nullptr) {
		bctbx_list_free_with_data(mRoutesString, (bctbx_list_free_func)bctbx_free);
		mRoutesString = nullptr;
	}

	bctbx_list_t *iterator = (bctbx_list_t *) routes;
	while (iterator != nullptr) {
		char *route = (char *)bctbx_list_get_data(iterator);
		if (route != NULL && route[0] != '\0') {
			string tmp;
			/*try to prepend 'sip:' */
			if (strstr(route, "sip:") == nullptr && strstr(route, "sips:") == nullptr) {
				tmp.append("sip:");
			}
			tmp.append(route);

			SalAddress *addr = sal_address_new(tmp.c_str());
			if (addr != NULL) {
				sal_address_unref(addr);
				mRoutes = bctbx_list_append(mRoutes, linphone_address_new(tmp.c_str()));
				mRoutesString = bctbx_list_append(mRoutesString, bctbx_strdup(tmp.c_str()));
			} else {
				return -1;
			}
		}
		iterator = bctbx_list_next(iterator);
	}

	return 0;
}

void AccountParams::setPrivacy (LinphonePrivacyMask privacy) {
	mPrivacy = privacy;
}

LinphoneStatus AccountParams::setIdentityAddress (const LinphoneAddress* identityAddress) {
	if (!identityAddress || linphone_address_get_username(identityAddress) == nullptr) {
		char* as_string = identityAddress ? linphone_address_as_string(identityAddress) : ms_strdup("NULL");
		lWarning() << "Invalid sip identity: " << as_string;
		ms_free(as_string);
		return -1;
	}

	if (mIdentityAddress != nullptr) {
		linphone_address_unref(mIdentityAddress);
	}
	mIdentityAddress = linphone_address_clone(identityAddress);

	char *tmpIdentity = linphone_address_as_string(mIdentityAddress);
	mIdentity = tmpIdentity;
	bctbx_free(tmpIdentity);

	return 0;
}

void AccountParams::setAvpfMode (LinphoneAVPFMode avpfMode) {
	mAvpfMode = avpfMode;
}

void AccountParams::setNatPolicy (LinphoneNatPolicy *natPolicy) {
	if (natPolicy != nullptr) {
		linphone_nat_policy_ref(natPolicy); /* Prevent object destruction if the same policy is used */
	}
	if (mNatPolicy != nullptr) linphone_nat_policy_unref(mNatPolicy);
	mNatPolicy = natPolicy;
}

void AccountParams::setPushNotificationConfig (LinphonePushNotificationConfig *pushNotificationConfig) {
	if (mPushNotificationConfig != nullptr) linphone_push_notification_config_unref(mPushNotificationConfig);

	mPushNotificationConfig = linphone_push_notification_config_clone(pushNotificationConfig);
}

// -----------------------------------------------------------------------------

int AccountParams::getExpires () const {
	return mExpires;
}

int AccountParams::getQualityReportingInterval () const {
	return mQualityReportingInterval;
}

int AccountParams::getPublishExpires () const {
	/*default value is same as register*/
	return mPublishExpires < 0 ? mExpires : mPublishExpires;
}

uint8_t AccountParams::getAvpfRrInterval () const {
	return mAvpfRrInterval;
}

bool AccountParams::getRegisterEnabled () const {
	return mRegisterEnabled;
}

bool AccountParams::getDialEscapePlusEnabled () const {
	return mDialEscapePlusEnabled;
}

bool AccountParams::getQualityReportingEnabled () const {
	return mQualityReportingEnabled;
}

bool AccountParams::getPublishEnabled () const {
	return mPublishEnabled;
}

bool AccountParams::getOutboundProxyEnabled () const {
	LinphoneAddress *address = mRoutes != nullptr ? (LinphoneAddress *)bctbx_list_get_data(mRoutes) : nullptr;
	return address != nullptr && mProxyAddress != nullptr && linphone_address_weak_equal(mProxyAddress, address);
}

bool AccountParams::getPushNotificationAllowed () const {
	return mPushNotificationAllowed;
}

bool AccountParams::getRemotePushNotificationAllowed () const {
	return mRemotePushNotificationAllowed;
}

bool AccountParams::getUseInternationalPrefixForCallsAndChats () const {
	return mUseInternationalPrefixForCallsAndChats;
}

bool AccountParams::isPushNotificationAvailable () const {
	string prid = L_C_TO_STRING(linphone_push_notification_config_get_prid(mPushNotificationConfig));
	string param = L_C_TO_STRING(linphone_push_notification_config_get_param(mPushNotificationConfig));
	string basicToken = L_C_TO_STRING(linphone_push_notification_config_get_voip_token(mPushNotificationConfig));
	string remoteToken = L_C_TO_STRING(linphone_push_notification_config_get_remote_token(mPushNotificationConfig));
	string bundle = L_C_TO_STRING(linphone_push_notification_config_get_bundle_identifier(mPushNotificationConfig));
	// Accounts can support multiple types of push. Push notification is ready when all supported push's tokens to set

	bool paramAvailable = !param.empty() || !bundle.empty();
	bool pridAvailable = !prid.empty() || !((mPushNotificationAllowed && basicToken.empty()) || (mRemotePushNotificationAllowed && remoteToken.empty()));
	return paramAvailable && pridAvailable;

}

void* AccountParams::getUserData () const {
	return mUserData;
}

const std::string& AccountParams::getInternationalPrefix () const {
	return mInternationalPrefix;
}

const char* AccountParams::getDomain () const {
	return mIdentityAddress ? linphone_address_get_domain(mIdentityAddress) : nullptr;
}

const std::string& AccountParams::getProxy () const {
	return mProxy;
}

const std::string& AccountParams::getRealm () const {
	return mRealm;
}

const std::string& AccountParams::getQualityReportingCollector () const {
	return mQualityReportingCollector;
}

const std::string& AccountParams::getContactParameters () const {
	return mContactParameters;
}

const std::string& AccountParams::getContactUriParameters () const {
	return mContactUriParameters;
}

const std::string& AccountParams::getRefKey () const {
	return mRefKey;
}

const std::string& AccountParams::getDependsOn () const {
	return mDependsOn;
}

const std::string& AccountParams::getIdKey () const {
	return mIdKey;
}

const std::string& AccountParams::getConferenceFactoryUri () const {
	return mConferenceFactoryUri;
}

const std::string& AccountParams::getFileTransferServer () const {
	return mFileTransferServer;
}

const std::string& AccountParams::getIdentity () const {
	return mIdentity;
}

const bctbx_list_t* AccountParams::getRoutes () const {
	return mRoutes;
}

const bctbx_list_t* AccountParams::getRoutesString () const {
	return mRoutesString;
}

LinphonePrivacyMask AccountParams::getPrivacy () const {
	return mPrivacy;
}

LinphoneAddress* AccountParams::getIdentityAddress () const {
	return mIdentityAddress;
}

LinphoneAVPFMode AccountParams::getAvpfMode () const {
	return mAvpfMode;
}

LinphoneNatPolicy* AccountParams::getNatPolicy () const {
	return mNatPolicy;
}

LinphonePushNotificationConfig *AccountParams::getPushNotificationConfig () const {
	return mPushNotificationConfig;
}

// -----------------------------------------------------------------------------

LinphoneStatus AccountParams::setServerAddress (const LinphoneAddress *serverAddr) {
	bool outboundProxyEnabled = getOutboundProxyEnabled();

	if (mProxyAddress) linphone_address_unref(mProxyAddress);
	mProxyAddress = linphone_address_clone(serverAddr);

	char *tmpProxy = linphone_address_as_string(serverAddr);
	mProxy = tmpProxy;
	bctbx_free(tmpProxy);

	if (outboundProxyEnabled) {
		// Setting this to true will do the job of setting the routes
		setOutboundProxyEnabled(true);
	}

	return 0;
}

const LinphoneAddress *AccountParams::getServerAddress () const {
	return mProxyAddress;
}

LinphoneStatus AccountParams::setServerAddressAsString (const std::string &serverAddr) {
	LinphoneAddress *addr = nullptr;

	if (!serverAddr.empty()) {
		if (serverAddr.rfind("sip:") == string::npos && serverAddr.rfind("sips:") == string::npos) {
			string modified("");
			modified.append("sip:").append(serverAddr);
			addr = linphone_address_new(modified.c_str());
		}

		if (addr == nullptr) addr = linphone_address_new(serverAddr.c_str());
		if (addr) {
			bool outboundProxyEnabled = getOutboundProxyEnabled();

			if (mProxyAddress) linphone_address_unref(mProxyAddress);
			mProxyAddress = linphone_address_clone(addr);

			char *tmpProxy = linphone_address_as_string(addr);
			mProxy = tmpProxy;
			bctbx_free(tmpProxy);

			if (outboundProxyEnabled) {
				// Setting this to true will do the job of setting the routes
				setOutboundProxyEnabled(true);
			}

			linphone_address_unref(addr);
		} else {
			lWarning() << "Could not parse " << serverAddr;
			return -1;
		}
	}

	return 0;
}

const std::string& AccountParams::getServerAddressAsString () const {
	return mProxy;
}

void AccountParams::setTransport (LinphoneTransportType transport) {
	linphone_address_set_transport(mProxyAddress, transport);

	char *tmpProxy = linphone_address_as_string(mProxyAddress);
	mProxy = tmpProxy;
	bctbx_free(tmpProxy);

	if (getOutboundProxyEnabled()) {
		setOutboundProxyEnabled(true);
	}
}

LinphoneTransportType AccountParams::getTransport () const {
	return linphone_address_get_transport(mProxyAddress);
}

void AccountParams::writeToConfigFile (LinphoneConfig *config, int index) {
	char key[50];

	sprintf(key, "proxy_%i", index);
	linphone_config_clean_section(config, key);

	if (!mProxy.empty()){
		linphone_config_set_string(config, key, "reg_proxy", mProxy.c_str());
	}
	if (mRoutesString != NULL) {
		linphone_config_set_string_list(config, key, "reg_route", mRoutesString);
	}
	if (!mIdentity.empty()){
		linphone_config_set_string(config, key, "reg_identity", mIdentity.c_str());
	}
	if (!mRealm.empty()){
		linphone_config_set_string(config, key, "realm", mRealm.c_str());
	}
	if (!mContactParameters.empty()){
		linphone_config_set_string(config, key, "contact_parameters", mContactParameters.c_str());
	}
	if (!mContactUriParameters.empty()){
		linphone_config_set_string(config, key, "contact_uri_parameters", mContactUriParameters.c_str());
	}
	if (!mQualityReportingCollector.empty()){
		linphone_config_set_string(config, key, "quality_reporting_collector", mQualityReportingCollector.c_str());
	}
	linphone_config_set_int(config, key, "quality_reporting_enabled", mQualityReportingEnabled);
	linphone_config_set_int(config, key, "quality_reporting_interval", mQualityReportingInterval);
	linphone_config_set_int(config, key, "reg_expires", mExpires);
	linphone_config_set_int(config, key, "reg_sendregister", mRegisterEnabled);
	linphone_config_set_int(config, key, "publish", mPublishEnabled);
	linphone_config_set_int(config, key, "avpf", mAvpfMode);
	linphone_config_set_int(config, key, "avpf_rr_interval", mAvpfRrInterval);
	linphone_config_set_int(config, key, "dial_escape_plus", mDialEscapePlusEnabled);
	linphone_config_set_string(config,key,"dial_prefix", mInternationalPrefix.c_str());
	linphone_config_set_int(config, key, "use_dial_prefix_for_calls_and_chats", mUseInternationalPrefixForCallsAndChats);
	linphone_config_set_int(config, key, "privacy", (int)mPrivacy);
	linphone_config_set_int(config, key, "push_notification_allowed", (int)mPushNotificationAllowed);
	linphone_config_set_int(config, key, "remote_push_notification_allowed", (int)mRemotePushNotificationAllowed);
	if (!mRefKey.empty()) linphone_config_set_string(config, key, "refkey", mRefKey.c_str());
	if (!mDependsOn.empty()) linphone_config_set_string(config, key, "depends_on", mDependsOn.c_str());
	if (!mIdKey.empty()) linphone_config_set_string(config, key, "idkey", mIdKey.c_str());
	linphone_config_set_int(config, key, "publish_expires", mPublishExpires);

	if (mNatPolicy != NULL) {
		linphone_config_set_string(config, key, "nat_policy_ref", mNatPolicy->ref);
		linphone_nat_policy_save_to_config(mNatPolicy);
	}

	linphone_config_set_string(config, key, "conference_factory_uri", mConferenceFactoryUri.c_str());
}

LINPHONE_END_NAMESPACE
