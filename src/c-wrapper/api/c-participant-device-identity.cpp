/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "c-wrapper/c-wrapper.h"
#include "conference/participant-device-identity.h"
#include "linphone/api/c-participant-device-identity.h"

// =============================================================================

using namespace std;
using namespace LinphonePrivate;

// =============================================================================

LinphoneParticipantDeviceIdentity *linphone_participant_device_identity_new(const LinphoneAddress *address,
                                                                            const char *name) {
#ifdef HAVE_ADVANCED_IM
	return ParticipantDeviceIdentity::createCObject(Address::toCpp(address)->getSharedFromThis(), L_C_TO_STRING(name));
#else
	return NULL;
#endif
}

LinphoneParticipantDeviceIdentity *
linphone_participant_device_identity_ref(LinphoneParticipantDeviceIdentity *deviceIdentity) {
#ifdef HAVE_ADVANCED_IM
	belle_sip_object_ref(deviceIdentity);
	return deviceIdentity;
#else
	return NULL;
#endif
}

void linphone_participant_device_identity_unref(LinphoneParticipantDeviceIdentity *deviceIdentity) {
#ifdef HAVE_ADVANCED_IM
	belle_sip_object_unref(deviceIdentity);
#endif
}

void linphone_participant_device_identity_set_capability_descriptor(LinphoneParticipantDeviceIdentity *deviceIdentity,
                                                                    const char *descriptor) {
#ifdef HAVE_ADVANCED_IM
	ParticipantDeviceIdentity::toCpp(deviceIdentity)->setCapabilityDescriptor(L_C_TO_STRING(descriptor));
#endif
}
void linphone_participant_device_identity_set_capability_descriptor_2(LinphoneParticipantDeviceIdentity *deviceIdentity,
                                                                      const bctbx_list_t *descriptor_list) {
#ifdef HAVE_ADVANCED_IM
	ParticipantDeviceIdentity::toCpp(deviceIdentity)
	    ->setCapabilityDescriptor(L_GET_CPP_LIST_FROM_C_LIST(descriptor_list, const char *, string));
#endif
}
const char *linphone_participant_device_identity_get_capability_descriptor(
    const LinphoneParticipantDeviceIdentity *deviceIdentity) {
#ifdef HAVE_ADVANCED_IM
	return L_STRING_TO_C(ParticipantDeviceIdentity::toCpp(deviceIdentity)->getCapabilityDescriptor());
#endif
	return NULL;
}
const bctbx_list_t *linphone_participant_device_identity_get_capability_descriptor_list(
    const LinphoneParticipantDeviceIdentity *deviceIdentity) {
#ifdef HAVE_ADVANCED_IM
	return L_GET_C_LIST_FROM_CPP_LIST(ParticipantDeviceIdentity::toCpp(deviceIdentity)->getCapabilityDescriptorList());
#endif
	return NULL;
}

const LinphoneAddress *
linphone_participant_device_identity_get_address(const LinphoneParticipantDeviceIdentity *deviceIdentity) {
#ifdef HAVE_ADVANCED_IM
	return ParticipantDeviceIdentity::toCpp(deviceIdentity)->getAddress()->toC();
#endif
	return NULL;
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER
