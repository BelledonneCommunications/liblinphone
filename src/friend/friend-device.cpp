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

#include "friend-device.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

FriendDevice::FriendDevice(const Address &address, const string &deviceName, LinphoneSecurityLevel securityLevel) {
	mAddress = (new Address(address))->toSharedPtr();
	mDeviceName = deviceName;
	mSecurityLevel = securityLevel;
}

// -----------------------------------------------------------------------------

FriendDevice *FriendDevice::clone() const {
	return new FriendDevice(*this);
}

LINPHONE_END_NAMESPACE