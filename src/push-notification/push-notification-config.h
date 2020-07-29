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

#pragma once

#include <belle-sip/object++.hh>
#include "linphone/api/c-types.h"

using namespace std;
LINPHONE_BEGIN_NAMESPACE

class PushNotificationConfig: public bellesip::HybridObject<LinphonePushNotificationConfig, PushNotificationConfig> {
  public:
	PushNotificationConfig();

	
	const string &getProvider () const;
	void setProvider (const string &provider);
	const string &getTeamId () const;
	void setTeamId (const string &teamId);
	const string &getMsgStr () const;
	void setMsgStr (const string &msgStr);
	const string &getCallStr () const;
	void setCallStr (const string &callStr);
	const string &getGroupChatStr () const;
	void setGroupChatStr (const string &groupChatStr);
	const string &getPrid () const;
	void setPrid (const string &prid);
	const string &getBundleIdentifer () const;
	void setBundleIdentifer (const string &bundleIdentifer);
	const string &getServices () const;
	void setServices (const string &services);
	const string &getCallSnd () const;
	void setCallSnd (const string &callSnd);
	const string &getMsgSnd () const;
	void setMsgSnd (const string &msgSnd);
	const string &getBundleIdentifier () const;
	void setBundleIdentifier (const string &bundleIdentifier);
	const string &getVoipToken () const;
	void setVoipToken (const string &voipToken);
	const string &getRemoteToken () const;
	void setRemoteToken (const string &remoteToken);
	const string &getParam () const;
	void setParam (const string &param);
	

  private:
	string mProvider;
	string mTeamId;
	string mMsgStr;
	string mCallStr;
	string mGroupChatStr;
	string mPrid;
	string mBundleIdentifer;
	string mServices;
	string mCallSnd;
	string mMsgSnd;
	string mExtensions;
	string mVoipToken;
	string mRemoteToken;
	string mParam;
};

LINPHONE_END_NAMESPACE
