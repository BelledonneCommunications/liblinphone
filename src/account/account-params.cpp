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

#include "account-params.h"

#include <set>

#include "c-wrapper/internal/c-tools.h"
#include "core/core.h"
#include "linphone/types.h"
#include "nat/nat-policy.h"
#include "private.h"
#include "push-notification/push-notification-config.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

static string generate_account_id() {
	char id[17] = {0};
	belle_sip_random_token(id, 16);
	return string("proxy_config_").append(id); // TODO: change to account
}

AccountParams::AccountParams(LinphoneCore *lc, bool useDefaultValues) {
	if (!lc && useDefaultValues) {
		lWarning() << "Unable to apply proxy default values: LinphoneCore is null.";
		useDefaultValues = false;
	}

	mExpires = useDefaultValues ? linphone_config_get_default_int(lc->config, "proxy", "reg_expires", 3600) : 3600;
	mRegisterEnabled =
	    useDefaultValues ? !!linphone_config_get_default_int(lc->config, "proxy", "reg_sendregister", 1) : 1;
	mInternationalPrefix =
	    useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "dial_prefix", "") : "";
	mInternationalPrefixIsoCountryCode =
	    useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "dial_prefix_iso_country_code", "")
	                     : "";
	mUseInternationalPrefixForCallsAndChats =
	    useDefaultValues
	        ? !!linphone_config_get_default_int(lc->config, "proxy", "use_dial_prefix_for_calls_and_chats", true)
	        : true;
	mDialEscapePlusEnabled =
	    useDefaultValues ? !!linphone_config_get_default_int(lc->config, "proxy", "dial_escape_plus", false) : false;
	mPrivacy = useDefaultValues ? (LinphonePrivacyMask)linphone_config_get_default_int(lc->config, "proxy", "privacy",
	                                                                                   LinphonePrivacyDefault)
	                            : (LinphonePrivacyMask)LinphonePrivacyDefault;
	mIdentity = useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "reg_identity", "") : "";
	mIdentityAddress = Address::create(mIdentity);
	mProxy = useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "reg_proxy", "") : "";
	mProxyAddress = Address::create(mProxy);
	string route = useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "reg_route", "") : "";
	if (!route.empty()) {
		const std::list<std::shared_ptr<Address>> routes{Address::create(route)};
		setRoutes(routes);
	}
	mRealm = useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "realm", "") : "";
	mQualityReportingEnabled =
	    useDefaultValues ? !!linphone_config_get_default_int(lc->config, "proxy", "quality_reporting_enabled", false)
	                     : false;
	mQualityReportingCollector =
	    useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "quality_reporting_collector", "")
	                     : "";
	mQualityReportingInterval =
	    useDefaultValues ? linphone_config_get_default_int(lc->config, "proxy", "quality_reporting_interval", 0) : 0;
	mContactParameters =
	    useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "contact_parameters", "") : "";
	mContactUriParameters =
	    useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "contact_uri_parameters", "") : "";
	mAllowCpimMessagesInBasicChatRooms =
	    useDefaultValues
	        ? !!linphone_config_get_default_int(lc->config, "proxy", "cpim_in_basic_chat_rooms_enabled", false)
	        : false;

	mAvpfMode = useDefaultValues ? static_cast<LinphoneAVPFMode>(linphone_config_get_default_int(
	                                   lc->config, "proxy", "avpf", LinphoneAVPFDefault))
	                             : LinphoneAVPFDefault;
	mAvpfRrInterval =
	    useDefaultValues ? !!linphone_config_get_default_int(lc->config, "proxy", "avpf_rr_interval", 5) : 5;
	mPublishExpires =
	    useDefaultValues ? linphone_config_get_default_int(lc->config, "proxy", "publish_expires", 600) : 600;
	mPublishEnabled =
	    useDefaultValues ? !!linphone_config_get_default_int(lc->config, "proxy", "publish", false) : false;

	bool pushAllowedDefault = false;
	bool remotePushAllowedDefault = false;
#if defined(__ANDROID__) || TARGET_OS_IPHONE
	pushAllowedDefault = true;
