/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#include "account-device.h"
#include "linphone/utils/utils.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

AccountDevice::AccountDevice(const string &uuid, const string &updateTime, const string &userAgent) {
	mUUID = uuid;
	mUserAgent = userAgent;
	mDeviceName = "";

	auto openParanthesis = mUserAgent.find("(");
	if (openParanthesis != mUserAgent.npos) {
		auto closeParanthesis = mUserAgent.find(")", openParanthesis);
		if (closeParanthesis != mUserAgent.npos) {
			mDeviceName = mUserAgent.substr(openParanthesis + 1, closeParanthesis - openParanthesis - 1);
		}
	}

	mUpdateTime = updateTime;
	mUpdateTimestamp = Utils::iso8601ToTime(mUpdateTime);
	if (mUpdateTimestamp == 0) {
		lError() << "[Account Device] Couldn't parse [" << mUpdateTime << "] as a time_t!";
	} else {
		lDebug() << "[Account Device] Parsed " << mUpdateTime << " = " << mUpdateTimestamp;
	}
}

AccountDevice::~AccountDevice() {
}

string AccountDevice::toString() const {
	std::ostringstream ss;
	ss << "[Account Device] UUID [" << mUUID << "], name [" << mDeviceName << "], last updated at [" << mUpdateTime
	   << "(" << mUpdateTimestamp << ")], user-agent [" << mUserAgent << "]";
	return ss.str();
}

LINPHONE_END_NAMESPACE
