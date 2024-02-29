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

#ifndef LINPHONE_FRIEND_DEVICE_H
#define LINPHONE_FRIEND_DEVICE_H

#include "linphone/api/c-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup buddy_list
 * @{
 */

/**
 * Clones a device.
 * @param device The #LinphoneFriendDevice object to be cloned. @notnil
 * @return The newly created #LinphoneFriendDevice object. @notnil
 */
LINPHONE_PUBLIC LinphoneFriendDevice *linphone_friend_device_clone(const LinphoneFriendDevice *device);

/**
 * Takes a reference on a #LinphoneFriendDevice.
 * @param device The #LinphoneFriendDevice object. @notnil
 * @return the same #LinphoneFriendDevice object. @notnil
 */
LINPHONE_PUBLIC LinphoneFriendDevice *linphone_friend_device_ref(LinphoneFriendDevice *device);

/**
 * Releases a #LinphoneFriendDevice.
 * @param device The #LinphoneFriendDevice object. @notnil
 */
LINPHONE_PUBLIC void linphone_friend_device_unref(LinphoneFriendDevice *device);

// =============================================================================

/**
 * Gets the address associated to this device.
 * @param device The #LinphoneFriendDevice object. @notnil
 * @return the address (including gruu) to which this device is linked. @notnil
 */
LINPHONE_PUBLIC const LinphoneAddress *linphone_friend_device_get_address(const LinphoneFriendDevice *device);

/**
 * Gets the display name of this device.
 * @param device The #LinphoneFriendDevice object. @notnil
 * @return the name of the device. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_friend_device_get_display_name(const LinphoneFriendDevice *device);

/**
 * Gets the current security level of this device.
 * @param device The #LinphoneFriendDevice object. @notnil
 * @return the current #LinphoneSecurityLevel of the device.
 */
LINPHONE_PUBLIC LinphoneSecurityLevel linphone_friend_device_get_security_level(const LinphoneFriendDevice *device);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_FRIEND_DEVICE_H */
