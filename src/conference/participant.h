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

#ifndef _L_PARTICIPANT_H_
#define _L_PARTICIPANT_H_

#include <list>

#include <belle-sip/object++.hh>

#include "address/identity-address.h"
#include "chat/chat-room/abstract-chat-room.h"
#include "conference/params/call-session-params.h"

#include "conference/participant-device.h"
#include "conference/session/call-session.h"
#include "conference/session/call-session-listener.h"
#include "conference/params/call-session-params.h"

// =============================================================================

namespace LinphoneTest {
	class LocalConferenceTester;
}

LINPHONE_BEGIN_NAMESPACE

class ClientGroupChatRoom;
class Conference;

class LINPHONE_PUBLIC Participant : public bellesip::HybridObject<LinphoneParticipant, Participant> {
	// TODO: Remove... It's ugly.
	friend class Call;
	friend class ClientGroupChatRoom;
	friend class ClientGroupChatRoomPrivate;
	friend class Conference;
	friend class LimeX3dhEncryptionEngine;
	friend class LocalConference;
	friend class LocalConferenceEventHandler;
	friend class LocalConferenceListEventHandler;
	friend class MainDb;
	friend class MainDbPrivate;
	friend class MediaSessionPrivate;
	friend class ParticipantDevice;
	friend class RemoteConference;
	friend class ServerGroupChatRoom;
	friend class ServerGroupChatRoomPrivate;

	friend class LinphoneTest::LocalConferenceTester;
public:
	explicit Participant (Conference *conference, const IdentityAddress &address);
	Participant ();
	void configure (Conference *conference, const IdentityAddress &address);
	virtual ~Participant();
	// non clonable object
	Participant *clone() const override { return nullptr; }
>>>>>>> Delete private classes of LocalConferenceEventHandler, RemoteConferenceEventHandler, Participant, ParticipantDevice, Conference, LocalConference, RemoteConference, RemoteConferenceEventHandler, LocalConferenceEventHandler

	const IdentityAddress &getAddress () const;
	AbstractChatRoom::SecurityLevel getSecurityLevel () const;
	bool isAdmin () const;

	const std::list<std::shared_ptr<ParticipantDevice>> &getDevices () const;
	std::shared_ptr<ParticipantDevice> findDevice (const IdentityAddress &gruu) const;
	std::shared_ptr<ParticipantDevice> findDevice (const std::shared_ptr<const CallSession> &session);

	inline void setAdmin (bool isAdmin) { this->isThisAdmin = isAdmin; }

protected:
	std::shared_ptr<Core> getCore () const { return mConference ? mConference->getCore() : nullptr; }
	Conference *getConference () const { return mConference; }
	void setConference (Conference *conference) { mConference = conference; }

	std::shared_ptr<CallSession> createSession (const Conference &conference, const CallSessionParams *params, bool hasMedia, CallSessionListener *listener);
	std::shared_ptr<CallSession> createSession (const std::shared_ptr<Core> &core, const CallSessionParams *params, bool hasMedia, CallSessionListener *listener);

	inline std::shared_ptr<CallSession> getSession () const { return session; }
	inline void removeSession () { session.reset(); }
	inline void setAddress (const IdentityAddress &addr) { this->addr = addr; }

	std::shared_ptr<ParticipantDevice> addDevice (const IdentityAddress &gruu, const std::string &name = "");
	void clearDevices ();
	void removeDevice (const IdentityAddress &gruu);

private:

	Conference *mConference = nullptr;
	IdentityAddress addr;
	bool isThisAdmin = false;
	std::shared_ptr<CallSession> session;
	std::list<std::shared_ptr<ParticipantDevice>> devices;

	L_DISABLE_COPY(Participant);
};

inline std::ostream &operator<< (std::ostream &os, const Participant &participant) {
	return os << participant.getAddress().asString();
	return os;
}

LINPHONE_END_NAMESPACE

#endif // ifndef _L_PARTICIPANT_H_
