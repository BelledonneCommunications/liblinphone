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

#ifndef _L_SERVER_GROUP_CHAT_ROOM_H_
#define _L_SERVER_GROUP_CHAT_ROOM_H_

#include "chat/chat-room/chat-room.h"
#include "conference/local-conference.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class SalCallOp;
class ServerGroupChatRoomPrivate;

class ServerGroupChatRoom : public ChatRoom , public ConferenceListener {
public:
	ServerGroupChatRoom (const std::shared_ptr<Core> &core, SalCallOp *op);

	ServerGroupChatRoom (
		const std::shared_ptr<Core> &core,
		const ConferenceAddress &peerAddress,
		AbstractChatRoom::CapabilitiesMask capabilities,
		const std::shared_ptr<ChatRoomParams> &params,
		const std::string &subject,
		std::list<std::shared_ptr<Participant>> &&participants,
		unsigned int lastNotifyId
	);

	virtual ~ServerGroupChatRoom ();

	std::shared_ptr<Core> getCore () const;

	void allowCpim (bool value) override;
	void allowMultipart (bool value) override;
	bool canHandleCpim () const override;
	bool canHandleMultipart () const override;

	CapabilitiesMask getCapabilities () const override;
	bool hasBeenLeft () const override;

	const ConferenceAddress getConferenceAddress () const override;

	bool addParticipant (const IdentityAddress &participantAddress) override;
	bool addParticipant (std::shared_ptr<Call> call) override {return getConference()->addParticipant(call); };

	void join (const IdentityAddress &participantAddress) override { getConference()->join(participantAddress); };
	bool update(const ConferenceParamsInterface &newParameters) override { return getConference()->update(newParameters); };

	bool removeParticipant (const std::shared_ptr<Participant> &participant) override;

	std::shared_ptr<Participant> findParticipant (const std::shared_ptr<const CallSession> &session) const;
	std::shared_ptr<Participant> findParticipant (const IdentityAddress &participantAddress) const override;
	std::shared_ptr<Participant> findCachedParticipant (const std::shared_ptr<const CallSession> &session) const;
	std::shared_ptr<Participant> findCachedParticipant (const IdentityAddress &participantAddress) const;
	std::shared_ptr<ParticipantDevice> findCachedParticipantDevice (const std::shared_ptr<const CallSession> &session) const;

	std::shared_ptr<Participant> getMe () const override;
	int getParticipantCount () const override;
	const std::list<std::shared_ptr<Participant>> &getParticipants () const override;

	void setParticipantAdminStatus (const std::shared_ptr<Participant> &participant, bool isAdmin) override;

	const std::string &getSubject () const override;
	void setSubject (const std::string &subject) override;

	void join () override;
	void leave () override;

	/* ConferenceListener */
	virtual void onFirstNotifyReceived (const IdentityAddress &addr) override;

	const ConferenceId &getConferenceId () const override { return getConference()->getConferenceId(); };

	void setState (ConferenceInterface::State state) override;
	void subscribeReceived (LinphoneEvent *event);

private:

	std::list<std::shared_ptr<Participant>> cachedParticipants; /*list of participant that habe been added to the chat room. It includes participants that are currently active in the chat room as well as past participants.*/

	L_DECLARE_PRIVATE(ServerGroupChatRoom);
	L_DISABLE_COPY(ServerGroupChatRoom);
};

std::ostream &operator<< (std::ostream &stream, const ServerGroupChatRoom *chatRoom);

LINPHONE_END_NAMESPACE

#endif // ifndef _L_SERVER_GROUP_CHAT_ROOM_H_
