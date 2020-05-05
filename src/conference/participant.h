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

#include "address/identity-address.h"
#include "chat/chat-room/abstract-chat-room.h"
#include "conference/params/call-session-params.h"
#include "object/object.h"

#include "object/object-p.h"

#include "conference/participant-device.h"
#include "conference/session/call-session.h"
#include "conference/session/call-session-listener.h"
#include "conference/params/call-session-params.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ClientGroupChatRoom;
class Conference;

class Participant : public Object {
	// TODO: Remove... It's ugly.
	friend class Call;
	friend class CallPrivate;
	friend class ClientGroupChatRoom;
	friend class ClientGroupChatRoomPrivate;
	friend class Conference;
	friend class LimeX3dhEncryptionEngine;
	friend class LocalConference;
	friend class LocalConferenceCall;
	friend class LocalConferenceCallPrivate;
	friend class LocalConferenceEventHandler;
	friend class LocalConferenceListEventHandler;
	friend class MainDb;
	friend class MainDbPrivate;
	friend class MediaSessionPrivate;
	friend class ParticipantDevice;
	friend class RemoteConference;
	friend class RemoteConferenceCall;
	friend class RemoteConferenceCallPrivate;
	friend class ServerGroupChatRoom;
	friend class ServerGroupChatRoomPrivate;

public:
	L_OVERRIDE_SHARED_FROM_THIS(Participant);

	explicit Participant (Conference *conference, const IdentityAddress &address);
	virtual ~Participant();

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
	inline std::shared_ptr<CallSession> getSession () const { return session; }
	inline void removeSession () { session.reset(); }
	inline void setAddress (const IdentityAddress &newAddr) { addr = newAddr; }

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
