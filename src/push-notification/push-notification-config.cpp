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

using namespace std;

LINPHONE_BEGIN_NAMESPACE

PushNotificationConfig::PushNotificationConfig() {
	mProvider = "";
	mTeamId = "ABCD1234";
	mMsgStr = "IM_MSG";
	mCallStr = "IC_MSG";
	mGroupChatStr = "GC_MSG";
	mPrid = "";
	mBundleIdentifer = "";
	mServices = "";
	mCallSnd = "notes_of_the_optimistic.caf";
	mMsgSnd = "msg.caf";
	mExtensions = "pn-timeout=0;pn-silent=1";
	mVoipToken = "";
	mRemoteToken = "";
	mParam = "";
}

const string &PushNotificationConfig::getProvider() const {
	return mProvider;
}

void PushNotificationConfig::setProvider(const string &provider) {
	mProvider = provider;
}

const string &PushNotificationConfig::getTeamId() const {
	return mTeamId;
}

void PushNotificationConfig::setTeamId(const string &teamId) {
	mTeamId = teamId;
}

const string &PushNotificationConfig::getMsgStr() const {
	return mMsgStr;
}

void PushNotificationConfig::setMsgStr(const string &msgStr) {
	mMsgStr = msgStr;
}

const string &PushNotificationConfig::getCallStr() const {
	return mCallStr;
}

void PushNotificationConfig::setCallStr(const string &callStr) {
	mCallStr = callStr;
}

const string &PushNotificationConfig::getGroupChatStr() const {
	return mGroupChatStr;
}

void PushNotificationConfig::setGroupChatStr(const string &groupChatStr) {
	mGroupChatStr = groupChatStr;
}

const string &PushNotificationConfig::getPrid() const {
	return mPrid;
}

void PushNotificationConfig::setPrid(const string &prid) {
	mPrid = prid;
}

const string &PushNotificationConfig::getBundleIdentifer() const {
	return mBundleIdentifer;
}

void PushNotificationConfig::setBundleIdentifer(const string &bundleIdentifer) {
	mBundleIdentifer = bundleIdentifer;
}

const string &PushNotificationConfig::getServices() const {
	return mServices;
}

void PushNotificationConfig::setServices(const string &services) {
	mServices = services;
}

const string &PushNotificationConfig::getCallSnd() const {
	return mCallSnd;
}

void PushNotificationConfig::setCallSnd(const string &callSnd) {
	mCallSnd = callSnd;
}

const string &PushNotificationConfig::getMsgSnd() const {
	return mMsgSnd;
}

void PushNotificationConfig::setMsgSnd(const string &msgSnd) {
	mMsgSnd = msgSnd;
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

const string &PushNotificationConfig::getParam() const {
	return mParam;
}

void PushNotificationConfig::setParam(const string &param) {
	mParam = param;
}


LINPHONE_END_NAMESPACE
