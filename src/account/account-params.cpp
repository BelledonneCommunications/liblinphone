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

#include "account-params.h"
#include "push-notification/push-notification-config.h"
#include "c-wrapper/internal/c-tools.h"
#include "linphone/api/c-address.h"
#include "nat/nat-policy.h"
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
	mAllowCpimMessagesInBasicChatRooms = lc ? !!linphone_config_get_default_int(lc->config, "proxy", "cpim_in_basic_chat_rooms_enabled", false) : false;
	
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

	/* CAUTION: the nat_policy_ref meaning in default values is different than in usual [nat_policy_%i] section.
	 * This is not consistent and error-prone.
	 * Normally, the nat_policy_ref refers to a "ref" entry within a [nat_policy_%i] section.
	 */

	string natPolicyRef = lc ? linphone_config_get_default_string(lc->config, "proxy", "nat_policy_ref", "") : "";
	if (!natPolicyRef.empty()) {
		NatPolicy * policy = nullptr;
		if (linphone_config_has_section(lc->config, natPolicyRef.c_str())){
			/* Odd method - to be deprecated, inconsistent */
			policy = new NatPolicy(L_GET_CPP_PTR_FROM_C_OBJECT(lc), NatPolicy::ConstructionMethod::FromSectionName, natPolicyRef);
		}else{
			/* Usual method */
			policy = new NatPolicy(L_GET_CPP_PTR_FROM_C_OBJECT(lc), NatPolicy::ConstructionMethod::FromRefName, natPolicyRef);
		}
		if (policy){
			setNatPolicy(policy->toSharedPtr());
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

	string audioVideoConferenceFactoryUri = lc ? linphone_config_get_default_string(lc->config, "proxy", "audio_video_conference_factory_uri", "") : "";
	mAudioVideoConferenceFactoryAddress = nullptr;
	if (!audioVideoConferenceFactoryUri.empty()) {
		mAudioVideoConferenceFactoryAddress = linphone_address_new(audioVideoConferenceFactoryUri.c_str());
	}

	if (lc && lc->push_config) {
		mPushNotificationConfig = PushNotificationConfig::toCpp(lc->push_config)->clone();
	} else {
		mPushNotificationConfig = new PushNotificationConfig();
		mPushNotificationConfig->readPushParamsFromString(string(lc ? linphone_config_get_default_string(lc->config, "proxy", "push_parameters", "") : ""));
	}
	mRtpBundleEnabled = lc ? !!linphone_config_get_default_int(lc->config, "proxy", "rtp_bundle", linphone_core_rtp_bundle_enabled(lc)) : false;
	mRtpBundleAssumption = lc ? !!linphone_config_get_default_int(lc->config, "proxy", "rtp_bundle_assumption", false) : false;
	
	string customContact = lc ? linphone_config_get_default_string(lc->config, "proxy", "custom_contact", "") : "";
	setCustomContact(customContact);

	string limeServerUrl = lc ? linphone_config_get_default_string(lc->config, "proxy", "lime_server_url", "") : "";
	setLimeServerUrl(limeServerUrl);
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
	} else {
		// Remove default routes if the existing account doesn't have any
		if (mRoutes) {
			bctbx_list_free_with_data(mRoutes, (bctbx_list_free_func)linphone_address_unref);
			mRoutes = nullptr;
		}
		if (mRoutesString) {
			bctbx_list_free_with_data(mRoutesString, (bctbx_list_free_func)bctbx_free);
			mRoutesString = nullptr;
		}
	}

	mRealm = linphone_config_get_string(config, key, "realm", mRealm.c_str());

	mQualityReportingEnabled = !!linphone_config_get_int(config, key, "quality_reporting_enabled", mQualityReportingEnabled);
	mQualityReportingCollector = linphone_config_get_string(config, key, "quality_reporting_collector", mQualityReportingCollector.c_str());
	mQualityReportingInterval = linphone_config_get_int(config, key, "quality_reporting_interval", mQualityReportingInterval);
	mContactParameters = linphone_config_get_string(config, key, "contact_parameters", mContactParameters.c_str());
	mContactUriParameters = linphone_config_get_string(config, key, "contact_uri_parameters", mContactUriParameters.c_str());
	string pushParameters = linphone_config_get_string(config, key, "push_parameters", "");
	
	//mPushNotificationConfig can't be null because it is always created in AccountParams(lc) called previously
	if (linphone_core_is_push_notification_enabled(lc) && !pushParameters.empty()) {
		mPushNotificationConfig->readPushParamsFromString(pushParameters);
	} else if (!mContactUriParameters.empty()){
		mPushNotificationConfig->readPushParamsFromString(mContactUriParameters);
	}
	
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
	mAllowCpimMessagesInBasicChatRooms = !!linphone_config_get_int(config, key, "cpim_in_basic_chat_rooms_enabled", mAllowCpimMessagesInBasicChatRooms);

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
		/* CAUTION: the nat_policy_ref meaning in default values is different than in usual [nat_policy_%i] section.
		 * This is not consistent and error-prone.
		 * Normally, the nat_policy_ref refers to a "ref" entry within a [nat_policy_%i] section.
		 */
		LinphoneNatPolicy *natPolicy;
		if (linphone_config_has_section(config, nat_policy_ref)){
			/* Odd method - to be deprecated, inconsistent */
			natPolicy = linphone_core_create_nat_policy_from_config(lc, nat_policy_ref);
		} else {
			/* Usual method */
			natPolicy = linphone_core_create_nat_policy_from_ref(lc, nat_policy_ref);
		}
		mNatPolicy = NatPolicy::toCpp(natPolicy)->toSharedPtr();
	}

	mConferenceFactoryUri = linphone_config_get_string(config, key, "conference_factory_uri", mConferenceFactoryUri.c_str());
	string audioVideoConferenceFactoryUri = linphone_config_get_string(config, key, "audio_video_conference_factory_uri", "");
	mAudioVideoConferenceFactoryAddress = nullptr;
	if (!audioVideoConferenceFactoryUri.empty()) {
		mAudioVideoConferenceFactoryAddress = linphone_address_new(audioVideoConferenceFactoryUri.c_str());
	}
	mRtpBundleEnabled = !!linphone_config_get_bool(config, key, "rtp_bundle", linphone_core_rtp_bundle_enabled(lc));
	mRtpBundleAssumption = !!linphone_config_get_bool(config, key, "rtp_bundle_assumption", FALSE);

	setCustomContact(linphone_config_get_string(config, key, "custom_contact", ""));

	mLimeServerUrl = linphone_config_get_string(config, key, "lime_server_url", mLimeServerUrl.c_str());

	readCustomParamsFromConfigFile (config, key);
}

