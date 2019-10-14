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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_ADVANCED_IM
#include "chat/chat-room/server-group-chat-room-p.h"
#endif
#include "c-wrapper/c-wrapper.h"

// =============================================================================

#ifdef HAVE_ADVANCED_IM
L_DECLARE_C_CLONABLE_OBJECT_IMPL(ParticipantDeviceIdentity);
#endif

using namespace std;

// =============================================================================

LinphoneParticipantDeviceIdentity *linphone_participant_device_identity_new (const LinphoneAddress *address, const char *name) {
#ifdef HAVE_ADVANCED_IM
	LinphonePrivate::ParticipantDeviceIdentity *cppPtr = new LinphonePrivate::ParticipantDeviceIdentity(
		*L_GET_CPP_PTR_FROM_C_OBJECT(address),
		L_C_TO_STRING(name)
	);
	LinphoneParticipantDeviceIdentity *object = L_INIT(ParticipantDeviceIdentity);
	L_SET_CPP_PTR_FROM_C_OBJECT(object, cppPtr);

	return object;
#else
	return NULL;
#endif
}

LinphoneParticipantDeviceIdentity *linphone_participant_device_identity_clone (const LinphoneParticipantDeviceIdentity *deviceIdentity) {
#ifdef HAVE_ADVANCED_IM
	return reinterpret_cast<LinphoneParticipantDeviceIdentity *>(belle_sip_object_clone(BELLE_SIP_OBJECT(deviceIdentity)));
#else
	return NULL;
#endif
}

LinphoneParticipantDeviceIdentity *linphone_participant_device_identity_ref (LinphoneParticipantDeviceIdentity *deviceIdentity) {
#ifdef HAVE_ADVANCED_IM
	belle_sip_object_ref(deviceIdentity);
	return deviceIdentity;
#else
	return NULL;
#endif
}

void linphone_participant_device_identity_unref (LinphoneParticipantDeviceIdentity *deviceIdentity) {
#ifdef HAVE_ADVANCED_IM
	belle_sip_object_unref(deviceIdentity);
#endif
}
