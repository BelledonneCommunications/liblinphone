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

#include <memory>
#include <string>

#include "conference_private.h"
#include "private_functions.h"
#include "conference/notify-conference-listener.h"
#include "conference/participant-device.h"
#include "conference/participant.h"

LINPHONE_BEGIN_NAMESPACE

void NotifyConferenceListener::onParticipantAdded (const std::shared_ptr<ConferenceParticipantEvent> &event) {
	const std::shared_ptr<Participant> participant = event->getParticipant();
	_linphone_conference_notify_participant_added(conf->toC(), participant->toC());
}

void NotifyConferenceListener::onParticipantRemoved (const std::shared_ptr<ConferenceParticipantEvent> &event) {
	const std::shared_ptr<Participant> participant = event->getParticipant();
	_linphone_conference_notify_participant_removed(conf->toC(), participant->toC());

}

void NotifyConferenceListener::onParticipantSetAdmin (const std::shared_ptr<ConferenceParticipantEvent> &event) {
	const std::shared_ptr<Participant> participant = event->getParticipant();
	_linphone_conference_notify_participant_admin_status_changed(conf->toC(), participant->toC());
}

void NotifyConferenceListener::onSubjectChanged (const std::shared_ptr<ConferenceSubjectEvent> &event) {
	_linphone_conference_notify_subject_changed(conf->toC(), event->getSubject().c_str());
}

void NotifyConferenceListener::onParticipantDeviceAdded (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event) {
	const std::shared_ptr<ParticipantDevice> device = event->getDevice();
	_linphone_conference_notify_participant_device_added(conf->toC(), device->toC());
}

void NotifyConferenceListener::onParticipantDeviceRemoved (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event) {
	const std::shared_ptr<ParticipantDevice> device = event->getDevice();
	_linphone_conference_notify_participant_device_removed(conf->toC(), device->toC());
}

void NotifyConferenceListener::onStateChanged (ConferenceInterface::State newState) {
	_linphone_conference_notify_state_changed(conf->toC(), (LinphoneConferenceState)newState);
}

LINPHONE_END_NAMESPACE
