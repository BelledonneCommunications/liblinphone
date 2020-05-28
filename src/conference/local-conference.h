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

#ifndef _L_LOCAL_CONFERENCE_H_
#define _L_LOCAL_CONFERENCE_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "conference.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LocalConferenceEventHandler;

class LINPHONE_PUBLIC LocalConference : 
	public Conference,
	public ConferenceListenerInterface {
	friend class ServerGroupChatRoomPrivate;
	friend class LocalConferenceEventHandler;
public:
	LocalConference (const std::shared_ptr<Core> &core, const IdentityAddress &myAddress, CallSessionListener *listener);
	virtual ~LocalConference ();

	/* ConferenceInterface */
	bool addParticipant (const IdentityAddress &addr, const CallSessionParams *params, bool hasMedia) override;
	bool removeParticipant (const std::shared_ptr<Participant> &participant) override;

	std::shared_ptr<ConferenceParticipantEvent> notifyParticipantAdded (const Address &addr);
	std::shared_ptr<ConferenceParticipantEvent> notifyParticipantRemoved (const Address &addr);
	std::shared_ptr<ConferenceParticipantEvent> notifyParticipantSetAdmin (const Address &addr, bool isAdmin);
	std::shared_ptr<ConferenceSubjectEvent> notifySubjectChanged ();
	std::shared_ptr<ConferenceParticipantDeviceEvent> notifyParticipantDeviceAdded (const Address &addr, const Address &gruu);
	std::shared_ptr<ConferenceParticipantDeviceEvent> notifyParticipantDeviceRemoved (const Address &addr, const Address &gruu);

//	void notifyFullState (const std::string &notify, const std::shared_ptr<ParticipantDevice> &device);

	void subscribeReceived (LinphoneEvent *event);

protected:
#ifdef HAVE_ADVANCED_IM
	std::shared_ptr<LocalConferenceEventHandler> eventHandler;
#endif

private:

	L_DISABLE_COPY(LocalConference);

};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_LOCAL_CONFERENCE_H_
