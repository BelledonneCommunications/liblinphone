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


using namespace std;
using namespace LinphonePrivate;

// =============================================================================

LinphoneParticipantDeviceIdentity *linphone_participant_device_identity_new (const LinphoneAddress *address, const char *name) {
#ifdef HAVE_ADVANCED_IM
	return ParticipantDeviceIdentity::createCObject(*L_GET_CPP_PTR_FROM_C_OBJECT(address), name);
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


void linphone_participant_device_identity_set_capability_descriptor(LinphoneParticipantDeviceIdentity *deviceIdentity, const char *descriptor){
#ifdef HAVE_ADVANCED_IM
	ParticipantDeviceIdentity::toCpp(deviceIdentity)->setCapabilityDescriptor(descriptor);
#endif
}

const char* linphone_participant_device_identity_get_capability_descriptor(const LinphoneParticipantDeviceIdentity *deviceIdentity){
#ifdef HAVE_ADVANCED_IM
	return ParticipantDeviceIdentity::toCpp(deviceIdentity)->getCapabilityDescriptor().c_str();
#endif
	return NULL;
}

const LinphoneAddress* linphone_participant_device_identity_get_address(const LinphoneParticipantDeviceIdentity *deviceIdentity){
#ifdef HAVE_ADVANCED_IM
	return ParticipantDeviceIdentity::toCpp(deviceIdentity)->getLinphoneAddress();
#endif
	return NULL;
}

