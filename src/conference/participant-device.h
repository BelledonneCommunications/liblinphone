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

#ifndef _L_PARTICIPANT_DEVICE_H_
#define _L_PARTICIPANT_DEVICE_H_

#include <string>

#include "address/identity-address.h"
#include "chat/chat-room/abstract-chat-room.h"
#include "chat/encryption/encryption-engine.h"

#include "linphone/types.h"
#include "linphone/utils/general.h"
#include "object/object.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class CallSession;
class Core;
class Participant;
class ParticipantDevicePrivate;

class ParticipantDevice : public Object {
public:
	enum class State {
		Joining, //an INVITE has been sent
		Present, //the SIP session has been concluded, participant is part of the conference
		Leaving, //A BYE is pending
		Left, //The Session is terminated
		ScheduledForJoining, //Initial state for the server group chatroom, when the participant has not yet been INVITEd.
		ScheduledForLeaving, //Transitional state for a participant that will receive a BYE shortly.
	};

	ParticipantDevice ();
	explicit ParticipantDevice (Participant *participant, const IdentityAddress &gruu, const std::string &name = "");
	virtual ~ParticipantDevice ();

	bool operator== (const ParticipantDevice &device) const;

	std::shared_ptr<Core> getCore () const;

	inline const IdentityAddress &getAddress () const { return mGruu; }
	inline const std::string &getName () const { return mName; }
	inline void setName (const std::string &name) { mName = name; }
	Participant *getParticipant () const { return mParticipant; }
	inline std::shared_ptr<CallSession> getSession () const { return mSession; }
	inline void setSession (std::shared_ptr<CallSession> session) { mSession = session; }
	inline State getState () const { return mState; }
	inline void setState (State newState) { mState = newState; }
	AbstractChatRoom::SecurityLevel getSecurityLevel () const;

	inline bool isSubscribedToConferenceEventPackage () const { return mConferenceSubscribeEvent != nullptr; }
	LinphoneEvent *getConferenceSubscribeEvent () const { return mConferenceSubscribeEvent; }
	void setConferenceSubscribeEvent (LinphoneEvent *ev);

	bool isValid () const { return mGruu.isValid(); }

private:
	L_OVERRIDE_SHARED_FROM_THIS(ParticipantDevice);

	Participant *mParticipant = nullptr;
	IdentityAddress mGruu;
	std::string mName;
	std::shared_ptr<CallSession> mSession;
	LinphoneEvent *mConferenceSubscribeEvent = nullptr;
	State mState = State::Joining;

	L_DECLARE_PRIVATE(ParticipantDevice);
	L_DISABLE_COPY(ParticipantDevice);
};

std::ostream &operator<< (std::ostream &stream, ParticipantDevice::State state);

LINPHONE_END_NAMESPACE

#endif // ifndef _L_PARTICIPANT_DEVICE_H_