#endif
	mPushNotificationAllowed =
	    useDefaultValues
	        ? !!linphone_config_get_default_int(lc->config, "proxy", "push_notification_allowed", pushAllowedDefault)
	        : pushAllowedDefault;
	mRemotePushNotificationAllowed =
	    useDefaultValues ? !!linphone_config_get_default_int(lc->config, "proxy", "remote_push_notification_allowed",
	                                                         remotePushAllowedDefault)
	                     : remotePushAllowedDefault;
	mForceRegisterOnPush = useDefaultValues
	                           ? !!linphone_config_get_default_int(lc->config, "proxy", "force_register_on_push", false)
	                           : false;
	mUnregisterAtStop =
	    useDefaultValues ? !!linphone_config_get_default_int(lc->config, "proxy", "unregister_at_stop", true) : true;
	mRefKey = useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "refkey", "") : "";

	/* CAUTION: the nat_policy_ref meaning in default values is different than in usual [nat_policy_%i] section.
	 * This is not consistent and error-prone.
	 * Normally, the nat_policy_ref refers to a "ref" entry within a [nat_policy_%i] section.
	 */

	string natPolicyRef =
	    useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "nat_policy_ref", "") : "";
	if (!natPolicyRef.empty()) {
		std::shared_ptr<NatPolicy> policy = nullptr;
		if (lc && linphone_config_has_section(lc->config, natPolicyRef.c_str())) {
			/* Odd method - to be deprecated, inconsistent */
			policy = NatPolicy::create(L_GET_CPP_PTR_FROM_C_OBJECT(lc), NatPolicy::ConstructionMethod::FromSectionName,
			                           natPolicyRef);
		} else {
			/* Usual method */
			policy = NatPolicy::create(L_GET_CPP_PTR_FROM_C_OBJECT(lc), NatPolicy::ConstructionMethod::FromRefName,
			                           natPolicyRef);
		}
		if (policy) {
			mNatPolicy = policy;
		} else {
			lError() << "Cannot create default nat policy with ref [" << natPolicyRef << "] for account [" << this
			         << "]";
		}
	}
	mDependsOn = useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "depends_on", "") : "";
	string idkey = useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "idkey", "") : "";
	if (!idkey.empty()) {
		mIdKey = idkey;
	} else {
		mIdKey = generate_account_id();
	}
	string conferenceFactoryUri =
	    useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "conference_factory_uri", "") : "";
	setConferenceFactoryUri(conferenceFactoryUri);

	string audioVideoConferenceFactoryUri =
	    useDefaultValues
	        ? linphone_config_get_default_string(lc->config, "proxy", "audio_video_conference_factory_uri", "")
	        : "";
	mAudioVideoConferenceFactoryAddress = nullptr;
	if (!audioVideoConferenceFactoryUri.empty()) {
		mAudioVideoConferenceFactoryAddress = Address::create(audioVideoConferenceFactoryUri);
	}

	mCcmpServerUrl =
	    useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "ccmp_server_url", "") : "";

	if (lc && lc->push_config) {
		mPushNotificationConfig = PushNotificationConfig::toCpp(lc->push_config)->clone();
	} else {
		mPushNotificationConfig = new PushNotificationConfig();
		mPushNotificationConfig->readPushParamsFromString(string(
		    useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "push_parameters", "") : ""));
	}
	mRtpBundleEnabled = useDefaultValues ? !!linphone_config_get_default_int(lc->config, "proxy", "rtp_bundle",
	                                                                         linphone_core_rtp_bundle_enabled(lc))
	                                     : false;
	mRtpBundleAssumption = useDefaultValues
	                           ? !!linphone_config_get_default_int(lc->config, "proxy", "rtp_bundle_assumption", false)
	                           : false;

	string customContact =
	    useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "custom_contact", "") : "";
	setCustomContact(customContact);

	string limeServerUrl =
	    useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "lime_server_url", "") : "";
	setLimeServerUrl(limeServerUrl);

	string limeAlgo = useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "lime_algo", "") : "";
	setLimeAlgo(limeAlgo);

	string pictureUri =
	    useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "picture_uri", "") : "";
	setPictureUri(pictureUri);

	string mwiServerUri =
	    useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "mwi_server_uri", "") : "";
	mMwiServerAddress = nullptr;
	if (!mwiServerUri.empty()) {
		setMwiServerAddress(Address::create(mwiServerUri));
	}

	string voicemailUri =
	    useDefaultValues ? linphone_config_get_default_string(lc->config, "proxy", "voicemail_uri", "") : "";
	mVoicemailAddress = nullptr;
	if (!voicemailUri.empty()) {
		setVoicemailAddress(Address::create(voicemailUri));
	}

	mInstantMessagingEncryptionMandatory =
	    useDefaultValues ? !!linphone_config_get_default_int(lc->config, "proxy", "im_encryption_mandatory", 0) : 0;

	string supportedTags = lc ? linphone_config_get_default_string(lc->config, "proxy", "supported", "empty") : "empty";
	if (useDefaultValues && supportedTags != "empty") {
		vector<string> splitTags = bctoolbox::Utils::split(supportedTags, ",");
		list<string> supportedTagsList;
		for (const auto &tag : splitTags)
			supportedTagsList.push_back(Utils::trim(tag));
		mSupportedTagsList.mList = supportedTagsList;
		mUseSupportedTags = true;
	} else if (lc) {
		vector<string> splitTags = bctoolbox::Utils::split(lc->sal->getSupportedTags(), ",");
		list<string> supportedTagsList;
		for (const auto &tag : splitTags)
			supportedTagsList.push_back(Utils::trim(tag));
		mSupportedTagsList.mList = supportedTagsList;
	}
}

