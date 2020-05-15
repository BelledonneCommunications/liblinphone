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

#ifndef _L_REMOTE_CONFERENCE_H_
#define _L_REMOTE_CONFERENCE_H_

#include "conference.h"
#include "core/core-accessor.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class RemoteConferenceEventHandler;

class LINPHONE_PUBLIC RemoteConference : public Conference {
	friend class ClientGroupChatRoomPrivate;

public:
	RemoteConference (const std::shared_ptr<Core> &core, const IdentityAddress &myAddress, CallSessionListener *listener);
	virtual ~RemoteConference ();

	/* ConferenceInterface */
	bool addParticipant (const IdentityAddress &addr, const CallSessionParams *params, bool hasMedia) override;
	bool removeParticipant (const std::shared_ptr<Participant> &participant) override;

protected:
	std::shared_ptr<Participant> focus;
	std::shared_ptr<RemoteConferenceEventHandler> eventHandler;


	/* ConferenceListener */
	void onConferenceCreated (const IdentityAddress &addr) override;
	void onConferenceTerminated (const IdentityAddress &addr) override;
	void onFirstNotifyReceived (const IdentityAddress &addr) override;
	void onParticipantAdded (const std::shared_ptr<ConferenceParticipantEvent> &event, bool isFullState) override;
	void onParticipantRemoved (const std::shared_ptr<ConferenceParticipantEvent> &event, bool isFullState) override;
	void onParticipantSetAdmin (const std::shared_ptr<ConferenceParticipantEvent> &event, bool isFullState) override;
	void onSubjectChanged (const std::shared_ptr<ConferenceSubjectEvent> &event, bool isFullState) override;
	void onParticipantDeviceAdded (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, bool isFullState) override;
	void onParticipantDeviceRemoved (const std::shared_ptr<ConferenceParticipantDeviceEvent> &event, bool isFullState) override;

private:
	L_DISABLE_COPY(RemoteConference);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_REMOTE_CONFERENCE_H_
