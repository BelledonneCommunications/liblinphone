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
	const std::shared_ptr<ConferenceParams> params
	) : Conference(core, myAddress, listener, params) {
	// Set last notify to 1 in order to ensure that the 1st notify to remote conference is correctly processed
	// Remote conference sets last notify to 0 in its constructor
	lastNotify = 1;
#ifdef HAVE_ADVANCED_IM
	eventHandler = std::make_shared<LocalConferenceEventHandler>(this);
	addListener(eventHandler);
#endif
}

LocalConference::~LocalConference () {
#ifdef HAVE_ADVANCED_IM
	eventHandler.reset();
#endif
}

// -----------------------------------------------------------------------------

void LocalConference::subscribeReceived (LinphoneEvent *event) {
#ifdef HAVE_ADVANCED_IM
	shared_ptr<AbstractChatRoom> chatRoom = L_GET_CPP_PTR_FROM_C_OBJECT(linphone_event_get_core(event))->findChatRoom(conferenceId);
	eventHandler->subscribeReceived(event, !!(chatRoom->getCapabilities() & AbstractChatRoom::Capabilities::OneToOne));
#endif
}

void LocalConference::notifyFullState () {
	++lastNotify;
	Conference::notifyFullState();
}

shared_ptr<ConferenceParticipantEvent> LocalConference::notifyParticipantAdded (time_t creationTime,  const bool isFullState, const Address &addr) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantAdded (creationTime,  isFullState, addr);
}

shared_ptr<ConferenceParticipantEvent> LocalConference::notifyParticipantRemoved (time_t creationTime,  const bool isFullState, const Address &addr) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantRemoved (creationTime,  isFullState, addr);
}

shared_ptr<ConferenceParticipantEvent> LocalConference::notifyParticipantSetAdmin (time_t creationTime,  const bool isFullState, const Address &addr, bool isAdmin) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantSetAdmin (creationTime,  isFullState, addr, isAdmin);
}

shared_ptr<ConferenceSubjectEvent> LocalConference::notifySubjectChanged (time_t creationTime, const bool isFullState, const std::string subject) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifySubjectChanged (creationTime, isFullState, subject);
}

shared_ptr<ConferenceParticipantDeviceEvent> LocalConference::notifyParticipantDeviceAdded (time_t creationTime,  const bool isFullState, const Address &addr, const Address &gruu, const std::string name) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantDeviceAdded (creationTime,  isFullState, addr, gruu, name);
}

shared_ptr<ConferenceParticipantDeviceEvent> LocalConference::notifyParticipantDeviceRemoved (time_t creationTime,  const bool isFullState, const Address &addr, const Address &gruu) {
	// Increment last notify before notifying participants so that the delta can be calculated correctly
	++lastNotify;
	return Conference::notifyParticipantDeviceRemoved (creationTime,  isFullState, addr, gruu);
}

LINPHONE_END_NAMESPACE
