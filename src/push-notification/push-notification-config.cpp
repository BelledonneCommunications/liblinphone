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

#include "push-notification-config.h"
#include "address/address.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

PushNotificationConfig::PushNotificationConfig() {
#ifdef __ANDROID__
	mPushParams[PushConfigProviderKey] = "fcm";
#elif TARGET_OS_IPHONE
	mPushParams[PushConfigProviderKey] = "apns";
#else
	mPushParams[PushConfigProviderKey] = "";
#endif
	mPushParams[PushConfigParamKey] = "";
	mPushParams[PushConfigPridKey] = "";
	mPushParams[PushConfigTimeoutKey] = "0";
	mPushParams[PushConfigSilentKey] = "1";
	mPushParams[PushConfigMsgStrKey] = "IM_MSG";
	mPushParams[PushConfigCallStrKey] = "IC_MSG";
	mPushParams[PushConfigGroupChatStrKey] = "GC_MSG";
	mPushParams[PushConfigCallSoundKey] = "notes_of_the_optimistic.caf";
	mPushParams[PushConfigMsgSoundKey] = "msg.caf";
	mPushParams[PushConfigRemotePushIntervalKey] = "";

	mTeamId = "ABCD1234";
	mBundleIdentifer = "";
	mVoipToken = "";
	mRemoteToken = "";
}

PushNotificationConfig::PushNotificationConfig(const PushNotificationConfig &other) : HybridObject(other) {
	mPushParams = other.mPushParams;
	mTeamId = other.mTeamId;
	mBundleIdentifer = other.mBundleIdentifer;
	mVoipToken = other.mVoipToken;
	mRemoteToken = other.mRemoteToken;
	mTokensHaveChanged = other.mTokensHaveChanged;
}

PushNotificationConfig *PushNotificationConfig::clone() const {
	return new PushNotificationConfig(*this);
}

PushNotificationConfig &PushNotificationConfig::operator=(const PushNotificationConfig &other) {
	if (this != &other) {
		mPushParams = other.mPushParams;
		mTeamId = other.mTeamId;
		mBundleIdentifer = other.mBundleIdentifer;
		mVoipToken = other.mVoipToken;
		mRemoteToken = other.mRemoteToken;
		mTokensHaveChanged = other.mTokensHaveChanged;
	}
	return *this;
}

bool PushNotificationConfig::isEqual(const PushNotificationConfig &other) const {
	return mPushParams == other.mPushParams && mTeamId == other.mTeamId && mBundleIdentifer == other.mBundleIdentifer &&
	       mVoipToken == other.mVoipToken && mRemoteToken == other.mRemoteToken;
}

const string &PushNotificationConfig::getProvider() const {
	return mPushParams.at(PushConfigProviderKey);
}
void PushNotificationConfig::setProvider(const string &provider) {
	mPushParams[PushConfigProviderKey] = provider;
}

const string &PushNotificationConfig::getMsgStr() const {
	return mPushParams.at(PushConfigMsgStrKey);
}
void PushNotificationConfig::setMsgStr(const string &msgStr) {
	mPushParams[PushConfigMsgStrKey] = msgStr;
}

const string &PushNotificationConfig::getCallStr() const {
	return mPushParams.at(PushConfigCallStrKey);
}
void PushNotificationConfig::setCallStr(const string &callStr) {
	mPushParams[PushConfigCallStrKey] = callStr;
}

const string &PushNotificationConfig::getGroupChatStr() const {
	return mPushParams.at(PushConfigGroupChatStrKey);
}
void PushNotificationConfig::setGroupChatStr(const string &groupChatStr) {
	mPushParams[PushConfigGroupChatStrKey] = groupChatStr;
}

const string &PushNotificationConfig::getPrid() const {
	return mPushParams.at(PushConfigPridKey);
}
void PushNotificationConfig::setPrid(const string &prid) {
	mPushParams[PushConfigPridKey] = prid;
}

const string &PushNotificationConfig::getCallSnd() const {
	return mPushParams.at(PushConfigCallSoundKey);
}
void PushNotificationConfig::setCallSnd(const string &callSnd) {
	mPushParams[PushConfigCallSoundKey] = callSnd;
}

const string &PushNotificationConfig::getMsgSnd() const {
	return mPushParams.at(PushConfigMsgSoundKey);
}
void PushNotificationConfig::setMsgSnd(const string &msgSnd) {
	mPushParams[PushConfigMsgSoundKey] = msgSnd;
}

