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
 * @param address a #LinphoneAddress of the participant device @notnil
 * @param name the name of the participant device @maybenil
 * @return a new #LinphoneParticipantDeviceIdentity @maybenil
 **/
LINPHONE_PUBLIC LinphoneParticipantDeviceIdentity *linphone_participant_device_identity_new (const LinphoneAddress *address, const char *name);

/**
 * Increment reference count of #LinphoneParticipantDeviceIdentity object.
 * @param device_identity the #LinphoneParticipantDeviceIdentity object @notnil
 * @return the same #LinphoneParticipantDeviceIdentity object @notnil
 **/
LINPHONE_PUBLIC LinphoneParticipantDeviceIdentity *linphone_participant_device_identity_ref (LinphoneParticipantDeviceIdentity *device_identity);

/**
 * Decrement reference count of #LinphoneParticipantDeviceIdentity object. When dropped to zero, memory is freed.
 * @param device_identity the #LinphoneParticipantDeviceIdentity object @notnil
 **/
LINPHONE_PUBLIC void linphone_participant_device_identity_unref (LinphoneParticipantDeviceIdentity *device_identity);


/**
 * Set the capability descriptor (currently +org.linphone.specs value) for this participant device identity.
 * @param device_identity the #LinphoneParticipantDeviceIdentity object @notnil
 * @param capability_descriptor the capability descriptor string.
 *
 **/
LINPHONE_PUBLIC void linphone_participant_device_identity_set_capability_descriptor(LinphoneParticipantDeviceIdentity *device_identity, const char *capability_descriptor);

/**
 * Get the capability descriptor (currently +org.linphone.specs value) for this participant device identity.
 * @param device_identity the #LinphoneParticipantDeviceIdentity object @notnil
 * @return the capability descriptor string.
 *
 **/
LINPHONE_PUBLIC const char* linphone_participant_device_identity_get_capability_descriptor(const LinphoneParticipantDeviceIdentity *device_identity);

/**
 * Get the address of the participant device.
 * @param device_identity the #LinphoneParticipantDeviceIdentity @notnil
 * @return the address. @notnil
 */
LINPHONE_PUBLIC const LinphoneAddress* linphone_participant_device_identity_get_address(const LinphoneParticipantDeviceIdentity *deviceIdentity);

/**
 * @}
 */

#ifdef __cplusplus
	}
#endif // ifdef __cplusplus

#endif // ifndef _L_C_PARTICIPANT_DEVICE_IDENTiTY_H_