AccountParams::AccountParams(LinphoneCore *lc, int index) : AccountParams(lc, false) {
	LpConfig *config = lc->config;

	char key[50];
	snprintf(key, sizeof(key), "proxy_%i", index); // TODO: change to account

	mIdentity = linphone_config_get_string(config, key, "reg_identity", mIdentity.c_str());
	std::shared_ptr<Address> identity_address = Address::create(mIdentity);
	setIdentityAddress(identity_address);

	setServerAddressAsString(linphone_config_get_string(config, key, "reg_proxy", getServerAddressAsString().c_str()));

	bctbx_list_t *routes = linphone_config_get_string_list(config, key, "reg_route", nullptr);
	if (routes != nullptr) {
		setRoutesFromStringList(routes);
		bctbx_list_free_with_data(routes, (bctbx_list_free_func)bctbx_free);
	}

	mRealm = linphone_config_get_string(config, key, "realm", mRealm.c_str());

	mQualityReportingEnabled =
	    !!linphone_config_get_int(config, key, "quality_reporting_enabled", mQualityReportingEnabled);
	mQualityReportingCollector =
	    linphone_config_get_string(config, key, "quality_reporting_collector", mQualityReportingCollector.c_str());
	mQualityReportingInterval =
	    linphone_config_get_int(config, key, "quality_reporting_interval", mQualityReportingInterval);
	mContactParameters = linphone_config_get_string(config, key, "contact_parameters", mContactParameters.c_str());
	mContactUriParameters =
	    linphone_config_get_string(config, key, "contact_uri_parameters", mContactUriParameters.c_str());
	string pushParameters = linphone_config_get_string(config, key, "push_parameters", "");

	// mPushNotificationConfig can't be null because it is always created in AccountParams(lc) called previously
	if (linphone_core_is_push_notification_enabled(lc) && !pushParameters.empty()) {
		mPushNotificationConfig->readPushParamsFromString(pushParameters);
	} else if (!mContactUriParameters.empty()) {
		mPushNotificationConfig->readPushParamsFromString(mContactUriParameters);
	} else if (lc && lc->push_config) {
		mPushNotificationConfig->unref(); // Coming from constructor
		mPushNotificationConfig = PushNotificationConfig::toCpp(lc->push_config)->clone();
	}

	mExpires = linphone_config_get_int(config, key, "reg_expires", mExpires);
	mRegisterEnabled = !!linphone_config_get_int(config, key, "reg_sendregister", mRegisterEnabled);
	mPublishEnabled = !!linphone_config_get_int(config, key, "publish", mPublishEnabled);
	setPushNotificationAllowed(
	    !!linphone_config_get_int(config, key, "push_notification_allowed", mPushNotificationAllowed));
	setRemotePushNotificationAllowed(
	    !!linphone_config_get_int(config, key, "remote_push_notification_allowed", mRemotePushNotificationAllowed));
	setForceRegisterOnPushNotification(
	    !!linphone_config_get_int(config, key, "force_register_on_push", mForceRegisterOnPush));
	setUnregisterAtStop(!!linphone_config_get_int(config, key, "unregister_at_stop", mUnregisterAtStop));
	mAvpfMode =
	    static_cast<LinphoneAVPFMode>(linphone_config_get_int(config, key, "avpf", static_cast<int>(mAvpfMode)));
	mAvpfRrInterval = (uint8_t)linphone_config_get_int(config, key, "avpf_rr_interval", (int)mAvpfRrInterval);
	mDialEscapePlusEnabled = !!linphone_config_get_int(config, key, "dial_escape_plus", mDialEscapePlusEnabled);
	mInternationalPrefix = linphone_config_get_string(config, key, "dial_prefix", mInternationalPrefix.c_str());
	mInternationalPrefixIsoCountryCode = linphone_config_get_string(config, key, "dial_prefix_iso_country_code",
	                                                                mInternationalPrefixIsoCountryCode.c_str());
	mUseInternationalPrefixForCallsAndChats = !!linphone_config_get_int(
	    config, key, "use_dial_prefix_for_calls_and_chats", mUseInternationalPrefixForCallsAndChats);
	mAllowCpimMessagesInBasicChatRooms =
	    !!linphone_config_get_int(config, key, "cpim_in_basic_chat_rooms_enabled", mAllowCpimMessagesInBasicChatRooms);

	mPrivacy =
	    static_cast<LinphonePrivacyMask>(linphone_config_get_int(config, key, "privacy", static_cast<int>(mPrivacy)));

	mRefKey = linphone_config_get_string(config, key, "refkey", mRefKey.c_str());
	mIdKey = linphone_config_get_string(config, key, "idkey", mRefKey.c_str());
	if (mIdKey.empty()) {
		mIdKey = generate_account_id();
		lWarning() << "generated proxyconfig idkey = [" << mIdKey << "]";
	}
	mDependsOn = linphone_config_get_string(config, key, "depends_on", mDependsOn.c_str());

	mPublishExpires = linphone_config_get_int(config, key, "publish_expires", mPublishExpires);

	const char *nat_policy_ref = linphone_config_get_string(config, key, "nat_policy_ref", nullptr);
	if (nat_policy_ref != nullptr) {
		/* CAUTION: the nat_policy_ref meaning in default values is different than in usual [nat_policy_%i] section.
		 * This is not consistent and error-prone.
		 * Normally, the nat_policy_ref refers to a "ref" entry within a [nat_policy_%i] section.
		 */
		LinphoneNatPolicy *natPolicy;
		if (linphone_config_has_section(config, nat_policy_ref)) {
			/* Odd method - to be deprecated, inconsistent */
			natPolicy = linphone_core_create_nat_policy_from_config(lc, nat_policy_ref);
		} else {
			/* Usual method */
			natPolicy = linphone_core_create_nat_policy_from_ref(lc, nat_policy_ref);
		}
		mNatPolicy = NatPolicy::toCpp(natPolicy)->toSharedPtr();
	}

	setConferenceFactoryUri(L_C_TO_STRING(linphone_config_get_string(config, key, "conference_factory_uri", "")));

	string audioVideoConferenceFactoryUri =
	    linphone_config_get_string(config, key, "audio_video_conference_factory_uri", "");
	mAudioVideoConferenceFactoryAddress = nullptr;
	if (!audioVideoConferenceFactoryUri.empty()) {
		mAudioVideoConferenceFactoryAddress = Address::create(audioVideoConferenceFactoryUri);
	}

	mCcmpServerUrl = linphone_config_get_string(config, key, "ccmp_server_url", "");

	mRtpBundleEnabled = !!linphone_config_get_bool(config, key, "rtp_bundle", linphone_core_rtp_bundle_enabled(lc));
	mRtpBundleAssumption = !!linphone_config_get_bool(config, key, "rtp_bundle_assumption", FALSE);

	setCustomContact(linphone_config_get_string(config, key, "custom_contact", ""));

	setLimeServerUrl(linphone_config_get_string(config, key, "lime_server_url", mLimeServerUrl.c_str()));
	setLimeAlgo(linphone_config_get_string(config, key, "lime_algo", mLimeAlgo.c_str()));

	setPictureUri(linphone_config_get_string(config, key, "picture_uri", mPictureUri.c_str()));

	string mwiServerUri = linphone_config_get_string(config, key, "mwi_server_uri", "");
	mMwiServerAddress = nullptr;
	if (!mwiServerUri.empty()) {
		setMwiServerAddress(Address::create(mwiServerUri));
	}
	string voicemailUri = linphone_config_get_string(config, key, "voicemail_uri", "");
	mVoicemailAddress = nullptr;
	if (!voicemailUri.empty()) {
		setVoicemailAddress(Address::create(voicemailUri));
	}

	mInstantMessagingEncryptionMandatory = !!linphone_config_get_bool(config, key, "im_encryption_mandatory", false);

	string supported_tags = linphone_config_get_string(config, key, "supported", "empty");
	if (supported_tags != "empty") {
		vector<string> splitTags = bctoolbox::Utils::split(supported_tags, ",");
		list<string> supportedTagsList;
		for (const auto &tag : splitTags)
			supportedTagsList.push_back(Utils::trim(tag));
		mSupportedTagsList.mList = supportedTagsList;
		mUseSupportedTags = true;
	}

	readCustomParamsFromConfigFile(config, key);
}

