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

#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "linphone/event.h"

#ifdef HAVE_ADVANCED_IM
#include "handlers/local-conference-event-handler.h"
#endif
#include "local-conference.h"
#include "logger/logger.h"
#include "participant.h"

// TODO: Remove me later.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

LocalConference::LocalConference (
	const shared_ptr<Core> &core,
	const IdentityAddress &myAddress,
	CallSessionListener *listener,
	const std::shared_ptr<ConferenceParams> params ,
	ConferenceListener *confListener
	) : Conference(core, myAddress, listener, params) {
	// Set last notify to 1 in order to ensure that the 1st notify to remote conference is correctly processed
	// Remote conference sets last notify to 0 in its constructor
	lastNotify = 1;

	this->confParams->enableLocalParticipant(false);

#ifdef HAVE_ADVANCED_IM
	eventHandler = std::make_shared<LocalConferenceEventHandler>(this, confListener);
	addListener(eventHandler);
#endif
}

LocalConference::~LocalConference () {
#ifdef HAVE_ADVANCED_IM
	eventHandler.reset();
#endif
}

bool LocalConference::isIn() const{
	return true;
}

// -----------------------------------------------------------------------------

void LocalConference::subscribeReceived (LinphoneEvent *event) {
#ifdef HAVE_ADVANCED_IM
	if (event) {
		const LinphoneAddress *lAddr = linphone_event_get_from(event);
		char *addrStr = linphone_address_as_string(lAddr);
		Address participantAddress(addrStr);
		bctbx_free(addrStr);

		shared_ptr<Participant> participant = findParticipant (participantAddress);

		if (participant) {
			const LinphoneAddress *lContactAddr = linphone_event_get_remote_contact(event);
			char *contactAddrStr = linphone_address_as_string(lContactAddr);
			IdentityAddress contactAddr(contactAddrStr);
			bctbx_free(contactAddrStr);
			shared_ptr<ParticipantDevice> device = participant->findDevice(contactAddr);

			if (device) {
				vector<string> acceptedContents = vector<string>();
				const auto message = (belle_sip_message_t*)event->op->getRecvCustomHeaders();
				for (belle_sip_header_t *acceptHeader=belle_sip_message_get_header(message,"Accept"); acceptHeader != NULL; acceptHeader = belle_sip_header_get_next(acceptHeader)) {
					acceptedContents.push_back(L_C_TO_STRING(belle_sip_header_get_unparsed_value(acceptHeader)));
				}
				const auto protocols = Utils::parseCapabilityDescriptor(device->getCapabilityDescriptor());
				const auto ephemeral = protocols.find("ephemeral");
				if (ephemeral != protocols.end()) {
					const auto ephemeralVersion = ephemeral->second;
					device->enableAdminModeSupport((ephemeralVersion > Utils::Version(1,1)));
				} else {
					device->enableAdminModeSupport(false);
				}
			}
		}
	}

	eventHandler->subscribeReceived(event);
#endif
}

void LocalConference::notifyFullState () {
	++lastNotify;
	Conference::notifyFullState();
}

shared_ptr<ConferenceParticipantEvent> LocalConference::notifyParticipantAdded (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantAdded (creationTime,  isFullState, participant);
}

shared_ptr<ConferenceParticipantEvent> LocalConference::notifyParticipantRemoved (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantRemoved (creationTime,  isFullState, participant);
}

shared_ptr<ConferenceParticipantEvent> LocalConference::notifyParticipantSetAdmin (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, bool isAdmin) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantSetAdmin (creationTime,  isFullState, participant, isAdmin);
}

shared_ptr<ConferenceSubjectEvent> LocalConference::notifySubjectChanged (time_t creationTime, const bool isFullState, const std::string subject) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifySubjectChanged (creationTime, isFullState, subject);
}

shared_ptr<ConferenceEphemeralMessageEvent> LocalConference::notifyEphemeralModeChanged (time_t creationTime, const bool isFullState, const EventLog::Type type) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyEphemeralModeChanged (creationTime, isFullState, type);
}

shared_ptr<ConferenceEphemeralMessageEvent> LocalConference::notifyEphemeralMessageEnabled (time_t creationTime, const bool isFullState, const bool enable) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyEphemeralMessageEnabled (creationTime, isFullState, enable);
}

shared_ptr<ConferenceEphemeralMessageEvent> LocalConference::notifyEphemeralLifetimeChanged (time_t creationTime, const bool isFullState, const long lifetime) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyEphemeralLifetimeChanged (creationTime, isFullState, lifetime);
}

shared_ptr<ConferenceParticipantDeviceEvent> LocalConference::notifyParticipantDeviceAdded (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantDeviceAdded (creationTime,  isFullState, participant, participantDevice);
}

shared_ptr<ConferenceParticipantDeviceEvent> LocalConference::notifyParticipantDeviceRemoved (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantDeviceRemoved (creationTime,  isFullState, participant, participantDevice);
}

LINPHONE_END_NAMESPACE
