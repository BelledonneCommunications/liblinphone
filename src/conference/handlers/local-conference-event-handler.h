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

#include <string>

#include "linphone/types.h"

#include "address/address.h"
#include "core/core-accessor.h"

#include "conference/conference-id.h"
#include "xml/conference-info.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ConferenceId;
class ConferenceParticipantDeviceEvent;
class ConferenceParticipantEvent;
class ConferenceSubjectEvent;
class LocalConference;
class Participant;
class ParticipantDevice;

class LINPHONE_PUBLIC LocalConferenceEventHandler {
friend class LocalConferenceListEventHandler;
#ifdef LINPHONE_TESTER
	friend class Tester;
#endif
public:
	LocalConferenceEventHandler (LocalConference *localConference, unsigned int notify = 0);

	void subscribeReceived (LinphoneEvent *lev, bool oneToOne = false);
	void subscriptionStateChanged (LinphoneEvent *lev, LinphoneSubscriptionState state);

	void setLastNotify (unsigned int lastNotify);
	void setConferenceId (const ConferenceId &conferenceId);
	const ConferenceId &getConferenceId () const;

	std::string getNotifyForId (int notifyId, bool oneToOne = false);

//protected:
	void notifyFullState (const std::string &notify, const std::shared_ptr<ParticipantDevice> &device);
	void notifyAllExcept (const std::string &notify, const std::shared_ptr<Participant> &exceptParticipant);
	void notifyAll (const std::string &notify);
	std::string createNotifyFullState (int notifyId = -1, bool oneToOne = false);
	std::string createNotifyMultipart (int notifyId);
	std::string createNotifyParticipantAdded (const Address &addr, int notifyId = -1);
	std::string createNotifyParticipantAdminStatusChanged (const Address &addr, bool isAdmin, int notifyId = -1);
	std::string createNotifyParticipantRemoved (const Address &addr, int notifyId = -1);
	std::string createNotifyParticipantDeviceAdded (const Address &addr, const Address &gruu, int notifyId = -1);
	std::string createNotifyParticipantDeviceRemoved (const Address &addr, const Address &gruu, int notifyId = -1);
	std::string createNotifySubjectChanged (int notifyId = -1);

	inline unsigned int getLastNotify () const { return lastNotify; };

	static void notifyResponseCb (const LinphoneEvent *ev);

private:
	ConferenceId conferenceId;

	LocalConference *conf = nullptr;
	unsigned int lastNotify = 1;

	std::string createNotify (Xsd::ConferenceInfo::ConferenceType confInfo, int notifyId = -1, bool isFullState = false);
	std::string createNotifySubjectChanged (const std::string &subject, int notifyId = -1);
	void notifyParticipant (const std::string &notify, const std::shared_ptr<Participant> &participant);
	void notifyParticipantDevice (const std::string &notify, const std::shared_ptr<ParticipantDevice> &device, bool multipart = false);

	L_DISABLE_COPY(LocalConferenceEventHandler);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_LOCAL_CONFERENCE_EVENT_HANDLER_H_
