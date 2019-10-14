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

#ifndef _L_LOCAL_CONFERENCE_EVENT_HANDLER_H_
#define _L_LOCAL_CONFERENCE_EVENT_HANDLER_H_

#include "linphone/types.h"

#include "address/address.h"
#include "core/core-accessor.h"
#include "object/object.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ConferenceId;
class ConferenceParticipantDeviceEvent;
class ConferenceParticipantEvent;
class ConferenceSubjectEvent;
class LocalConference;
class LocalConferenceEventHandlerPrivate;

class LocalConferenceEventHandler : public Object {
friend class LocalConferenceListEventHandler;
public:
	LocalConferenceEventHandler (LocalConference *localConference, unsigned int notify = 0);

	void subscribeReceived (LinphoneEvent *lev, bool oneToOne = false);
	void subscriptionStateChanged (LinphoneEvent *lev, LinphoneSubscriptionState state);

	std::shared_ptr<ConferenceParticipantEvent> notifyParticipantAdded (const Address &addr);
	std::shared_ptr<ConferenceParticipantEvent> notifyParticipantRemoved (const Address &addr);
	std::shared_ptr<ConferenceParticipantEvent> notifyParticipantSetAdmin (const Address &addr, bool isAdmin);
	std::shared_ptr<ConferenceSubjectEvent> notifySubjectChanged ();
	std::shared_ptr<ConferenceParticipantDeviceEvent> notifyParticipantDeviceAdded (const Address &addr, const Address &gruu);
	std::shared_ptr<ConferenceParticipantDeviceEvent> notifyParticipantDeviceRemoved (const Address &addr, const Address &gruu);

	void setLastNotify (unsigned int lastNotify);
	void setConferenceId (const ConferenceId &conferenceId);
	const ConferenceId &getConferenceId () const;

	std::string getNotifyForId (int notifyId, bool oneToOne = false);

private:
	L_DECLARE_PRIVATE(LocalConferenceEventHandler);
	L_DISABLE_COPY(LocalConferenceEventHandler);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_LOCAL_CONFERENCE_EVENT_HANDLER_H_
