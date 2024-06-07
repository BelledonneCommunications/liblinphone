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

#ifndef _L_ACCOUNT_DEVICE_H_
#define _L_ACCOUNT_DEVICE_H_

#include "belle-sip/object++.hh"
#include "c-wrapper/c-wrapper.h"
#include "linphone/api/c-types.h"
#include "linphone/types.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class AccountDevice : public bellesip::HybridObject<LinphoneAccountDevice, AccountDevice> {

public:
	AccountDevice(const std::string &uuid, const std::string &updateTime, const std::string &userAgent);
	AccountDevice(const AccountDevice &other) = delete;
	virtual ~AccountDevice();

	const std::string &getUUID() const {
		return mUUID;
	}
	const std::string &getUserAgent() const {
		return mUserAgent;
	}
	const std::string &getName() const {
		return mDeviceName;
	}
	time_t getLastUpdateTimestamp() const {
		return mUpdateTimestamp;
	}
	const std::string &getLastUpdateTime() const {
		return mUpdateTime;
	}

	std::string toString() const override;

	std::ostream &operator<<(std::ostream &str) {
		str << this->toString();
		return str;
	}

private:
	std::string mUUID;
	std::string mUpdateTime;
	std::string mUserAgent;
	std::string mDeviceName;
	time_t mUpdateTimestamp;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_ACCOUNT_DEVICE_H_