AccountParams::AccountParams(const AccountParams &other) : HybridObject(other), CustomParams(other) {
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
	mForceRegisterOnPush = other.mForceRegisterOnPush;
	mUnregisterAtStop = other.mUnregisterAtStop;
	mUseInternationalPrefixForCallsAndChats = other.mUseInternationalPrefixForCallsAndChats;
	mAllowCpimMessagesInBasicChatRooms = other.mAllowCpimMessagesInBasicChatRooms;

	mUserData = other.mUserData;

	mInternationalPrefix = other.mInternationalPrefix;
	mInternationalPrefixIsoCountryCode = other.mInternationalPrefixIsoCountryCode;
	mProxy = other.mProxy;
	mRealm = other.mRealm;
	mQualityReportingCollector = other.mQualityReportingCollector;
	mContactParameters = other.mContactParameters;
	mContactUriParameters = other.mContactUriParameters;
	mRefKey = other.mRefKey;
	mDependsOn = other.mDependsOn;
	mIdKey = other.mIdKey;

	if (other.mConferenceFactoryAddress) {
		mConferenceFactoryAddress = other.mConferenceFactoryAddress->clone()->toSharedPtr();
	} else {
		mConferenceFactoryAddress = nullptr;
	}

	if (other.mAudioVideoConferenceFactoryAddress) {
		mAudioVideoConferenceFactoryAddress = other.mAudioVideoConferenceFactoryAddress->clone()->toSharedPtr();
	} else {
		mAudioVideoConferenceFactoryAddress = nullptr;
	}

	mCcmpServerUrl = other.mCcmpServerUrl;
	mCcmpUserId = other.mCcmpUserId;

	mFileTransferServer = other.mFileTransferServer;

	setRoutes(other.mRoutes);
	mPrivacy = other.mPrivacy;
	mIdentity = other.mIdentity;
	if (other.mIdentityAddress) {
		mIdentityAddress = other.mIdentityAddress->clone()->toSharedPtr();
	} else {
		mIdentityAddress = nullptr;
	}
	if (other.mProxyAddress) {
		mProxyAddress = other.mProxyAddress->clone()->toSharedPtr();
	} else {
		mProxyAddress = nullptr;
	}

	mAvpfMode = other.mAvpfMode;

	setNatPolicy(other.mNatPolicy);

	mPushNotificationConfig = other.mPushNotificationConfig->clone();
	mRtpBundleEnabled = other.mRtpBundleEnabled;
	mRtpBundleAssumption = other.mRtpBundleAssumption;
	if (other.mCustomContact) {
		mCustomContact = other.mCustomContact->clone()->toSharedPtr();
	} else {
		mCustomContact = nullptr;
	}
	mLimeServerUrl = other.mLimeServerUrl;
	mLimeAlgo = other.mLimeAlgo;
	mPictureUri = other.mPictureUri;
	if (other.mMwiServerAddress) {
		mMwiServerAddress = other.mMwiServerAddress->clone()->toSharedPtr();
	} else {
		mMwiServerAddress = nullptr;
	}
	if (other.mVoicemailAddress) {
		mVoicemailAddress = other.mVoicemailAddress->clone()->toSharedPtr();
	} else {
		mVoicemailAddress = nullptr;
	}

	mInstantMessagingEncryptionMandatory = other.mInstantMessagingEncryptionMandatory;

	mSupportedTagsList = other.mSupportedTagsList;
	mUseSupportedTags = other.mUseSupportedTags;
}

AccountParams::~AccountParams() {
	if (mConferenceFactoryAddressCstr) {
		ms_free(mConferenceFactoryAddressCstr);
		mConferenceFactoryAddressCstr = nullptr;
	}
	if (mMwiServerAddressCstr) {
		ms_free(mMwiServerAddressCstr);
		mMwiServerAddressCstr = nullptr;
	}
	if (mVoicemailAddressCstr) {
		ms_free(mVoicemailAddressCstr);
		mVoicemailAddressCstr = nullptr;
	}
	if (mCcmpServerUrlCstr) {
		ms_free(mCcmpServerUrlCstr);
		mCcmpServerUrlCstr = nullptr;
	}
	if (mCcmpUserIdCstr) {
		ms_free(mCcmpUserIdCstr);
		mCcmpUserIdCstr = nullptr;
	}
	if (mPushNotificationConfig) mPushNotificationConfig->unref();
	if (mRoutesCString) {
		bctbx_list_free_with_data(mRoutesCString, (bctbx_list_free_func)bctbx_free);
	}
}

AccountParams *AccountParams::clone() const {
	return new AccountParams(*this);
}

// -----------------------------------------------------------------------------

void AccountParams::setExpires(int expires) {
	if (expires < 0) expires = 600;
	mExpires = expires;
}

void AccountParams::setQualityReportingInterval(int qualityReportingInterval) {
	mQualityReportingInterval = qualityReportingInterval;
}

void AccountParams::setPublishExpires(int publishExpires) {
	mPublishExpires = publishExpires;
}

void AccountParams::setAvpfRrInterval(uint8_t avpfRrInterval) {
	if (avpfRrInterval > 5) avpfRrInterval = 5;
	mAvpfRrInterval = avpfRrInterval;
}

void AccountParams::setRegisterEnabled(bool enable) {
	mRegisterEnabled = enable;
}

void AccountParams::setDialEscapePlusEnabled(bool enable) {
	mDialEscapePlusEnabled = enable;
}

void AccountParams::setQualityReportingEnabled(bool enable) {
	mQualityReportingEnabled = enable;
}

void AccountParams::setPublishEnabled(bool enable) {
	mPublishEnabled = enable;
}

void AccountParams::setOutboundProxyEnabled(bool enable) {
	// If enable we remove all routes to only have the server address as route
	// If disable since we should only have the server address route, we can remove the list
	// Use only the proxy as route
	if (enable) {
		if (!mProxyAddress) {
			lError() << "Can't enable outbound proxy without having set the proxy address first!";
			return;
		}

		mRoutes.clear();
		mRoutes.push_front(mProxyAddress);
	} else {
		mRoutes.clear();
	}
	updateRoutesCString();
}

