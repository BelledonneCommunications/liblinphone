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

#ifndef LINPHONE_ACCOUNT_DEVICE_H
#define LINPHONE_ACCOUNT_DEVICE_H

#include "linphone/api/c-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup account
 * @{
 *
 */
/**
 * Takes a reference on a #LinphoneAccountDevice.
 * @param device The #LinphoneAccountDevice object. @notnil
 * @return the same #LinphoneAccountDevice object. @notnil
 */
LINPHONE_PUBLIC LinphoneAccountDevice *linphone_account_device_ref(LinphoneAccountDevice *device);

/**
 * Releases a #LinphoneAccountDevice.
 * @param device The #LinphoneAccountDevice object. @notnil
 */
LINPHONE_PUBLIC void linphone_account_device_unref(LinphoneAccountDevice *device);

// -----------------------------------------------------------------------------

/**
 * Gets the UUID of the device.
 * @param device the #LinphoneAccountDevice to know the UUID of. @notnil
 * @return the UUID of the device. @notnil
 */
LINPHONE_PUBLIC const char *linphone_account_device_get_uuid(const LinphoneAccountDevice *device);

/**
 * Gets the user-agent of the device.
 * @param device the #LinphoneAccountDevice to know the user-agent of. @notnil
 * @return the user-agent of the device. @notnil
 */
LINPHONE_PUBLIC const char *linphone_account_device_get_user_agent(const LinphoneAccountDevice *device);

/**
 * Gets the name of the device.
 * @param device the #LinphoneAccountDevice to know the name of. @notnil
 * @return the name of the device. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_account_device_get_name(const LinphoneAccountDevice *device);

/**
 * Gets the timestamp at which this devices was updated for the last time.
 * @param device the #LinphoneAccountDevice to know the name of. @notnil
 * @return the timestamp (time_t) at which the device was updated. @maybenil
 */
LINPHONE_PUBLIC time_t linphone_account_device_get_last_update_timestamp(const LinphoneAccountDevice *device);

/**
 * Gets the timestamp at which this devices was updated for the last time.
 * @param device the #LinphoneAccountDevice to know the name of. @notnil
 * @return the time under ISO 8601 format at which the device was updated. @maybenil
 */
LINPHONE_PUBLIC const char *linphone_account_device_get_last_update_time(const LinphoneAccountDevice *device);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_ACCOUNT_DEVICE_H */
