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

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ClientGroupChatRoom;
class Conference;
class ParticipantPrivate;

class Participant : public Object {
	// TODO: Remove... It's ugly.
	friend class Call;
	friend class ClientGroupChatRoom;
	friend class ClientGroupChatRoomPrivate;
	friend class Conference;
	friend class LimeX3dhEncryptionEngine;
	friend class LocalConference;
	friend class LocalConferenceEventHandler;
	friend class LocalConferenceEventHandlerPrivate;
	friend class LocalConferenceListEventHandler;
	friend class MainDb;
	friend class MainDbPrivate;
	friend class MediaSessionPrivate;
	friend class ParticipantDevice;
	friend class RemoteConference;
	friend class ServerGroupChatRoom;
	friend class ServerGroupChatRoomPrivate;

public:
	L_OVERRIDE_SHARED_FROM_THIS(Participant);

	explicit Participant (Conference *conference, const IdentityAddress &address);

	const IdentityAddress &getAddress () const;
	AbstractChatRoom::SecurityLevel getSecurityLevel () const;
	bool isAdmin () const;

private:
	L_DECLARE_PRIVATE(Participant);
	L_DISABLE_COPY(Participant);
};

inline std::ostream &operator<< (std::ostream &os, const Participant &participant) {
	return os << participant.getAddress().asString();
	return os;
}

LINPHONE_END_NAMESPACE

#endif // ifndef _L_PARTICIPANT_H_