AccountParams::AccountParams (const AccountParams &other) : HybridObject(other), CustomParams(other) {
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
	mAllowCpimMessagesInBasicChatRooms = other.mAllowCpimMessagesInBasicChatRooms;

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
	mAudioVideoConferenceFactoryAddress = other.mAudioVideoConferenceFactoryAddress ? linphone_address_clone(other.mAudioVideoConferenceFactoryAddress) : nullptr;
	mFileTransferServer = other.mFileTransferServer;
	mIdentity = other.mIdentity;

	mRoutes = bctbx_list_copy_with_data(other.mRoutes, (bctbx_list_copy_func)linphone_address_clone);
	mRoutesString = bctbx_list_copy_with_data(other.mRoutesString, (bctbx_list_copy_func)bctbx_strdup);

	mPrivacy = other.mPrivacy;

	mIdentityAddress = other.mIdentityAddress ? linphone_address_clone(other.mIdentityAddress) : nullptr;
	mProxyAddress = other.mProxyAddress ? linphone_address_clone(other.mProxyAddress) : nullptr;

	mAvpfMode = other.mAvpfMode;

	setNatPolicy(other.mNatPolicy);
	
	mPushNotificationConfig = other.mPushNotificationConfig->clone();
	mRtpBundleEnabled = other.mRtpBundleEnabled;
	mRtpBundleAssumption = other.mRtpBundleAssumption;
	mCustomContact = other.mCustomContact ? linphone_address_clone(other.mCustomContact) : nullptr;
	mLimeServerUrl = other.mLimeServerUrl;
}

