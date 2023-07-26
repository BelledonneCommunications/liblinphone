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

#ifndef _L_FRIEND_DEVICE_H_
#define _L_FRIEND_DEVICE_H_

#include "c-wrapper/c-wrapper.h"
#include "linphone/api/c-types.h"
#include "linphone/enums/c-enums.h"
#include "linphone/enums/encryption-engine-enums.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class FriendDevice : public bellesip::HybridObject<LinphoneFriendDevice, FriendDevice> {
public:
	FriendDevice(const Address &address, const std::string &deviceName, LinphoneSecurityLevel securityLevel);
	FriendDevice(const FriendDevice &other) = default;
	~FriendDevice() = default;

	FriendDevice *clone() const override;

	const std::shared_ptr<const Address> &getAddress() const {
		return mAddress;
	}

	const std::string &getName() const {
		return mDeviceName;
	}

	LinphoneSecurityLevel getSecurityLevel() const {
		return mSecurityLevel;
	}

	void setSecurityLevel(LinphoneSecurityLevel securityLevel) {
		mSecurityLevel = securityLevel;
	}

private:
	std::shared_ptr<const Address> mAddress = nullptr;
	std::string mDeviceName = "";
	LinphoneSecurityLevel mSecurityLevel;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_FRIEND_DEVICE_H_
