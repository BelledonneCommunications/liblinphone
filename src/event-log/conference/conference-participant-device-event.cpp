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

#include "conference-participant-device-event.h"
#include "conference-participant-event-p.h"
#include "conference/participant-device.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

class ConferenceParticipantDeviceEventPrivate : public ConferenceParticipantEventPrivate {
public:
	std::shared_ptr<ParticipantDevice>  device;
	string deviceName;
};

// -----------------------------------------------------------------------------

ConferenceParticipantDeviceEvent::ConferenceParticipantDeviceEvent (
	Type type,
	time_t creationTime,
	const ConferenceId &conferenceId,
	const std::shared_ptr<Participant> &participant,
	const std::shared_ptr<ParticipantDevice> &device,
	const string &name
) : ConferenceParticipantEvent(
	*new ConferenceParticipantDeviceEventPrivate,
	type,
	creationTime,
	conferenceId,
	participant
) {
	L_D();
	L_ASSERT(
		type == Type::ConferenceParticipantDeviceAdded ||
		type == Type::ConferenceParticipantDeviceRemoved
	);
	d->device = device;
	d->deviceName = name;
}

const std::shared_ptr<ParticipantDevice> &ConferenceParticipantDeviceEvent::getDevice () const {
	L_D();
	return d->device;
}

const IdentityAddress &ConferenceParticipantDeviceEvent::getDeviceAddress () const {
	L_D();
	return d->device->getAddress();
}

const string &ConferenceParticipantDeviceEvent::getDeviceName () const {
	L_D();
	return d->deviceName;
}

LINPHONE_END_NAMESPACE
