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

#include <bctoolbox/defs.h>

#include "account/account-device.h"
#include "c-wrapper/c-wrapper.h"
#include "core/core-p.h"

using namespace LinphonePrivate;

// =============================================================================

LinphoneAccountDevice *linphone_account_device_ref(LinphoneAccountDevice *device) {
	AccountDevice::toCpp(device)->ref();
	return device;
}

void linphone_account_device_unref(LinphoneAccountDevice *device) {
	AccountDevice::toCpp(device)->unref();
}

// =============================================================================

const char *linphone_account_device_get_uuid(const LinphoneAccountDevice *device) {
	return L_STRING_TO_C(AccountDevice::toCpp(device)->getUUID());
}

const char *linphone_account_device_get_user_agent(const LinphoneAccountDevice *device) {
	return L_STRING_TO_C(AccountDevice::toCpp(device)->getUserAgent());
}

const char *linphone_account_device_get_name(const LinphoneAccountDevice *device) {
	return L_STRING_TO_C(AccountDevice::toCpp(device)->getName());
}

time_t linphone_account_device_get_last_update_timestamp(const LinphoneAccountDevice *device) {
	return AccountDevice::toCpp(device)->getLastUpdateTimestamp();
}

const char *linphone_account_device_get_last_update_time(const LinphoneAccountDevice *device) {
	return L_STRING_TO_C(AccountDevice::toCpp(device)->getLastUpdateTime());
}

// =============================================================================