void AccountParams::setPushNotificationAllowed(bool allow) {
	mPushNotificationAllowed = allow;
}

void AccountParams::setRemotePushNotificationAllowed(bool allow) {
	mRemotePushNotificationAllowed = allow;
}

void AccountParams::setForceRegisterOnPushNotification(bool force) {
	mForceRegisterOnPush = force;
}

void AccountParams::setUnregisterAtStop(bool enable) {
	mUnregisterAtStop = enable;
}

void AccountParams::setUseInternationalPrefixForCallsAndChats(bool enable) {
	mUseInternationalPrefixForCallsAndChats = enable;
}

void AccountParams::setCpimMessagesAllowedInBasicChatRooms(bool allow) {
	mAllowCpimMessagesInBasicChatRooms = allow;
}

void AccountParams::setUserData(void *userData) {
	mUserData = userData;
}

void AccountParams::setInternationalPrefix(const std::string &internationalPrefix) {
	mInternationalPrefix = internationalPrefix;
}

void AccountParams::setInternationalPrefixIsoCountryCode(const std::string &internationalPrefixIsoCountryCode) {
	mInternationalPrefixIsoCountryCode = internationalPrefixIsoCountryCode;
}

void AccountParams::setProxy(const std::string &proxy) {
	mProxy = proxy;
}

void AccountParams::setRealm(const std::string &realm) {
	mRealm = realm;
}

void AccountParams::setQualityReportingCollector(const std::string &qualityReportingCollector) {
	if (!qualityReportingCollector.empty()) {
		std::shared_ptr<Address> addr = Address::create(qualityReportingCollector);

		if (!addr) {
			lError() << "Invalid SIP collector URI: " << qualityReportingCollector
			         << ". Quality reporting will be DISABLED.";
		} else {
			mQualityReportingCollector = qualityReportingCollector;
		}
	}
}

void AccountParams::setContactParameters(const std::string &contactParameters) {
	mContactParameters = contactParameters;
}

void AccountParams::setContactUriParameters(const std::string &contactUriParameters) {
	mContactUriParameters = contactUriParameters;
}

void AccountParams::setRefKey(const std::string &refKey) {
	mRefKey = refKey;
}

void AccountParams::setDependsOn(const std::string &dependsOn) {
	mDependsOn = dependsOn;
}

void AccountParams::setIdKey(const std::string &idKey) {
	mIdKey = idKey;
}

void AccountParams::setConferenceFactoryUri(const std::string &conferenceFactoryUri) {
	setConferenceFactoryAddress(conferenceFactoryUri.empty() ? nullptr : Address::create(conferenceFactoryUri));
}

void AccountParams::setConferenceFactoryAddress(const std::shared_ptr<const Address> &conferenceFactoryAddress) {
	if (mConferenceFactoryAddress != nullptr) {
		mConferenceFactoryAddress = nullptr;
	}
	if (conferenceFactoryAddress != nullptr) {
		mConferenceFactoryAddress = conferenceFactoryAddress->clone()->toSharedPtr();
	}
}

void AccountParams::setAudioVideoConferenceFactoryAddress(
    const std::shared_ptr<const Address> &audioVideoConferenceFactoryAddress) {
	if (mAudioVideoConferenceFactoryAddress != nullptr) {
		mAudioVideoConferenceFactoryAddress = nullptr;
	}
	if (audioVideoConferenceFactoryAddress != nullptr) {
		mAudioVideoConferenceFactoryAddress = audioVideoConferenceFactoryAddress->clone()->toSharedPtr();
	}
}

void AccountParams::setCcmpServerUrl(const std::string &ccmpServerUrl) {
	mCcmpServerUrl = ccmpServerUrl;
}

void AccountParams::setFileTransferServer(const std::string &fileTransferServer) {
	mFileTransferServer = fileTransferServer;
}

LinphoneStatus AccountParams::setRoutes(const std::list<std::shared_ptr<Address>> &routes) {
	mRoutes = routes;
	updateRoutesCString();
	return 0;
}

LinphoneStatus AccountParams::setRoutesFromStringList(const bctbx_list_t *routes) {
	mRoutes.clear();
	bctbx_list_t *iterator = (bctbx_list_t *)routes;
	bool error = false;
	while (iterator != nullptr) {
		char *route = (char *)bctbx_list_get_data(iterator);
		if (route != nullptr && route[0] != '\0') {
			string tmp;
			/*try to prepend 'sip:' */
			if (strstr(route, "sip:") == nullptr && strstr(route, "sips:") == nullptr) {
				tmp.append("sip:");
			}
			tmp.append(route);

			SalAddress *addr = sal_address_new(tmp.c_str());
			if (addr != nullptr) {
				mRoutes.emplace_back(Address::create(addr, true));
			} else {
				error = true;
			}
		}
		iterator = bctbx_list_next(iterator);
	}
	updateRoutesCString();
	return (error) ? -1 : 0;
}

void AccountParams::setPrivacy(LinphonePrivacyMask privacy) {
	mPrivacy = privacy;
}

LinphoneStatus AccountParams::setIdentityAddress(const std::shared_ptr<const Address> &identityAddress) {
	if (!identityAddress || identityAddress->getUsername().empty()) {
		lWarning() << "Invalid sip identity: " << identityAddress->toString();
		return -1;
	}

	mIdentityAddress = identityAddress->clone()->toSharedPtr();
	mIdentity = mIdentityAddress->toString();

	return 0;
}

void AccountParams::setAvpfMode(LinphoneAVPFMode avpfMode) {
	mAvpfMode = avpfMode;
}

void AccountParams::setNatPolicy(const shared_ptr<NatPolicy> &natPolicy) {
	mNatPolicy = natPolicy ? natPolicy->clone()->toSharedPtr() : nullptr;
}