const string &PushNotificationConfig::getParam() const {
	return mPushParams.at(PushConfigParamKey);
}
void PushNotificationConfig::setParam(const string &param) {
	mPushParams[PushConfigParamKey] = param;
}

const string &PushNotificationConfig::getBundleIdentifer() const {
	return mBundleIdentifer;
}
void PushNotificationConfig::setBundleIdentifer(const string &bundleIdentifer) {
	mBundleIdentifer = bundleIdentifer;
}

const string &PushNotificationConfig::getVoipToken() const {
	return mVoipToken;
}
void PushNotificationConfig::setVoipToken(const string &voipToken) {
	if (mVoipToken != voipToken) {
		mTokensHaveChanged = true;
		mVoipToken = voipToken;
	}
}

const string &PushNotificationConfig::getRemoteToken() const {
	return mRemoteToken;
}
void PushNotificationConfig::setRemoteToken(const string &remoteToken) {
	if (mVoipToken != remoteToken) {
		mTokensHaveChanged = true;
		mRemoteToken = remoteToken;
	}
}

const string &PushNotificationConfig::getTeamId() const {
	return mTeamId;
}
void PushNotificationConfig::setTeamId(const string &teamId) {
	mTeamId = teamId;
}

const string &PushNotificationConfig::getRemotePushInterval() const {
	return mPushParams.at(PushConfigRemotePushIntervalKey);
}
void PushNotificationConfig::setRemotePushInterval(const string &remotePushInterval) {
	mPushParams[PushConfigRemotePushIntervalKey] = remotePushInterval;
}

void PushNotificationConfig::generatePushParams(bool voipPushAllowed, bool remotePushAllowed) {
	if (mPushParams[PushConfigProviderKey].empty()) {
#ifdef __ANDROID__
		mPushParams[PushConfigProviderKey] = "fcm";
#elif TARGET_OS_IPHONE
		mPushParams[PushConfigProviderKey] = "apns";
#endif
	}
	/* Push notification may be allowed and requested to the system, but sometimes the tokens are not
	 * yet available. Modify the enablement so that we don't generate an ill-formed push parameter line.
	 */
	if (mRemoteToken.empty()) remotePushAllowed = false;
	if (mVoipToken.empty()) voipPushAllowed = false;

	if (mPushParams[PushConfigParamKey].empty() ||
	    (mTokensHaveChanged && (!mVoipToken.empty() || !mRemoteToken.empty()))) {
		string services;
		if (voipPushAllowed) {
			services += "voip";
			if (remotePushAllowed) services += "&";
		}
		if (remotePushAllowed) services += "remote";

		mPushParams[PushConfigParamKey] = mTeamId + "." + mBundleIdentifer + "." + services;
	}

	if (mPushParams[PushConfigPridKey].empty() ||
	    (mTokensHaveChanged && (!mVoipToken.empty() || !mRemoteToken.empty()))) {
		string newPrid;
		if (voipPushAllowed) {
			newPrid += mVoipToken;
			if (remotePushAllowed) newPrid += "&";
		}
		if (remotePushAllowed) newPrid += mRemoteToken;
		mPushParams[PushConfigPridKey] = newPrid;
	}
	mTokensHaveChanged = false;
}

map<string, string> const &PushNotificationConfig::getPushParamsMap() {
	return mPushParams;
}

string PushNotificationConfig::asString(bool withRemoteSpecificParams) const {
	string serializedConfig;

	auto appendParam = [&](string const &paramName) {
		if (!mPushParams.at(paramName).empty()) serializedConfig += paramName + "=" + mPushParams.at(paramName) + ";";
	};

	appendParam(PushConfigPridKey);
	appendParam(PushConfigProviderKey);
	appendParam(PushConfigParamKey);
	appendParam(PushConfigSilentKey);
	appendParam(PushConfigTimeoutKey);

	if (withRemoteSpecificParams) {
		appendParam(PushConfigMsgStrKey);
		appendParam(PushConfigCallStrKey);
		appendParam(PushConfigGroupChatStrKey);
		appendParam(PushConfigCallSoundKey);
		appendParam(PushConfigMsgSoundKey);
		appendParam(PushConfigRemotePushIntervalKey);
	}

	return serializedConfig;
}

void PushNotificationConfig::readPushParamsFromString(string const &serializedConfig) {
	std::shared_ptr<Address> pushParamsWrapper = Address::create("sip:dummy;" + serializedConfig);
	for (auto &param : mPushParams) {
		string paramValue = pushParamsWrapper->getUriParamValue(param.first);
		if (!paramValue.empty()) param.second = paramValue;
	}
}

LINPHONE_END_NAMESPACE
