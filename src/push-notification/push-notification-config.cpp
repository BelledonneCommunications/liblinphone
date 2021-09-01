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

#include "push-notification-config.h"
#include "address/address.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

PushNotificationConfig::PushNotificationConfig() {
	mPushParams[PushConfigProviderKey] = "";
	mPushParams[PushConfigParamKey] = "";
	mPushParams[PushConfigPridKey] = "";
	mPushParams[PushConfigTimeoutKey] = "0";
	mPushParams[PushConfigSilentKey] = "1";
	mPushParams[PushConfigMsgStrKey] = "IM_MSG";
	mPushParams[PushConfigCallStrKey] = "IC_MSG";
	mPushParams[PushConfigGroupChatStrKey] = "GC_MSG";
	mPushParams[PushConfigCallSoundKey] = "notes_of_the_optimistic.caf";
	mPushParams[PushConfigMsgSoundKey] = "msg.caf";
	
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
}

PushNotificationConfig* PushNotificationConfig::clone () const {
	return new PushNotificationConfig(*this);
}

PushNotificationConfig& PushNotificationConfig::operator=(const PushNotificationConfig& other)
 {
	 if (this != &other) {
		 mPushParams = other.mPushParams;
		 mTeamId = other.mTeamId;
		 mBundleIdentifer = other.mBundleIdentifer;
		 mVoipToken = other.mVoipToken;
		 mRemoteToken = other.mRemoteToken;
	 }
	 return *this;
 }

bool PushNotificationConfig::isEqual(const PushNotificationConfig& other) const {
	return 	mPushParams == other.mPushParams &&
	mTeamId == other.mTeamId &&
	mBundleIdentifer == other.mBundleIdentifer &&
	mVoipToken == other.mVoipToken &&
	mRemoteToken == other.mRemoteToken;
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
	mVoipToken = voipToken;
}

const string &PushNotificationConfig::getRemoteToken() const {
	return mRemoteToken;
}
void PushNotificationConfig::setRemoteToken(const string &remoteToken) {
	mRemoteToken = remoteToken;
}

const string &PushNotificationConfig::getTeamId() const {
	return mTeamId;
}
void PushNotificationConfig::setTeamId(const string &teamId) {
	mTeamId = teamId;
}

void PushNotificationConfig::generatePushParams(bool voipPushAllowed, bool remotePushAllowed) {
	
	if (mPushParams[PushConfigProviderKey].empty()) {
#ifdef __ANDROID__
		mPushParams[PushConfigProviderKey] = "fcm";
#elif TARGET_OS_IPHONE
		mPushParams[PushConfigProviderKey] = "apns";
#endif
	}
	
	if (mPushParams[PushConfigParamKey].empty()) {
		string services;
		if (voipPushAllowed) {
			services += "voip";
			if (remotePushAllowed)
				services += "&";
		}
		if (remotePushAllowed)
			services += "remote";

		mPushParams[PushConfigParamKey] = mTeamId + "." + mBundleIdentifer + "." + services;
	}

	if (mPushParams[PushConfigPridKey].empty()) {
		string newPrid;
		if (voipPushAllowed) {
			newPrid += mVoipToken;
			if (remotePushAllowed)
				newPrid += "&";
		}
		if (remotePushAllowed)
			newPrid += mRemoteToken;
		mPushParams[PushConfigPridKey] = newPrid;
	}
}

map<string, string> const& PushNotificationConfig::getPushParamsMap() {
	return mPushParams;
}

string PushNotificationConfig::asString(bool withRemoteSpecificParams, bool isLegacy) const {
	string serializedConfig;
	auto convertParamNameToLegacyIfNeeded = [&](string const& paramName) -> string {
		if (isLegacy) {
			if (paramName == PushConfigPridKey) return string("pn-tok");
			if (paramName == PushConfigParamKey) return string("app-id");
			if (paramName == PushConfigProviderKey) return string("pn-type");
		}
		
		return paramName;
	};
	
	auto appendParam = [&](string const& paramName) {
		if (!mPushParams.at(paramName).empty())
			serializedConfig += convertParamNameToLegacyIfNeeded(paramName) + "=" + mPushParams.at(paramName) + ";";
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
	}
	
	return serializedConfig;
}

void PushNotificationConfig::readPushParamsFromString(string const& serializedConfig) {
	Address pushParamsWrapper("sip:dummy;" + serializedConfig);
	for (auto &param : mPushParams) {
		string paramValue = pushParamsWrapper.getUriParamValue(param.first);
		if (paramValue.empty()) {
			// Check for legacy parameters
			if (param.first == PushConfigPridKey) paramValue = pushParamsWrapper.getUriParamValue("pn-tok");
			if (param.first == PushConfigParamKey) paramValue = pushParamsWrapper.getUriParamValue("app-id");
			if (param.first == PushConfigProviderKey) paramValue = pushParamsWrapper.getUriParamValue("pn-type");
		}
		if (!paramValue.empty())
			param.second = paramValue;
	}
}

LINPHONE_END_NAMESPACE
