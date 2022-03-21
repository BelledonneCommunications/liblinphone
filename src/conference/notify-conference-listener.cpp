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

void NotifyConferenceListener::onParticipantAdded (const std::shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) {
	_linphone_conference_notify_participant_added(conf->toC(), participant->toC());
}

void NotifyConferenceListener::onParticipantRemoved (const std::shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) {
	_linphone_conference_notify_participant_removed(conf->toC(), participant->toC());

}

void NotifyConferenceListener::onParticipantSetAdmin (const std::shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) {
	_linphone_conference_notify_participant_admin_status_changed(conf->toC(), participant->toC());
}

void NotifyConferenceListener::onAvailableMediaChanged (const std::shared_ptr<ConferenceAvailableMediaEvent> &event) {
	_linphone_conference_notify_available_media_changed(conf->toC());
}

void NotifyConferenceListener::onSubjectChanged (const std::shared_ptr<ConferenceSubjectEvent> &event) {
	_linphone_conference_notify_subject_changed(conf->toC(), event->getSubject().c_str());
}

void NotifyConferenceListener::onParticipantDeviceIsSpeakingChanged (const std::shared_ptr<ParticipantDevice> &device, bool isSpeaking) {
	_linphone_conference_notify_participant_device_is_speaking_changed(conf->toC(), device->toC(), isSpeaking);
}

void NotifyConferenceListener::onParticipantDeviceIsMuted (const std::shared_ptr<ParticipantDevice> &device, bool isMuted) {
	_linphone_conference_notify_participant_device_is_muted(conf->toC(), device->toC(), isMuted);
}

void NotifyConferenceListener::onParticipantDeviceJoined (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) {
	_linphone_conference_notify_participant_device_joined(conf->toC(), device->toC());
}

void NotifyConferenceListener::onParticipantDeviceLeft (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) {
	_linphone_conference_notify_participant_device_left(conf->toC(), device->toC());
}

void NotifyConferenceListener::onParticipantDeviceMediaAvailabilityChanged (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) {
	_linphone_conference_notify_participant_device_media_availability_changed(conf->toC(), device->toC());
}

void NotifyConferenceListener::onParticipantDeviceMediaCapabilityChanged (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) {
	_linphone_conference_notify_participant_device_media_capability_changed(conf->toC(), device->toC());
}

void NotifyConferenceListener::onParticipantDeviceAdded (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) {
	_linphone_conference_notify_participant_device_added(conf->toC(), device->toC());
}

void NotifyConferenceListener::onParticipantDeviceRemoved (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) {
	_linphone_conference_notify_participant_device_removed(conf->toC(), device->toC());
}

void NotifyConferenceListener::onStateChanged (ConferenceInterface::State newState) {
	_linphone_conference_notify_state_changed(conf->toC(), (LinphoneConferenceState)newState);
}

LINPHONE_END_NAMESPACE
