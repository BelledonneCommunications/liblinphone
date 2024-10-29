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

#pragma once

#include "belle-sip/object++.hh"
#include "linphone/api/c-types.h"
#include <map>
using namespace std;
LINPHONE_BEGIN_NAMESPACE

static string const PushConfigProviderKey = "pn-provider";
static string const PushConfigPridKey = "pn-prid";
static string const PushConfigParamKey = "pn-param";
static string const PushConfigMsgStrKey = "pn-msg-str";
static string const PushConfigCallStrKey = "pn-call-str";
static string const PushConfigGroupChatStrKey = "pn-groupchat-str";
static string const PushConfigTimeoutKey = "pn-timeout";
static string const PushConfigSilentKey = "pn-silent";
static string const PushConfigCallSoundKey = "pn-call-snd";
static string const PushConfigMsgSoundKey = "pn-msg-snd";
static string const PushConfigRemotePushIntervalKey = "pn-call-remote-push-interval";

class PushNotificationConfig : public bellesip::HybridObject<LinphonePushNotificationConfig, PushNotificationConfig> {
public:
	PushNotificationConfig();
	// Reconstruct a PushNotificationConfig from a string obtain by calling PushNotificationconfig::asString()
	PushNotificationConfig(std::string const &serializedConfig);
	PushNotificationConfig(const PushNotificationConfig &other);

	PushNotificationConfig *clone() const override;
	PushNotificationConfig &operator=(const PushNotificationConfig &other);

	bool isEqual(const PushNotificationConfig &other) const;

	const string &getProvider() const;
	void setProvider(const string &provider);
	const string &getTeamId() const;
	void setTeamId(const string &teamId);
	const string &getMsgStr() const;
	void setMsgStr(const string &msgStr);
	const string &getCallStr() const;
	void setCallStr(const string &callStr);
	const string &getGroupChatStr() const;
	void setGroupChatStr(const string &groupChatStr);
	const string &getPrid() const;
	void setPrid(const string &prid);
	const string &getBundleIdentifer() const;
	void setBundleIdentifer(const string &bundleIdentifer);
	const string &getCallSnd() const;
	void setCallSnd(const string &callSnd);
	const string &getMsgSnd() const;
	void setMsgSnd(const string &msgSnd);
	const string &getBundleIdentifier() const;
	void setBundleIdentifier(const string &bundleIdentifier);
	const string &getVoipToken() const;
	void setVoipToken(const string &voipToken);
	const string &getRemoteToken() const;
	void setRemoteToken(const string &remoteToken);
	const string &getParam() const;
	void setParam(const string &param);
	const string &getRemotePushInterval() const;
	void setRemotePushInterval(const string &remotePushInterval);

	void generatePushParams(bool voipPushAllowed, bool remotePushAllowed);
	map<string, string> const &getPushParamsMap();

	/* Write the mPushParams map in the form : "param1=param1value;param2=param2value;"                         *
	 * MsgStr, CallStr, GroupChatStr, CallSnd and MsgSnd will only be saved if withRemoteSpecificParams == true *
	 * /!\ TeamId, BundleId, VoipToken and RemoteToken will not be saved /!\                                    */
	string asString(bool withRemoteSpecificParams = true) const;
	void readPushParamsFromString(string const &serializedConfig);

private:
	string mTeamId;
	string mBundleIdentifer;
	string mVoipToken;
	string mRemoteToken;
	bool mTokensHaveChanged = false;

	map<string, string> mPushParams;
};

LINPHONE_END_NAMESPACE
