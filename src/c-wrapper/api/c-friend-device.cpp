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

#include <ctype.h>

#include "c-wrapper/c-wrapper.h"
#include "friend/friend-device.h"
#include "linphone/api/c-friend-device.h"
#include "linphone/wrapper_utils.h"

using namespace LinphonePrivate;

// =============================================================================

LinphoneFriendDevice *linphone_friend_device_clone(const LinphoneFriendDevice *device) {
	return FriendDevice::toCpp(device)->clone()->toC();
}

LinphoneFriendDevice *linphone_friend_device_ref(LinphoneFriendDevice *device) {
	FriendDevice::toCpp(device)->ref();
	return device;
}

void linphone_friend_device_unref(LinphoneFriendDevice *device) {
	FriendDevice::toCpp(device)->unref();
}

// =============================================================================

const LinphoneAddress *linphone_friend_device_get_address(const LinphoneFriendDevice *device) {
	return FriendDevice::toCpp(device)->getAddress()->toC();
}

const char *linphone_friend_device_get_display_name(const LinphoneFriendDevice *device) {
	return L_STRING_TO_C(FriendDevice::toCpp(device)->getName());
}

LinphoneSecurityLevel linphone_friend_device_get_security_level(const LinphoneFriendDevice *device) {
	return FriendDevice::toCpp(device)->getSecurityLevel();
}

// =============================================================================