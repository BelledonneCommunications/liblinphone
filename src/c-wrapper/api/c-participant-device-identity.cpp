/*
 * c-address.cpp
 * Copyright (C) 2010-2018 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "chat/chat-room/server-group-chat-room-p.h"
#include "c-wrapper/c-wrapper.h"

// =============================================================================

L_DECLARE_C_CLONABLE_OBJECT_IMPL(ParticipantDeviceIdentity);

using namespace std;

// =============================================================================

LinphoneParticipantDeviceIdentity *linphone_participant_device_identity_new (const LinphoneAddress *address, const char *name) {
	LinphonePrivate::ParticipantDeviceIdentity *cppPtr = new LinphonePrivate::ParticipantDeviceIdentity(
		*L_GET_CPP_PTR_FROM_C_OBJECT(address),
		L_C_TO_STRING(name)
	);
	LinphoneParticipantDeviceIdentity *object = L_INIT(ParticipantDeviceIdentity);
	L_SET_CPP_PTR_FROM_C_OBJECT(object, cppPtr);

	return object;
}

LinphoneParticipantDeviceIdentity *linphone_participant_device_identity_clone (const LinphoneParticipantDeviceIdentity *deviceIdentity) {
	return reinterpret_cast<LinphoneParticipantDeviceIdentity *>(belle_sip_object_clone(BELLE_SIP_OBJECT(deviceIdentity)));
}

LinphoneParticipantDeviceIdentity *linphone_participant_device_identity_ref (LinphoneParticipantDeviceIdentity *deviceIdentity) {
	belle_sip_object_ref(deviceIdentity);
	return deviceIdentity;
}

void linphone_participant_device_identity_unref (LinphoneParticipantDeviceIdentity *deviceIdentity) {
	belle_sip_object_unref(deviceIdentity);
}