void AccountParams::setPushNotificationConfig(PushNotificationConfig *pushNotificationConfig) {
	if (mPushNotificationConfig) mPushNotificationConfig->unref();

	mPushNotificationConfig = pushNotificationConfig;
	mPushNotificationConfig->ref();
}

void AccountParams::enableRtpBundle(bool value) {
	mRtpBundleEnabled = value;
}

void AccountParams::enableRtpBundleAssumption(bool value) {
	mRtpBundleAssumption = value;
}

void AccountParams::setCustomContact(const std::shared_ptr<const Address> &contact) {
	mCustomContact = contact ? contact->clone()->toSharedPtr() : nullptr;
}

void AccountParams::setCustomContact(const string &contact) {
	std::shared_ptr<Address> address = !contact.empty() ? Address::create(contact) : nullptr;
	if (address == nullptr && !contact.empty()) {
		lError() << "AccountParams: invalid custom contact '" << contact << "'";
	}
	mCustomContact = address;
}

// -----------------------------------------------------------------------------

int AccountParams::getExpires() const {
	return mExpires;
}

int AccountParams::getQualityReportingInterval() const {
	return mQualityReportingInterval;
}

int AccountParams::getPublishExpires() const {
	// Default value has changed, but still take into account config files with the last one which was -1.
	return mPublishExpires < 0 ? 600 : mPublishExpires;
}

uint8_t AccountParams::getAvpfRrInterval() const {
	return mAvpfRrInterval;
}

bool AccountParams::getRegisterEnabled() const {
	return mRegisterEnabled;
}

bool AccountParams::getDialEscapePlusEnabled() const {
	return mDialEscapePlusEnabled;
}

bool AccountParams::getQualityReportingEnabled() const {
	return mQualityReportingEnabled;
}

bool AccountParams::getPublishEnabled() const {
	return mPublishEnabled;
}

bool AccountParams::getOutboundProxyEnabled() const {
	std::shared_ptr<Address> address = !mRoutes.empty() ? mRoutes.front() : nullptr;
	return address != nullptr && mProxyAddress != nullptr && mProxyAddress->weakEqual(*address);
}

bool AccountParams::getPushNotificationAllowed() const {
	return mPushNotificationAllowed;
}

bool AccountParams::getRemotePushNotificationAllowed() const {
	return mRemotePushNotificationAllowed;
}

bool AccountParams::getForceRegisterOnPushNotification() const {
	return mForceRegisterOnPush;
}
bool AccountParams::getUnregisterAtStop() const {
	return mUnregisterAtStop;
}

bool AccountParams::getUseInternationalPrefixForCallsAndChats() const {
	return mUseInternationalPrefixForCallsAndChats;
}

bool AccountParams::isPushNotificationAvailable() const {
	string prid = mPushNotificationConfig->getPrid();
	string param = mPushNotificationConfig->getParam();
	string basicToken = mPushNotificationConfig->getVoipToken();
	string remoteToken = mPushNotificationConfig->getRemoteToken();
	string bundle = mPushNotificationConfig->getBundleIdentifer();
	// Accounts can support multiple types of push. Push notification is ready when all supported push's tokens to set

	bool paramAvailable = !param.empty() || !bundle.empty();
	bool pridAvailable = !prid.empty() || !((mPushNotificationAllowed && basicToken.empty()) ||
	                                        (mRemotePushNotificationAllowed && remoteToken.empty()));
	return paramAvailable && pridAvailable;
}

bool AccountParams::isCpimMessagesAllowedInBasicChatRooms() const {
	return mAllowCpimMessagesInBasicChatRooms;
}

void *AccountParams::getUserData() const {
	return mUserData;
}

const std::string &AccountParams::getInternationalPrefix() const {
	return mInternationalPrefix;
}

const std::string &AccountParams::getInternationalPrefixIsoCountryCode() const {
	return mInternationalPrefixIsoCountryCode;
}

const char *AccountParams::getDomainCstr() const {
	return mIdentityAddress ? mIdentityAddress->getDomainCstr() : nullptr;
}

const std::string AccountParams::getDomain() const {
	return mIdentityAddress ? mIdentityAddress->getDomain() : std::string();
}

const std::string &AccountParams::getProxy() const {
	return mProxy;
}

const std::string &AccountParams::getRealm() const {
	return mRealm;
}

const std::string &AccountParams::getQualityReportingCollector() const {
	return mQualityReportingCollector;
}

const std::string &AccountParams::getContactParameters() const {
	return mContactParameters;
}

const std::string &AccountParams::getContactUriParameters() const {
	return mContactUriParameters;
}

const std::string &AccountParams::getRefKey() const {
	return mRefKey;
}

const std::string &AccountParams::getDependsOn() const {
	return mDependsOn;
}

const std::string &AccountParams::getIdKey() const {
	return mIdKey;
}

const char *AccountParams::getConferenceFactoryCstr() const {
	if (mConferenceFactoryAddressCstr) {
		ms_free(mConferenceFactoryAddressCstr);
		mConferenceFactoryAddressCstr = nullptr;
	}
	if (mConferenceFactoryAddress) {
		mConferenceFactoryAddressCstr = mConferenceFactoryAddress->asStringUriOnlyCstr();
	}
	return mConferenceFactoryAddressCstr;
}

std::shared_ptr<const Address> AccountParams::getConferenceFactoryAddress() const {
	return mConferenceFactoryAddress;
}

std::shared_ptr<const Address> AccountParams::getAudioVideoConferenceFactoryAddress() const {
	return mAudioVideoConferenceFactoryAddress;
}

const char *AccountParams::getCcmpUserIdCstr() const {
	const auto &userId = getCcmpUserId();
	if (mCcmpUserIdCstr) {
		ms_free(mCcmpUserIdCstr);
		mCcmpUserIdCstr = nullptr;
	}
	if (!userId.empty()) {
		mCcmpUserIdCstr = ms_strdup(userId.c_str());
	}
	return mCcmpUserIdCstr;
}

const std::string &AccountParams::getCcmpUserId() const {
	if (mCcmpUserId.empty()) {
		mCcmpUserId = Utils::getXconId(mIdentityAddress);
	}
	return mCcmpUserId;
}

const char *AccountParams::getCcmpServerUrlCstr() const {
	if (mCcmpServerUrlCstr) {
		ms_free(mCcmpServerUrlCstr);
		mCcmpServerUrlCstr = nullptr;
	}
	if (!mCcmpServerUrl.empty()) {
		mCcmpServerUrlCstr = ms_strdup(mCcmpServerUrl.c_str());
	}
	return mCcmpServerUrlCstr;
}

const std::string &AccountParams::getCcmpServerUrl() const {
	return mCcmpServerUrl;
}

const std::string &AccountParams::getFileTransferServer() const {
	return mFileTransferServer;
}

const std::string &AccountParams::getIdentity() const {
	return mIdentity;
}

const std::list<std::shared_ptr<Address>> &AccountParams::getRoutes() const {
	return mRoutes;
}

const std::list<std::string> AccountParams::getRoutesString() const {
	std::list<std::string> routesString;
	for (const auto &r : mRoutes) {
		routesString.push_back(r->toString());
	}
	return routesString;
}

void AccountParams::updateRoutesCString() {
	if (mRoutesCString) {
		bctbx_list_free_with_data(mRoutesCString, (bctbx_list_free_func)bctbx_free);
		mRoutesCString = nullptr;
	}
	const auto routeString = getRoutesString();
	if (!routeString.empty()) {
		mRoutesCString = L_GET_C_LIST_FROM_CPP_LIST(routeString);
	}
}

const bctbx_list_t *AccountParams::getRoutesCString() const {
	return mRoutesCString;
}

LinphonePrivacyMask AccountParams::getPrivacy() const {
	return mPrivacy;
}

std::shared_ptr<Address> AccountParams::getIdentityAddress() const {
	return mIdentityAddress;
}

LinphoneAVPFMode AccountParams::getAvpfMode() const {
	return mAvpfMode;
}

shared_ptr<NatPolicy> AccountParams::getNatPolicy() const {
	return mNatPolicy;
}

PushNotificationConfig *AccountParams::getPushNotificationConfig() const {
	return mPushNotificationConfig;
}

bool AccountParams::rtpBundleEnabled() const {
	return mRtpBundleEnabled;
}

bool AccountParams::rtpBundleAssumptionEnabled() const {
	return mRtpBundleAssumption;
}

std::shared_ptr<const Address> AccountParams::getCustomContact() const {
	return mCustomContact;
}

void AccountParams::setLimeServerUrl(const std::string &url) {
	mLimeServerUrl = url;
}

const std::string &AccountParams::getLimeServerUrl() const {
	return mLimeServerUrl;
}

void AccountParams::setLimeAlgo(const std::string &algo) {
	mLimeAlgo = algo;
}

const std::string &AccountParams::getLimeAlgo() const {
	return mLimeAlgo;
}

void AccountParams::setPictureUri(const std::string &uri) {
	mPictureUri = uri;
}

const std::string &AccountParams::getPictureUri() const {
	return mPictureUri;
}

void AccountParams::setMwiServerAddress(const std::shared_ptr<Address> &address) {
	mMwiServerAddress = address;
}

std::shared_ptr<const Address> AccountParams::getMwiServerAddress() const {
	return mMwiServerAddress;
}

void AccountParams::setVoicemailAddress(const std::shared_ptr<Address> &address) {
	mVoicemailAddress = address;
}

std::shared_ptr<const Address> AccountParams::getVoicemailAddress() const {
	return mVoicemailAddress;
}

// -----------------------------------------------------------------------------

LinphoneStatus AccountParams::setServerAddress(const std::shared_ptr<const Address> &serverAddr) {
	if (serverAddr == nullptr) {
		mProxyAddress.reset();
		mProxy = "";
		return 0;
	}
	bool outboundProxyEnabled = getOutboundProxyEnabled();

	mProxyAddress = serverAddr->clone()->toSharedPtr();

	mProxy = mProxyAddress->toString();

	if (outboundProxyEnabled) {
		// Setting this to true will do the job of setting the routes
		setOutboundProxyEnabled(true);
	}

	return 0;
}

std::shared_ptr<const Address> AccountParams::getServerAddress() const {
	return mProxyAddress;
}

LinphoneStatus AccountParams::setServerAddressAsString(const std::string &serverAddr) {
	std::shared_ptr<Address> addr = nullptr;

	if (!serverAddr.empty()) {
		if (serverAddr.rfind("sip:") == string::npos && serverAddr.rfind("sips:") == string::npos) {
			string modified("");
			modified.append("sip:").append(serverAddr);
			addr = Address::create(modified);
		}

		if (addr == nullptr) addr = Address::create(serverAddr);
		if (addr) {
			bool outboundProxyEnabled = getOutboundProxyEnabled();

			mProxyAddress = addr->clone()->toSharedPtr();

			mProxy = mProxyAddress->toString();

			if (outboundProxyEnabled) {
				// Setting this to true will do the job of setting the routes
				setOutboundProxyEnabled(true);
			}
		} else {
			lWarning() << "Could not parse " << serverAddr;
			return -1;
		}
	}

	return 0;
}

const std::string &AccountParams::getServerAddressAsString() const {
	return mProxy;
}

void AccountParams::setTransport(LinphonePrivate::Transport transport) {
	mProxyAddress->setTransport(transport);

	mProxy = mProxyAddress->toString();

	if (getOutboundProxyEnabled()) {
		setOutboundProxyEnabled(true);
	}
}

LinphonePrivate::Transport AccountParams::getTransport() const {
	return mProxyAddress->getTransport();
}

const char *AccountParams::getMwiServerAddressCstr() const {
	if (mMwiServerAddressCstr) {
		ms_free(mMwiServerAddressCstr);
		mMwiServerAddressCstr = nullptr;
	}
	if (mMwiServerAddress) {
		mMwiServerAddressCstr = mMwiServerAddress->asStringUriOnlyCstr();
	}
	return mMwiServerAddressCstr;
}

