/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
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

#ifndef _L_C_PARTICIPANT_DEVICE_IDENTiTY_H_
#define _L_C_PARTICIPANT_DEVICE_IDENTiTY_H_

#include "linphone/api/c-types.h"

// =============================================================================

#ifdef __cplusplus
	extern "C" {
#endif // ifdef __cplusplus

/**
 * @addtogroup misc
 * @{
 */

/**
 * Constructs a #LinphoneParticipantDeviceIdentity object.
 **/
LINPHONE_PUBLIC LinphoneParticipantDeviceIdentity *linphone_participant_device_identity_new (const LinphoneAddress *address, const char *name);

/**
 * Clones a #LinphoneParticipantDeviceIdentity object.
 **/
LINPHONE_PUBLIC LinphoneParticipantDeviceIdentity *linphone_participant_device_identity_clone (const LinphoneParticipantDeviceIdentity *deviceIdentity);

/**
 * Increment reference count of #LinphoneParticipantDeviceIdentity object.
 **/
LINPHONE_PUBLIC LinphoneParticipantDeviceIdentity *linphone_participant_device_identity_ref (LinphoneParticipantDeviceIdentity *deviceIdentity);

/**
 * Decrement reference count of #LinphoneParticipantDeviceIdentity object. When dropped to zero, memory is freed.
 **/
LINPHONE_PUBLIC void linphone_participant_device_identity_unref (LinphoneParticipantDeviceIdentity *deviceIdentity);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_PARTICIPANT_DEVICE_IDENTiTY_H_