AccountParams::~AccountParams () {
	if (mIdentityAddress) linphone_address_unref(mIdentityAddress);
	if (mProxyAddress) linphone_address_unref(mProxyAddress);
	if (mRoutes) bctbx_list_free_with_data(mRoutes, (bctbx_list_free_func)linphone_address_unref);
	if (mRoutesString)  bctbx_list_free_with_data(mRoutesString, (bctbx_list_free_func)bctbx_free);
	if (mPushNotificationConfig) mPushNotificationConfig->unref();
	if (mAudioVideoConferenceFactoryAddress) linphone_address_unref(mAudioVideoConferenceFactoryAddress);
	if (mCustomContact) linphone_address_unref(mCustomContact);
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
		if (!mProxyAddress) {
			lError() << "Can't enable outbound proxy without having set the proxy address first!";
			return;
		}
		
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

void AccountParams::setCpimMessagesAllowedInBasicChatRooms (bool allow) {
	mAllowCpimMessagesInBasicChatRooms = allow;
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
	mConferenceFactoryUri = conferenceFactoryUri;
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

void AccountParams::setNatPolicy (const shared_ptr<NatPolicy> &natPolicy) {
	mNatPolicy = natPolicy;
}

void AccountParams::setPushNotificationConfig (PushNotificationConfig *pushNotificationConfig) {
	if (mPushNotificationConfig) mPushNotificationConfig->unref();
	
	mPushNotificationConfig = pushNotificationConfig;
	mPushNotificationConfig->ref();
}

void AccountParams::setAudioVideoConferenceFactoryAddress (const LinphoneAddress *audioVideoConferenceFactoryAddress) {
	if (mAudioVideoConferenceFactoryAddress != nullptr) {
		linphone_address_unref(mAudioVideoConferenceFactoryAddress);
		mAudioVideoConferenceFactoryAddress = nullptr;
	}
	if (audioVideoConferenceFactoryAddress != nullptr) {
		mAudioVideoConferenceFactoryAddress = linphone_address_clone(audioVideoConferenceFactoryAddress);
	}
}

void AccountParams::enableRtpBundle(bool value){
	mRtpBundleEnabled = value;
}

void AccountParams::enableRtpBundleAssumption(bool value){
	mRtpBundleAssumption = value;
}

void AccountParams::setCustomContact(const LinphoneAddress *contact){
	if (mCustomContact) linphone_address_unref(mCustomContact);
	mCustomContact = contact ? linphone_address_clone(contact) : nullptr;
}

void AccountParams::setCustomContact(const string &contact){
	LinphoneAddress *address = !contact.empty() ? linphone_address_new(contact.c_str()) : nullptr;
	if (address == nullptr && !contact.empty()){
		lError() << "AccountParams: invalid custom contact '" << contact << "'";
	}
	if (mCustomContact) linphone_address_unref(mCustomContact);
	mCustomContact = address;
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
	string prid = mPushNotificationConfig->getPrid();
	string param = mPushNotificationConfig->getParam();
	string basicToken = mPushNotificationConfig->getVoipToken();
	string remoteToken = mPushNotificationConfig->getRemoteToken();
	string bundle = mPushNotificationConfig->getBundleIdentifer();
	// Accounts can support multiple types of push. Push notification is ready when all supported push's tokens to set

	bool paramAvailable = !param.empty() || !bundle.empty();
	bool pridAvailable = !prid.empty() || !((mPushNotificationAllowed && basicToken.empty()) || (mRemotePushNotificationAllowed && remoteToken.empty()));
	return paramAvailable && pridAvailable;
}

bool AccountParams::isCpimMessagesAllowedInBasicChatRooms () const {
	return mAllowCpimMessagesInBasicChatRooms;
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

shared_ptr<NatPolicy> AccountParams::getNatPolicy () const {
	return mNatPolicy;
}

PushNotificationConfig* AccountParams::getPushNotificationConfig () const {
	return mPushNotificationConfig;
}

const LinphoneAddress* AccountParams::getAudioVideoConferenceFactoryAddress () const {
	return mAudioVideoConferenceFactoryAddress;
}

bool AccountParams::rtpBundleEnabled() const{
	return mRtpBundleEnabled;
}

bool AccountParams::rtpBundleAssumptionEnabled()const{
	return mRtpBundleAssumption;
}

const LinphoneAddress *AccountParams::getCustomContact()const{
	return mCustomContact;
}

void AccountParams::setLimeServerUrl(const std::string &url) {
	mLimeServerUrl = url;
}

const std::string& AccountParams::getLimeServerUrl() const {
	return mLimeServerUrl;
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
	
	string pushParams;
	if (mPushNotificationAllowed || mRemotePushNotificationAllowed) {
		pushParams = mPushNotificationConfig->asString(mRemotePushNotificationAllowed);
	}
	linphone_config_set_string(config, key, "push_parameters", pushParams.c_str());
	linphone_config_set_int(config, key, "quality_reporting_enabled", (int)mQualityReportingEnabled);
	linphone_config_set_int(config, key, "quality_reporting_interval", mQualityReportingInterval);
	linphone_config_set_int(config, key, "reg_expires", mExpires);
	linphone_config_set_int(config, key, "reg_sendregister", (int)mRegisterEnabled);
	linphone_config_set_int(config, key, "publish", (int)mPublishEnabled);
	linphone_config_set_int(config, key, "avpf", mAvpfMode);
	linphone_config_set_int(config, key, "avpf_rr_interval", mAvpfRrInterval);
	linphone_config_set_int(config, key, "dial_escape_plus", (int)mDialEscapePlusEnabled);
	linphone_config_set_string(config,key,"dial_prefix", mInternationalPrefix.c_str());
	linphone_config_set_int(config, key, "use_dial_prefix_for_calls_and_chats", mUseInternationalPrefixForCallsAndChats);
	linphone_config_set_int(config, key, "privacy", (int)mPrivacy);
	linphone_config_set_int(config, key, "push_notification_allowed", (int)mPushNotificationAllowed);
	linphone_config_set_int(config, key, "remote_push_notification_allowed", (int)mRemotePushNotificationAllowed);
	linphone_config_set_int(config, key, "cpim_in_basic_chat_rooms_enabled", (int)mAllowCpimMessagesInBasicChatRooms);
	if (!mRefKey.empty()) linphone_config_set_string(config, key, "refkey", mRefKey.c_str());
	if (!mDependsOn.empty()) linphone_config_set_string(config, key, "depends_on", mDependsOn.c_str());
	if (!mIdKey.empty()) linphone_config_set_string(config, key, "idkey", mIdKey.c_str());
	linphone_config_set_int(config, key, "publish_expires", mPublishExpires);

	if (mNatPolicy != NULL) {
		linphone_config_set_string(config, key, "nat_policy_ref", mNatPolicy->getRef().c_str());
	}

	linphone_config_set_string(config, key, "conference_factory_uri", mConferenceFactoryUri.c_str());

	if (mAudioVideoConferenceFactoryAddress != nullptr) {
		char * factory_address = linphone_address_as_string_uri_only(mAudioVideoConferenceFactoryAddress);
		linphone_config_set_string(config, key, "audio_video_conference_factory_uri", factory_address);
		ms_free(factory_address);
	}
	linphone_config_set_int(config, key, "rtp_bundle", mRtpBundleEnabled);
	linphone_config_set_int(config, key, "rtp_bundle_assumption", mRtpBundleAssumption);

	writeCustomParamsToConfigFile (config, key);

	linphone_config_set_string(config, key, "lime_server_url", mLimeServerUrl.c_str());
}

LINPHONE_END_NAMESPACE