const char *AccountParams::getVoicemailAddressCstr() const {
	if (mVoicemailAddressCstr) {
		ms_free(mVoicemailAddressCstr);
		mVoicemailAddressCstr = nullptr;
	}
	if (mVoicemailAddress) {
		mVoicemailAddressCstr = mVoicemailAddress->asStringUriOnlyCstr();
	}
	return mVoicemailAddressCstr;
}

bool AccountParams::isInstantMessagingEncryptionMandatory() const {
	return mInstantMessagingEncryptionMandatory;
}

void AccountParams::setInstantMessagingEncryptionMandatory(bool mandatory) {
	mInstantMessagingEncryptionMandatory = mandatory;
}

const std::list<std::string> &AccountParams::getSupportedTagsList() const {
	return mSupportedTagsList.mList;
}

const bctbx_list_t *AccountParams::getSupportedTagsCList() const {
	return mSupportedTagsList.getCList();
}

void AccountParams::setSupportedTagsList(const std::list<std::string> &supportedTagsList) {
	mSupportedTagsList.mList = supportedTagsList;
	mUseSupportedTags = true;
}

bool AccountParams::useSupportedTags() const {
	return mUseSupportedTags;
}

void AccountParams::writeToConfigFile(LinphoneConfig *config, int index) {
	char key[50];

	snprintf(key, sizeof(key), "proxy_%i", index);
	linphone_config_clean_section(config, key);

	if (!mProxy.empty()) {
		linphone_config_set_string(config, key, "reg_proxy", mProxy.c_str());
	}
	if (!mRoutes.empty()) {
		auto routesString = getRoutesCString();
		linphone_config_set_string_list(config, key, "reg_route", routesString);
	} else {
		linphone_config_clean_entry(config, key, "reg_route");
	}
	if (!mIdentity.empty()) {
		linphone_config_set_string(config, key, "reg_identity", mIdentity.c_str());
	}
	if (!mRealm.empty()) {
		linphone_config_set_string(config, key, "realm", mRealm.c_str());
	}
	if (!mContactParameters.empty()) {
		linphone_config_set_string(config, key, "contact_parameters", mContactParameters.c_str());
	}
	if (!mContactUriParameters.empty()) {
		linphone_config_set_string(config, key, "contact_uri_parameters", mContactUriParameters.c_str());
	}
	if (!mQualityReportingCollector.empty()) {
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
	linphone_config_set_string(config, key, "dial_prefix", mInternationalPrefix.c_str());
	linphone_config_set_string(config, key, "dial_prefix_iso_country_code", mInternationalPrefixIsoCountryCode.c_str());
	linphone_config_set_int(config, key, "use_dial_prefix_for_calls_and_chats",
	                        mUseInternationalPrefixForCallsAndChats);
	linphone_config_set_int(config, key, "privacy", (int)mPrivacy);
	linphone_config_set_int(config, key, "push_notification_allowed", (int)mPushNotificationAllowed);
	linphone_config_set_int(config, key, "remote_push_notification_allowed", (int)mRemotePushNotificationAllowed);
	linphone_config_set_int(config, key, "force_register_on_push", (int)mForceRegisterOnPush);
	linphone_config_set_int(config, key, "unregister_at_stop", (int)mUnregisterAtStop);
	linphone_config_set_int(config, key, "cpim_in_basic_chat_rooms_enabled", (int)mAllowCpimMessagesInBasicChatRooms);
	if (!mRefKey.empty()) linphone_config_set_string(config, key, "refkey", mRefKey.c_str());
	if (!mDependsOn.empty()) linphone_config_set_string(config, key, "depends_on", mDependsOn.c_str());
	if (!mIdKey.empty()) linphone_config_set_string(config, key, "idkey", mIdKey.c_str());
	linphone_config_set_int(config, key, "publish_expires", mPublishExpires);

	if (mNatPolicy != nullptr) {
		linphone_config_set_string(config, key, "nat_policy_ref", mNatPolicy->getRef().c_str());
	}

	if (mConferenceFactoryAddress != nullptr) {
		linphone_config_set_string(config, key, "conference_factory_uri", getConferenceFactoryCstr());
	}

	if (mAudioVideoConferenceFactoryAddress != nullptr) {
		char *factory_address = mAudioVideoConferenceFactoryAddress->asStringUriOnlyCstr();
		linphone_config_set_string(config, key, "audio_video_conference_factory_uri", factory_address);
		ms_free(factory_address);
	}

	if (!mCcmpServerUrl.empty()) {
		linphone_config_set_string(config, key, "ccmp_server_url", mCcmpServerUrl.c_str());
	}

	linphone_config_set_int(config, key, "rtp_bundle", mRtpBundleEnabled);
	linphone_config_set_int(config, key, "rtp_bundle_assumption", mRtpBundleAssumption);

	writeCustomParamsToConfigFile(config, key);

	linphone_config_set_string(config, key, "lime_server_url", mLimeServerUrl.c_str());
	linphone_config_set_string(config, key, "lime_algo", mLimeAlgo.c_str());
	linphone_config_set_string(config, key, "picture_uri", mPictureUri.c_str());
	if (mMwiServerAddress) {
		linphone_config_set_string(config, key, "mwi_server_uri", getMwiServerAddressCstr());
	}
	if (mVoicemailAddress) {
		linphone_config_set_string(config, key, "voicemail_uri", getVoicemailAddressCstr());
	}

	linphone_config_set_bool(config, key, "im_encryption_mandatory", mInstantMessagingEncryptionMandatory);

	if (mUseSupportedTags) {
		std::ostringstream result;
		auto cppList = mSupportedTagsList.mList;
		for (auto &tag : cppList) {
			if (tag != cppList.front()) {
				result << ", ";
			}
			result << tag;
		}
		linphone_config_set_string(config, key, "supported", result.str().c_str());
	}
}

LINPHONE_END_NAMESPACE
